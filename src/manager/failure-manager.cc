#include <iostream>
#include <json.hpp>

#include "failure-manager.hh"
#include "election-manager.hh"
#include "receiver-manager.hh"

#define TIMEOUT_DURATION 3
#define LOOP_SLEEP_DURATION 500
#define RECOVERY_DURATION 3

FailureManager::FailureManager(Messenger& messenger,
                               std::shared_ptr<ReceiverManager> receiverManager)
    : MessageReceiver(messenger, managedTag, receiverManager) {
}

void
broadcastPing(const Messenger& messenger) {
  int nodeId = messenger.getRank();
  int clusterSize = messenger.getClusterSize();

  for (int i = 0; i < clusterSize; i++) {
    if (i != nodeId) {
      Message message;
      messenger.setMessage(FailureCode::PING, message);

      messenger.send(i, message);
    }
  }
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

  int failedNodeId = nodeIndex < nodeId ? nodeIndex : nodeId + 1;

  if (electionManager->getLeaderNodeId() == failedNodeId) {
    electionManager->startElection();
  }

  isAlive[nodeIndex] = false;
}

void
handleNodeRecovery(const int& nodeIndex,
                   Messenger& messenger,
                   std::shared_ptr<ReceiverManager>& receiverManager,
                   int& curRecoveryId,
                   std::mutex& curRecoveryIdMutex,
                   bool& blockClientConn,
                   std::condition_variable& blockClientConnCond,
                   bool& clientConnBlocked,
                   std::condition_variable& clientConnBlockedCond,
                   std::unique_lock<std::mutex>& lock) {
  // pause the client manager
  blockClientConn = true;
  clientConnBlockedCond.wait(lock, [&] { return clientConnBlocked == false; });

  // TODO read the log and set the data field
  Message message;
  messenger.setMessage(FailureCode::STATE, "", message);

  {
    std::unique_lock<std::mutex> lock(curRecoveryIdMutex);

    curRecoveryId = message.getId();
  }

  int dstNodeId = nodeIndex < messenger.getRank() ? nodeIndex : nodeIndex + 1;
  messenger.send(dstNodeId, message);

  std::this_thread::sleep_for(std::chrono::seconds(RECOVERY_DURATION));


  {
    std::unique_lock<std::mutex> lock(curRecoveryIdMutex);

    curRecoveryId = -1;
  }

  // unpause the client manager
  blockClientConn = false;
  blockClientConnCond.notify_all();
}

void
checkTimeStamps(Messenger& messenger,
                std::mutex& mutex,
                std::vector<timePoint>& timeStamps,
                std::vector<bool>& isAlive,
                std::shared_ptr<ReceiverManager>& receiverManager,
                int& curRecoveryId,
                std::mutex& curRecoveryIdMutex,
                bool& blockClientConn,
                std::condition_variable& blockClientConnCond,
                bool& clientConnBlocked,
                std::condition_variable& clientConnBlockedCond) {
  for (int i = 0; i < messenger.getClusterSize() - 1; i++) {
    {
      std::unique_lock<std::mutex> lock(mutex);

      timePoint lastSeen = timeStamps[i];
      auto cur = std::chrono::high_resolution_clock::now();
      int elapsed =
          std::chrono::duration_cast<std::chrono::seconds>(cur - lastSeen)
              .count();
      
      // TODO remove
      int curNodeId = i < messenger.getRank() ? i : i + 1;
      if (isAlive[i] == true && elapsed > TIMEOUT_DURATION) {
        std::cout << "failure detected: " << messenger.getRank() << " failed : " << curNodeId << std::endl;
        handleNodeFailure(i, messenger, isAlive, receiverManager);
      } else if (isAlive[i] == false && elapsed < TIMEOUT_DURATION) {
        std::shared_ptr<ElectionManager> electionManager =
          receiverManager->getReceiver<ElectionManager>();
        bool noCurrentRecovery = curRecoveryId == -1;
        int leaderNodeId = electionManager->getLeaderNodeId();
        int nodeId = messenger.getRank();
        if (noCurrentRecovery == true && leaderNodeId == nodeId) {
          std::cout << "recovery detected: " << messenger.getRank() << " recovered : " << curNodeId << std::endl;
          handleNodeRecovery(i,
                             messenger,
                             receiverManager,
                             curRecoveryId,
                             curRecoveryIdMutex,
                             blockClientConn,
                             blockClientConnCond,
                             clientConnBlocked,
                             clientConnBlockedCond,
                             lock);
        }
      }
    }
  }
}

void
pingCheck(Messenger& messenger,
          std::mutex& mutex,
          std::vector<timePoint>& timeStamps,
          std::vector<bool>& isAlive,
          std::shared_ptr<ReceiverManager>& receiverManager,
          int& curRecoveryId,
          std::mutex& curRecoveryIdMutex,
          bool& blockClientConn,
          std::condition_variable& blockClientConnCond,
          bool& clientConnBlocked,
          std::condition_variable& clientConnBlockedCond) {
  std::shared_ptr<ReplManager> replManager =
      receiverManager->getReceiver<ReplManager>();

  bool isUp = true;
  while (isUp == true) {
    replManager->sleep();

    std::this_thread::sleep_for(std::chrono::milliseconds(LOOP_SLEEP_DURATION));

    checkTimeStamps(messenger,
                    mutex,
                    timeStamps,
                    isAlive,
                    receiverManager,
                    curRecoveryId,
                    curRecoveryIdMutex,
                    blockClientConn,
                    blockClientConnCond,
                    clientConnBlocked,
                    clientConnBlockedCond);
    broadcastPing(messenger);
  }
}

