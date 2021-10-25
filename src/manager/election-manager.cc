#include <iostream>
#include <chrono>
#include <thread>

#include "client-manager.hh"
#include "election-manager.hh"
#include "message-info.hh"
#include "receiver-manager.hh"

#define VICTORY_WAIT_DURATION 5
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
  int clusterSize = messenger.getClusterSize();
  for (int i = nodeId + 1; i < clusterSize; i++) {
    messenger.send(i, electionMessage);
  }
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
  int nodeId = messenger.getRank();
  int clusterSize = messenger.getClusterSize();

  Message victoryMessage;
  messenger.setMessage(LeaderElectionCode::VICTORY, victoryMessage);

  for (int i = 0; i < clusterSize; i++) {
    if (i == nodeId) {
      continue;
    }

    messenger.send(i, victoryMessage);
  }

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

      keepWaiting =
          (aliveReceived == true) ||
          (leaderNodeId == -1) && (elapsed < ELECTION_WAIT_DURATION);
    }
  }

  if (gotLeader(mutex, leaderNodeId) == false) {
    declareVictory(messenger, mutex, leaderNodeId);

    std::shared_ptr<ClientManager> clientManager =
        receiverManager->getReceiver<ClientManager>();
    clientManager->enableClientConn();
  }
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
  // TODO spawn thread & detach thread
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
waitForVictoryMessage(const Messenger& messenger,
                      std::mutex& mutex,
                      int& leaderNodeId) {
  using namespace std::chrono;
  auto cur = high_resolution_clock::now();
  auto start = high_resolution_clock::now();
  int elapsed = 0;

  bool gotVictory = false;
  while ((gotVictory == false) && (elapsed < VICTORY_WAIT_DURATION)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(LOOP_SLEEP_DURATION));

    Message message;
    int srcNodeId;
    messenger.receiveWithTagBlock(
        MessageTag::LEADER_ELECTION, srcNodeId, message);

    LeaderElectionCode code = message.getCode<LeaderElectionCode>();

    switch (code) {
    case LeaderElectionCode::VICTORY: {
      setLeaderNodeId(srcNodeId, mutex, leaderNodeId);
      break;
    }
    }

    cur = high_resolution_clock::now();
    elapsed = duration_cast<std::chrono::seconds>(cur - start).count();
  }
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
      setAliveReceived(true, m_mutex, m_aliveReceived);

      waitForVictoryMessage(m_messenger, m_mutex, m_leaderNodeId);

      if (gotLeader(m_mutex, m_leaderNodeId) == false) {
        broadcastElection(m_messenger, m_mutex, m_leaderNodeId, m_start, m_aliveReceived);
      }

      setAliveReceived(false, m_mutex, m_aliveReceived);
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
