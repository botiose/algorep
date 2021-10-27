#include <iostream>
#include <json.hpp>

#include "failure-manager.hh"
#include "election-manager.hh"
#include "receiver-manager.hh"

#define TIMEOUT_DURATION 3
#define LOOP_SLEEP_DURATION 500
#define RECOVERY_DURATION 3

FailureManager::FailureManager(Messenger& messenger,
                               std::shared_ptr<ReceiverManager> receiverManager,
                               LogFileManager& logFileManager)
    : MessageReceiver(messenger, managedTag, receiverManager),
      m_logFileManager(logFileManager) {
}

void
broadcastPing(Messenger& messenger) {
  Message message;
  messenger.setMessage(FailureCode::PING, message);

  int clusterSize = messenger.getClusterSize();
  messenger.broadcast(message, 0, clusterSize, false);
}

int
indexToId(const int& nodeId, const int& index) {
  return index < nodeId ? index : index + 1;
}

void
handleNodeFailure(const int& nodeIndex,
                  Messenger& messenger,
                  std::vector<bool>& isAlive,
                  std::shared_ptr<ReceiverManager>& receiverManager) {
  messenger.setNodeStatus(nodeIndex, false);

  int nodeId = messenger.getRank();
  std::shared_ptr<ElectionManager> electionManager =
      receiverManager->getReceiver<ElectionManager>();

  int failedNodeId = indexToId(nodeId, nodeIndex);

  if (electionManager->getLeaderNodeId() == failedNodeId) {
    electionManager->startElection();
  }

  isAlive[nodeIndex] = false;
}

void
handleNodeRecovery(int nodeIndex,
                   Messenger& messenger,
                   LogFileManager& logFileManager,
                   std::shared_ptr<ReceiverManager>& receiverManager,
                   FailureManager::Context& failureContext) {
  // pause the client manager
  {
    std::unique_lock<std::mutex> lock(failureContext.clientConnMutex);
    failureContext.blockClientConn = true;
    failureContext.clientConnBlockedCond.wait(
        lock, [&] { return failureContext.clientConnBlocked == false; });
  }

  std::string logContents;
  logFileManager.read(logContents);
  nlohmann::json json = {{"state", logContents}};
  const std::string& jsonString = json.dump();

  Message message;
  messenger.setMessage(FailureCode::STATE, jsonString, message);

  {
    std::unique_lock<std::mutex> lock(failureContext.curRecoveryIdMutex);

    failureContext.curRecoveryId = message.getId();
  }

  int dstNodeId = indexToId(messenger.getRank(), nodeIndex);
  messenger.send(dstNodeId, message);

  std::this_thread::sleep_for(std::chrono::seconds(RECOVERY_DURATION));

  {
    std::unique_lock<std::mutex> lock(failureContext.curRecoveryIdMutex);

    failureContext.curRecoveryId = -1;
  }

  // unpause the client manager
  {
    std::unique_lock<std::mutex> lock(failureContext.clientConnMutex);
    failureContext.blockClientConn = false;
    failureContext.blockClientConnCond.notify_all();
  }
}

void
checkTimeStamps(Messenger& messenger,
                LogFileManager& logFileManager,
                std::shared_ptr<ReceiverManager>& receiverManager,
                FailureManager::Context& failureContext,
                std::thread& recoveryThread) {
  for (int i = 0; i < messenger.getClusterSize() - 1; i++) {
    {
      std::unique_lock<std::mutex> lock(failureContext.mutex);

      timePoint lastSeen = failureContext.timeStamps[i];
      auto cur = std::chrono::high_resolution_clock::now();
      int elapsed =
          std::chrono::duration_cast<std::chrono::seconds>(cur - lastSeen)
              .count();

      if (failureContext.isAlive[i] == true && elapsed > TIMEOUT_DURATION) {
        std::cout << "failure detected: " << messenger.getRank() << " nodeIndex: " << i << std::endl; 
        handleNodeFailure(
            i, messenger, failureContext.isAlive, receiverManager);
      } else if (failureContext.isAlive[i] == false &&
                 elapsed < TIMEOUT_DURATION) {
        std::shared_ptr<ElectionManager> electionManager =
            receiverManager->getReceiver<ElectionManager>();
        bool noCurrentRecovery = failureContext.curRecoveryId == -1;
        int leaderNodeId = electionManager->getLeaderNodeId();
        int nodeId = messenger.getRank();
        if (noCurrentRecovery == true && leaderNodeId == nodeId) {
          std::cout << "recovery detected: " << messenger.getRank() << " nodeIndex: " << i << std::endl; 
          recoveryThread = std::thread(handleNodeRecovery,
                                       i,
                                       std::ref(messenger),
                                       std::ref(logFileManager),
                                       std::ref(receiverManager),
                                       std::ref(failureContext));
        }
      }
    }
  }
}