void
FailureManager::init() {
  int n = m_messenger.getClusterSize() - 1;

  m_timeStamps.resize(n);
  m_isAlive.resize(n);

  for (int i = 0; i < n; i++) {
    m_timeStamps[i] = std::chrono::high_resolution_clock::now();
    m_isAlive[i] = true;
  }

  std::thread pingThread = std::thread(pingCheck,
                                       std::ref(m_messenger),
                                       std::ref(m_mutex),
                                       std::ref(m_timeStamps),
                                       std::ref(m_isAlive),
                                       std::ref(m_receiverManager),
                                       std::ref(m_curRecoveryId),
                                       std::ref(m_curRecoveryIdMutex),
                                       std::ref(m_blockClientConn),
                                       std::ref(m_blockClientConnCond),
                                       std::ref(m_clientConnBlocked),
                                       std::ref(m_clientConnBlockedCond));
  pingThread.detach();
}

void
broadcastRecovered(const Messenger& messenger, const int& recoveredNodeId) {
  nlohmann::json json = {{"recoveredNodeId", recoveredNodeId}};
  const std::string& jsonString = json.dump();

  Message message;
  messenger.setMessage(FailureCode::RECOVERED, jsonString, message);

  for (int i = 0; i < messenger.getClusterSize(); i++) {
    if (i != messenger.getRank()) {
      messenger.send(i, message);
    }
  }
}

void
enableComm(const int& recoveredNodeId,
           Messenger& messenger,
           std::mutex& mutex,
           std::vector<bool>& isAlive) {
  int nodeId = messenger.getRank();
  int nodeIndex =
    recoveredNodeId < nodeId ? recoveredNodeId : recoveredNodeId - 1;

  messenger.setNodeStatus(nodeIndex, true);

  {
    std::unique_lock<std::mutex> lock(mutex);
    isAlive[nodeIndex] = true;
  }
}

void
FailureManager::handleMessage(const int& srcNodeId,
                              const Message& receivedMessage,
                              const Messenger::Connection& connection) {
  int clusterSize = m_messenger.getClusterSize();
  FailureCode code = receivedMessage.getCode<FailureCode>();
  int id = receivedMessage.getId();

  int nodeId = m_messenger.getRank();
  switch (code) {
  case FailureCode::PING: {
    std::cout << "ping: from: " << srcNodeId << " on: " << nodeId << std::endl; 
    int i = srcNodeId < nodeId ? srcNodeId : srcNodeId - 1;

    {
      std::unique_lock<std::mutex> lock(m_mutex);
      m_timeStamps[i] = std::chrono::high_resolution_clock::now();
    }
    break;
  }
  case FailureCode::STATE: {
    std::cout << "state" << std::endl; 
    // TODO replace the log file contents with the 'log' data field
    // const std::string& data = receivedMessage.getData();
    // nlohmann::json dataJson = nlohmann::json::parse(data);

    // dataJson.at();

    nlohmann::json json = {{"recoveryId", id}};
    const std::string& jsonString = json.dump();

    Message message;
    m_messenger.setMessage(FailureCode::STATE_UPDATED, jsonString, message);

    m_messenger.send(srcNodeId, message);
    break;
  }
  case FailureCode::STATE_UPDATED: {
    std::cout << "state updated" << std::endl; 
    const std::string& messageData = receivedMessage.getData();
    nlohmann::json json = nlohmann::json::parse(messageData);
    int recoveryId = json.at("recoveryId");
    bool correctRecoveryId;

    {
      std::unique_lock<std::mutex> lock(m_curRecoveryIdMutex);
      correctRecoveryId = m_curRecoveryId == recoveryId;
    }

    std::shared_ptr<ElectionManager> electionManager =
        m_receiverManager->getReceiver<ElectionManager>();

    bool amLeader = electionManager->getLeaderNodeId() == nodeId;
    if (correctRecoveryId == true && amLeader == true) {
      broadcastRecovered(m_messenger, srcNodeId);

      enableComm(srcNodeId, m_messenger, m_mutex, m_isAlive);
    }
    break;
  }
  case FailureCode::RECOVERED: {
    const std::string& messageData = receivedMessage.getData();
    nlohmann::json json = nlohmann::json::parse(messageData);
    int recoveredNodeId = json.at("recoveredNodeId");
    std::cout << "recovered: " << recoveredNodeId << std::endl; 
    int nodeIndex =
        recoveredNodeId < nodeId ? recoveredNodeId : recoveredNodeId - 1;

    if (nodeId == recoveredNodeId) {
      for (int i = 0; i < m_messenger.getClusterSize() - 1; i++) {
        m_timeStamps[i] = std::chrono::high_resolution_clock::now();
      }
    }
    else {
      enableComm(recoveredNodeId, m_messenger, m_mutex, m_isAlive);
    }
    break;
  }
  }
}

void
FailureManager::stopReceiver() {
  Message message;
  m_messenger.setMessage(FailureCode::SHUTDOWN, message);
  m_messenger.send(m_messenger.getRank(), message);
}

void
FailureManager::sleep() {
  std::unique_lock<std::mutex> lock(m_mutex);

  if (m_blockClientConn == true) {
    m_blockClientConnCond.wait(lock,
                               [&] { return m_blockClientConn == false; });
  }
}
