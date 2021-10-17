#include <iostream>
#include <chrono>

#include "election-manager.hh"
#include "message-info.hh"

#define ELECTION_WAIT_DURATION 5
#define VICTORY_WAIT_DURATION 60

void
declareVictory(const Messenger& messenger,
               const int& nodeId,
               const int& clusterSize,
               bool& gotVictor,
               int& victorNodeId) {
  Message victoryMessage;

  messenger.setMessage(LeaderElectionCode::VICTORY, victoryMessage);

  for (int i = 0; i < clusterSize; i++) {
    if (i == nodeId) {
      continue;
    }

    messenger.send(i, victoryMessage);
  }

  gotVictor = true;
  victorNodeId = nodeId;
}

void
busyLeaderElectionWait(const Messenger& messenger,
                       const int& nodeId,
                       const int& clusterSize,
                       const int& waitSeconds,
                       bool& gotVictor,
                       int& victorNodeId) {
  using namespace std::chrono;
  auto start = high_resolution_clock::now();
  auto cur = high_resolution_clock::now();
  int elapsed = 0;
  bool messageReceived = false;

  while ((messageReceived == false) && (elapsed < waitSeconds)) {
    int srcNodeId;
    Message receivedMessage;
    messenger.receiveWithTag(MessageTag::LEADER_ELECTION,
                             messageReceived,
                             srcNodeId,
                             receivedMessage);

    if (messageReceived == true) {
      LeaderElectionCode messageCode =
          receivedMessage.getCode<LeaderElectionCode>();

      switch (messageCode) {
      case LeaderElectionCode::ALIVE: {
        // If the current process receives an Answer from a process with a
        // higher ID, it sends no further messages for this election and waits
        // for a Victory message. (If there is no Victory message after a period
        // of time, it restarts the process at the beginning.)
        busyLeaderElectionWait(messenger,
                               nodeId,
                               clusterSize,
                               VICTORY_WAIT_DURATION,
                               gotVictor,
                               victorNodeId);
        break;
      }
      case LeaderElectionCode::ELECTION: {
        // If the current process receives an Election message from another
        // process with a lower ID it sends an Answer message back and
        // contineous the current election.
        Message activeMessage;
        messenger.setMessage(LeaderElectionCode::ALIVE, activeMessage);

        messenger.send(srcNodeId, activeMessage);
        break;
      }
      case LeaderElectionCode::VICTORY: {
        // If the current process receives a Coordinator message, it treats the
        // sender as the coordinator.
        victorNodeId = srcNodeId;
        gotVictor = true;
        break;
      }
      }
    }

    cur = high_resolution_clock::now();
    elapsed = duration_cast<std::chrono::seconds>(cur - start).count();
  }

  // If the current process receives no Answer after sending an Election
  // message, then it broadcasts a Victory message to all other processes and
  // becomes the Coordinator.
  if (messageReceived == false) {
    declareVictory(messenger, nodeId, clusterSize, gotVictor, victorNodeId);
  }
}

void
startElection(const Messenger& messenger,
              const int& nodeId,
              const int& clusterSize,
              int& leaderNodeId) {
  bool gotVictor = false;
  int victorNodeId;
  while (gotVictor == false) {
    Message message;
    messenger.setMessage(LeaderElectionCode::ELECTION, message);

    for (int dstNodeId = nodeId + 1; dstNodeId < clusterSize; dstNodeId++) {
      messenger.send(dstNodeId, message);
    }

    busyLeaderElectionWait(messenger,
                           nodeId,
                           clusterSize,
                           ELECTION_WAIT_DURATION,
                           gotVictor,
                           victorNodeId);
  }

  leaderNodeId = victorNodeId;
}

ElectionManager::ElectionManager(const Messenger& messenger,   std::shared_ptr<ReplManager> replManager)
  : MessageReceiver(messenger, MessageTag::LEADER_ELECTION), m_replManager(replManager) {
}

void
ElectionManager::handleMessage(const int& srcNodeId,
                               const Message& receivedMessage,
                               const Messenger::Connection& connection) {
  std::cout << "election manager: listening" << std::endl;
  int nodeId = m_messenger.getRank();
  int clusterSize = m_messenger.getClusterSize();
  LeaderElectionCode bullyCode = receivedMessage.getCode<LeaderElectionCode>();
  switch (bullyCode) {
  case LeaderElectionCode::ELECTION: {
    // If the current process receives an Election message from another process
    // with a lower ID it sends an Answer message back and starts the election
    // process at the beginning, by sending an Election message to
    // higher-numbered processes.
    Message activeMessage;

    m_messenger.setMessage(LeaderElectionCode::ALIVE, activeMessage);
    m_messenger.send(srcNodeId, activeMessage);

    if (srcNodeId < nodeId) {
      startElection(m_messenger, nodeId, clusterSize, m_leaderNodeId);
    }
    break;
  }
  case LeaderElectionCode::VICTORY: {
    m_leaderNodeId = srcNodeId;
    break;
  }
  }
}
