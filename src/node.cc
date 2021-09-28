#include "node.hh"
#include "message-codes.hh"

Node::Node(const int& nodeId, const int& clusterSize)
    : m_nodeId(nodeId), m_clusterSize(clusterSize),
      m_messenger(nodeId) {
}

void
Node::startElection() const {
  Message message;
  message.tag = static_cast<int>(MessageTag::BULLY);
  message.code = static_cast<int>(MessageCode::BULLY_ELECTION);

  for (int dstNode = m_nodeId + 1; dstNode < m_clusterSize; dstNode++) {
    m_messenger.sendMessage(dstNode, message);
  }
}


void
Node::receiveMessage(int& srcNodeId, Message& message) const {
  m_messenger.receiveMessage(srcNodeId, message);
}

int
Node::getNodeId() const {
  return m_nodeId;
};
