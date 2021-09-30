#include <iostream>

#include "node.hh"
#include "message-codes.hh"
#include "leader-election.hh"

void
handleBullyMessage(const Messenger& messenger,
                   const int& nodeId,
                   const int& clusterSize,
                   const int& srcNodeId,
                   const Message& receivedMessage,
                   int& leaderNodeId) {
  LeaderElectionCode bullyCode = static_cast<LeaderElectionCode>(receivedMessage.code);
  switch (bullyCode) {
  case LeaderElectionCode::ELECTION: {
    Message activeMessage{static_cast<int>(MessageTag::LEADER_ELECTION),
                          static_cast<int>(LeaderElectionCode::ALIVE)};

    messenger.send(srcNodeId, activeMessage);

    if (srcNodeId < nodeId) {
      leader_election::startElection(
          messenger, nodeId, clusterSize, leaderNodeId);
    }
    break;
  }
  case LeaderElectionCode::VICTORY: {
    leaderNodeId = srcNodeId;
    break;
  }
  }
}

void
Node::startReceiveLoop() {
  bool isUp = true;
  while (isUp == true) {
    int srcNodeId;
    Message receivedMessage;
    m_messenger.receiveBlock(srcNodeId, receivedMessage);

    MessageTag messageTag = static_cast<MessageTag>(receivedMessage.tag);
    switch (messageTag) {
    case MessageTag::LEADER_ELECTION: {
      handleBullyMessage(m_messenger,
                         m_nodeId,
                         m_clusterSize,
                         srcNodeId,
                         receivedMessage,
                         m_leaderNodeId);
      break;
    }
    }
  }
}

int
Node::getNodeId() const {
  return m_nodeId;
};

void
Node::startMessenger() {
  m_messenger.start(m_nodeId, m_clusterSize);

  m_leaderNodeId = m_clusterSize - 1;
}

void
Node::stopMessenger() const {
  m_messenger.stop();
}