void
pingCheck(Messenger& messenger,
          LogFileManager& logFileManager,
          std::shared_ptr<ReceiverManager>& receiverManager,
          FailureManager::Context& failureContext,
          std::thread& recoveryThread,
          bool& pingThreadIsUp) {
  std::shared_ptr<ReplManager> replManager =
      receiverManager->getReceiver<ReplManager>();

  bool isUp = true;
  while (isUp == true) {
    replManager->sleep();

    std::this_thread::sleep_for(std::chrono::milliseconds(LOOP_SLEEP_DURATION));

    checkTimeStamps(messenger,
                    logFileManager,
                    receiverManager,
                    failureContext,
                    recoveryThread);
    broadcastPing(messenger);

    {
      std::unique_lock<std::mutex> lock(failureContext.mutex);
      isUp = pingThreadIsUp;
    }    
  }
}

void
FailureManager::init() {
  int n = m_messenger.getClusterSize() - 1;

  m_context.timeStamps.resize(n);
  m_context.isAlive.resize(n);

  for (int i = 0; i < n; i++) {
    m_context.timeStamps[i] = std::chrono::high_resolution_clock::now();
    m_context.isAlive[i] = true;
  }

  m_pingThread = std::thread(pingCheck,
                             std::ref(m_messenger),
                             std::ref(m_logFileManager),
                             std::ref(m_receiverManager),
                             std::ref(m_context),
                             std::ref(m_recoveryThread),
                             std::ref(m_pingThreadIsUp));
}

int
idToIndex(const int& nodeId1, const int& nodeId2) {
  return nodeId2 < nodeId1 ? nodeId2 : nodeId2 - 1;
}

void
broadcastRecovered(Messenger& messenger, const int& recoveredNodeId) {
  nlohmann::json json = {{"recoveredNodeId", recoveredNodeId}};
  const std::string& jsonString = json.dump();

  Message message;
  messenger.setMessage(FailureCode::RECOVERED, jsonString, message);

  int clusterSize = messenger.getClusterSize();
  messenger.broadcast(message, 0, clusterSize, false);
}

void
enableComm(const int& recoveredNodeId,
           Messenger& messenger,
           std::mutex& mutex,
           std::vector<bool>& isAlive) {
  int nodeIndex = idToIndex(messenger.getRank(), recoveredNodeId);

  messenger.setNodeStatus(nodeIndex, true);

  {
    std::unique_lock<std::mutex> lock(mutex);
    isAlive[nodeIndex] = true;
  }
}

void
handleStateUpdate(const int& srcNodeId,
                  const Message& receivedMessage,
                  Messenger& messenger,
                  std::mutex& mutex,
                  std::mutex& curRecoveryIdMutex,
                  int& curRecoveryId,
                  std::shared_ptr<ReceiverManager>& receiverManager,
                  std::vector<bool>& isAlive) {
  const std::string& messageData = receivedMessage.getData();
  nlohmann::json json = nlohmann::json::parse(messageData);
  int recoveryId = json.at("recoveryId");
  bool correctRecoveryId;

  {
    std::unique_lock<std::mutex> lock(curRecoveryIdMutex);
    correctRecoveryId = curRecoveryId == recoveryId;
  }

  std::shared_ptr<ElectionManager> electionManager =
      receiverManager->getReceiver<ElectionManager>();

  int nodeId = messenger.getRank();
  bool amLeader = electionManager->getLeaderNodeId() == nodeId;
  if (correctRecoveryId == true && amLeader == true) {
    broadcastRecovered(messenger, srcNodeId);

    enableComm(srcNodeId, messenger, mutex, isAlive);
  }
}

