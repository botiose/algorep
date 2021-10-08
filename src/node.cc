#include <iostream>

#include "node.hh"
#include "leader-election.hh"
#include "consensus-manager.hh"

void
Node::startReceiveLoops() {
  bool isUp = true;
  while (isUp == true) {
    int srcNodeId;
    Message receivedMessage;
    m_messenger.receiveBlock(srcNodeId, receivedMessage);

    MessageTag messageTag = receivedMessage.getTag();
    switch (messageTag) {
    case MessageTag::LEADER_ELECTION: {
      leader_election::handleElectionMessage(m_messenger,
                                             m_nodeId,
                                             m_clusterSize,
                                             srcNodeId,
                                             receivedMessage,
                                             m_leaderNodeId);

      if (this->isLeader() == true) {
        this->startAcceptThread();
      }

      break;
    }
    case MessageTag::CONSENSUS: {
      m_consensusManager.handleConsensusMessage(
          m_messenger, m_clusterSize, srcNodeId, receivedMessage);
      break;
    }
    }
  }
}

// TODO remove
void
Node::startMessenger() {
  m_messenger.start(m_nodeId, m_clusterSize);

  m_leaderNodeId = m_clusterSize - 1;
}

// TODO remove
void
Node::stopMessenger() const {
  m_messenger.stop();
}

void
Node::startAcceptThread() const {
  // TODO implement
}

void
Node::replicateData(const std::string& data) const {
  ConsensusManager::startConsensus(m_messenger,
                                   m_nodeId, // TODO remove
                                   m_clusterSize,
                                   data);
}

bool
Node::isLeader() const {
  return m_nodeId == m_leaderNodeId;
}

void
createNode() const {
  // TODO implement
}

void
destroyNode() const {
  // TODO implement
}
