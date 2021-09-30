#include <chrono>

#include "leader-election.hh"

#include "message-codes.hh"

#define ELECTION_WAIT_DURATION 5
#define VICTORY_WAIT_DURATION 60

namespace leader_election {
void
declareVictory(const Messenger& messenger,
               const int& nodeId,
               const int& clusterSize,
               bool& gotVictor,
               int& victorNodeId) {
  Message victoryMessage;
  victoryMessage.tag = static_cast<int>(MessageTag::LEADER_ELECTION);
  victoryMessage.code = static_cast<int>(LeaderElectionCode::VICTORY);

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
          static_cast<LeaderElectionCode>(receivedMessage.code);

      switch (messageCode) {
      case LeaderElectionCode::ALIVE: {
        busyLeaderElectionWait(messenger,
                               nodeId,
                               clusterSize,
                               VICTORY_WAIT_DURATION,
                               gotVictor,
                               victorNodeId);
        break;
      }
      case LeaderElectionCode::ELECTION: {
        Message activeMessage{static_cast<int>(MessageTag::LEADER_ELECTION),
                              static_cast<int>(LeaderElectionCode::ALIVE)};

        messenger.send(srcNodeId, activeMessage);
        break;
      }
      case LeaderElectionCode::VICTORY: {
        victorNodeId = srcNodeId;
        gotVictor = true;
        break;
      }
      }
    }

    cur = high_resolution_clock::now();
    elapsed = duration_cast<std::chrono::seconds>(cur - start).count();
  }

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
    message.tag = static_cast<int>(MessageTag::LEADER_ELECTION);
    message.code = static_cast<int>(LeaderElectionCode::ELECTION);

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

} // namespace leader_election