void
handleRecovered(const Message& receivedMessage,
                Messenger& messenger,
                std::mutex& mutex,
                std::vector<timePoint>& timeStamps,
                std::vector<bool>& isAlive) {
  const std::string& messageData = receivedMessage.getData();
  nlohmann::json json = nlohmann::json::parse(messageData);
  int recoveredNodeId = json.at("recoveredNodeId");
  int nodeId = messenger.getRank();
  int nodeIndex = idToIndex(nodeId, recoveredNodeId);

  if (nodeId == recoveredNodeId) {
    for (int i = 0; i < messenger.getClusterSize() - 1; i++) {
      timeStamps[i] = std::chrono::high_resolution_clock::now();
    }
  } else {
    enableComm(recoveredNodeId, messenger, mutex, isAlive);
  }
}

void
handleState(const int& srcNodeId,
            const Message& receivedMessage,
            Messenger& messenger,
            LogFileManager& logFileManager) {
  const std::string& messageData = receivedMessage.getData();
  nlohmann::json messageDataJson = nlohmann::json::parse(messageData);
  const std::string& logContents = messageDataJson.at("state");  

  logFileManager.replace(logContents);

  int id = receivedMessage.getId();
  nlohmann::json json = {{"recoveryId", id}};
  const std::string& jsonString = json.dump();

  Message message;
  messenger.setMessage(FailureCode::STATE_UPDATED, jsonString, message);

  messenger.send(srcNodeId, message);
}

void
handlePing(const int& srcNodeId,
           Messenger& messenger,
           std::mutex& mutex,
           std::vector<timePoint>& timeStamps) {
  int i = idToIndex(messenger.getRank(), srcNodeId);

  {
    std::unique_lock<std::mutex> lock(mutex);
    timeStamps[i] = std::chrono::high_resolution_clock::now();
  }
}

void
FailureManager::handleMessage(const int& srcNodeId,
                              const Message& receivedMessage,
                              const Messenger::Connection& connection) {
  int clusterSize = m_messenger.getClusterSize();
  FailureCode code = receivedMessage.getCode<FailureCode>();
  switch (code) {
  case FailureCode::PING: {
    handlePing(srcNodeId, m_messenger, m_context.mutex, m_context.timeStamps);
    break;
  }
  case FailureCode::STATE: {
    handleState(srcNodeId, receivedMessage, m_messenger, m_logFileManager);
    break;
  }
  case FailureCode::STATE_UPDATED: {
    handleStateUpdate(srcNodeId,
                      receivedMessage,
                      m_messenger,
                      m_context.mutex,
                      m_context.curRecoveryIdMutex,
                      m_context.curRecoveryId,
                      m_receiverManager,
                      m_context.isAlive);
    break;
  }
  case FailureCode::RECOVERED: {
    handleRecovered(receivedMessage,
                    m_messenger,
                    m_context.mutex,
                    m_context.timeStamps,
                    m_context.isAlive);
    break;
  }
  }
}

void
FailureManager::stopReceiver() {
  Message message;
  m_messenger.setMessage(FailureCode::SHUTDOWN, message);

  m_messenger.send(m_messenger.getRank(), message);

  {
    std::unique_lock<std::mutex> lock(m_context.mutex);
    m_pingThreadIsUp = false;
  }  

  m_pingThread.join();
}

void
FailureManager::sleep() {
  std::unique_lock<std::mutex> lock(m_context.clientConnMutex);

  if (m_context.blockClientConn == true) {
    m_context.blockClientConnCond.wait(
        lock, [&] { return m_context.blockClientConn == false; });

    m_recoveryThread.join();
  }
}
