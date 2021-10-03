#include <iostream>

#include "node.hh"
#include "leader-election.hh"

void
Node::startReceiveLoop() {
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
    }
  }
}

void
Node::startMessenger() {
  m_messenger.start(m_nodeId, m_clusterSize);

  m_leaderNodeId = m_clusterSize - 1;
}

void
Node::stopMessenger() const {
  m_messenger.stop();
}

void
Node::startAcceptThread() const {
  // TODO implement
  // m_messenger.publish();
  // while (true) {
  //   m_messenger.acceptConnection();
  // }
}

void
Node::replicate(const std::string& data) const {
  
}

bool
Node::isLeader() const {
  return m_nodeId == m_leaderNodeId;
}
