#include <iostream>

#include "failure-manager.hh"
#include "election-manager.hh"
#include "receiver-manager.hh"

#define TIMEOUT_DURATION 3
#define LOOP_SLEEP_DURATION 500

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
handleNodeRecover(const int& nodeIndex,
                  Messenger& messenger,
                  std::vector<bool>& isAlive,
                  std::shared_ptr<ReceiverManager>& receiverManager) {
  // TODO send system state and pause until current consensus is done
  // TODO do this only if properly recovered
  messenger.setNodeStatus(nodeIndex, true);
  isAlive[nodeIndex] = true;
}

void
checkTimeStamps(Messenger& messenger,
                std::mutex& mutex,
                std::vector<timePoint>& timeStamps,
                std::vector<bool>& isAlive,
                std::shared_ptr<ReceiverManager>& receiverManager) {
  for (int i = 0; i < messenger.getClusterSize() - 1; i++) {
    {
      std::unique_lock<std::mutex> lock(mutex);

      timePoint lastSeen = timeStamps[i];
      auto cur = std::chrono::high_resolution_clock::now();
      int elapsed =
          std::chrono::duration_cast<std::chrono::seconds>(cur - lastSeen)
              .count();

      if (isAlive[i] == true && elapsed > TIMEOUT_DURATION) {
        std::cout << "failure detected" << std::endl;
        handleNodeFailure(i, messenger, isAlive, receiverManager);
      } else if (isAlive[i] == false && elapsed < TIMEOUT_DURATION) {
        std::cout << "recovery detected" << std::endl;
        handleNodeRecover(i, messenger, isAlive, receiverManager);
      }
    }
  }
}

void
pingCheck(Messenger& messenger,
          std::mutex& mutex,
          std::vector<timePoint>& timeStamps,
          std::vector<bool>& isAlive,
          std::shared_ptr<ReceiverManager>& receiverManager) {
  bool isUp = true;
  while (isUp == true) {
    checkTimeStamps(messenger, mutex, timeStamps, isAlive, receiverManager);
    broadcastPing(messenger);

    std::this_thread::sleep_for(std::chrono::milliseconds(LOOP_SLEEP_DURATION));
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

  m_pingThread = std::thread(pingCheck,
                             std::ref(m_messenger),
                             std::ref(m_mutex),
                             std::ref(m_timeStamps),
                             std::ref(m_isAlive), 
                             std::ref(m_receiverManager));
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
    std::cout << "ping" << std::endl; 
    
    int i = srcNodeId < nodeId ? srcNodeId : srcNodeId - 1;

    {
      std::unique_lock<std::mutex> lock(m_mutex);
      m_timeStamps[i] = std::chrono::high_resolution_clock::now();
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
