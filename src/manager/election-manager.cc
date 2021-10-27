#include <iostream>
#include <chrono>
#include <thread>

#include "client-manager.hh"
#include "election-manager.hh"
#include "message-info.hh"
#include "receiver-manager.hh"

#define ELECTION_WAIT_DURATION 5
#define LOOP_SLEEP_DURATION 10

ElectionManager::ElectionManager(
    Messenger& messenger, std::shared_ptr<ReceiverManager> receiverManager)
    : MessageReceiver(messenger, managedTag, receiverManager) {
}

void
broadcastElection(Messenger& messenger,
                  std::mutex& mutex,
                  int& leaderNodeId,
                  timePoint& start,
                  bool& aliveReceived) {
  {
    std::unique_lock<std::mutex> lock(mutex);

    aliveReceived = false;
    leaderNodeId = -1;
    start = std::chrono::high_resolution_clock::now();
  }

  Message electionMessage;
  messenger.setMessage(LeaderElectionCode::ELECTION, electionMessage);

  int nodeId = messenger.getRank();
  messenger.broadcast(electionMessage, nodeId + 1);
}

void
setLeaderNodeId(const int& value, std::mutex& mutex, int& leaderNodeId) {
  std::unique_lock<std::mutex> lock(mutex);

  leaderNodeId = value;
}

bool
gotLeader(std::mutex& mutex, const int& leaderNodeId) {
  std::unique_lock<std::mutex> lock(mutex);

  return leaderNodeId != -1;
}

void
declareVictory(const Messenger& messenger,
               std::mutex& mutex,
               int& leaderNodeId) {
  Message victoryMessage;
  messenger.setMessage(LeaderElectionCode::VICTORY, victoryMessage);

  int clusterSize = messenger.getClusterSize();
  messenger.broadcast(victoryMessage, 0, clusterSize, false);

  int nodeId = messenger.getRank();
  setLeaderNodeId(nodeId, mutex, leaderNodeId);
}

void
waitForVictory(Messenger& messenger,
               std::mutex& mutex,
               int& leaderNodeId,
               bool& aliveReceived,
               timePoint& start,
               std::shared_ptr<ReceiverManager>& receiverManager) {
  using namespace std::chrono;
  auto cur = high_resolution_clock::now();
  int elapsed = 0;

  bool keepWaiting = true;
  while (keepWaiting == true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(LOOP_SLEEP_DURATION));

    cur = high_resolution_clock::now();

    {
      std::unique_lock<std::mutex> lock(mutex);

      elapsed = duration_cast<std::chrono::seconds>(cur - start).count();

      keepWaiting = (aliveReceived == true) ||
                    (leaderNodeId == -1) && (elapsed < ELECTION_WAIT_DURATION);
    }
  }

  if (gotLeader(mutex, leaderNodeId) == false) {
    declareVictory(messenger, mutex, leaderNodeId);

    std::shared_ptr<ClientManager> clientManager =
        receiverManager->getReceiver<ClientManager>();
    clientManager->enableClientConn();
  }

  std::cout << "leader elected" << std::endl; 
}

void
triggerElection(Messenger& messenger,
                std::mutex& mutex,
                int& leaderNodeId,
                bool& aliveReceived,
                timePoint& start,
                std::shared_ptr<ReceiverManager> receiverManager) {
  broadcastElection(messenger, mutex, leaderNodeId, start, aliveReceived);

  waitForVictory(
      messenger, mutex, leaderNodeId, aliveReceived, start, receiverManager);
}

void
ElectionManager::startElection() {
  std::thread triggerThread = std::thread(triggerElection,
                                          std::ref(m_messenger),
                                          std::ref(m_mutex),
                                          std::ref(m_leaderNodeId),
                                          std::ref(m_aliveReceived),
                                          std::ref(m_start),
                                          std::ref(m_receiverManager));
  triggerThread.detach();
}

void
ElectionManager::init() {
  this->startElection();
}

void
respondAlive(const Messenger& messenger, const int& srcNodeId) {
  Message aliveMessage;
  messenger.setMessage(LeaderElectionCode::ALIVE, aliveMessage);

  messenger.send(srcNodeId, aliveMessage);
}

void
setAliveReceived(const bool& value, std::mutex& mutex, bool& aliveReceived) {
  std::unique_lock<std::mutex> lock(mutex);

  aliveReceived = value;
}

void
aliveWait(Messenger& messenger,
          std::mutex& mutex,
          bool& aliveReceived,
          int& leaderNodeId,
          timePoint& start) {
  setAliveReceived(true, mutex, aliveReceived);

  std::this_thread::sleep_for(std::chrono::seconds(ELECTION_WAIT_DURATION * 2));

  if (gotLeader(mutex, leaderNodeId) == false) {
    broadcastElection(messenger, mutex, leaderNodeId, start, aliveReceived);
  }

  setAliveReceived(false, mutex, aliveReceived);
}

void
ElectionManager::handleMessage(const int& srcNodeId,
                               const Message& receivedMessage,
                               const Messenger::Connection& connection) {
  int nodeId = m_messenger.getRank();
  int clusterSize = m_messenger.getClusterSize();
  LeaderElectionCode code = receivedMessage.getCode<LeaderElectionCode>();
  switch (code) {
  case LeaderElectionCode::ELECTION: {
    respondAlive(m_messenger, srcNodeId);
    broadcastElection(
        m_messenger, m_mutex, m_leaderNodeId, m_start, m_aliveReceived);
    break;
  }
  case LeaderElectionCode::ALIVE: {
    if (srcNodeId > nodeId) {
      std::thread aliveWaitThread = std::thread(aliveWait,
                                                std::ref(m_messenger),
                                                std::ref(m_mutex),
                                                std::ref(m_aliveReceived),
                                                std::ref(m_leaderNodeId),
                                                std::ref(m_start));
      aliveWaitThread.detach();
    }
    break;
  }
  case LeaderElectionCode::VICTORY: {
    setLeaderNodeId(srcNodeId, m_mutex, m_leaderNodeId);
    break;
  }
  }
}

void
ElectionManager::stopReceiver() {
  Message message;
  m_messenger.setMessage(LeaderElectionCode::SHUTDOWN, message);

  m_messenger.send(m_messenger.getRank(), message);
}

int
ElectionManager::getLeaderNodeId() {
  std::unique_lock<std::mutex> lock(m_mutex);

  return m_leaderNodeId;
}
