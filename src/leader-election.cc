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
  victoryMessage.tag = static_cast<int>(MessageTag::BULLY);
  victoryMessage.code = static_cast<int>(BullyCode::VICTORY);

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
busyBullyWait(const Messenger& messenger,
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
    messenger.receiveWithTag(
        MessageTag::BULLY, messageReceived, srcNodeId, receivedMessage);

    if (messageReceived == true) {
      BullyCode messageCode = static_cast<BullyCode>(receivedMessage.code);

      switch (messageCode) {
      case BullyCode::ALIVE: {
        busyBullyWait(messenger,
                      nodeId,
                      clusterSize,
                      VICTORY_WAIT_DURATION,
                      gotVictor,
                      victorNodeId);
        break;
      }
      case BullyCode::ELECTION: {
        Message activeMessage{static_cast<int>(MessageTag::BULLY),
                              static_cast<int>(BullyCode::ALIVE)};

        messenger.send(srcNodeId, activeMessage);
        break;
      }
      case BullyCode::VICTORY: {
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
    message.tag = static_cast<int>(MessageTag::BULLY);
    message.code = static_cast<int>(BullyCode::ELECTION);

    for (int dstNodeId = nodeId + 1; dstNodeId < clusterSize; dstNodeId++) {
      messenger.send(dstNodeId, message);
    }

    busyBullyWait(messenger,
                  nodeId,
                  clusterSize,
                  ELECTION_WAIT_DURATION,
                  gotVictor,
                  victorNodeId);
  }

  leaderNodeId = victorNodeId;
}

} // namespace leader_election
