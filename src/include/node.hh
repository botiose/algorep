#pragma once

#include "messenger.hh"

class Node {
public:
  Node(const int& nodeId, const int& clusterSize);

  void
  startElection() const;

  void
  receiveMessage(int& srcNodeId, Message& message) const;

  int
  getNodeId() const;

private:
  const Messenger m_messenger;

  const int m_nodeId;
  const int m_clusterSize;
};
