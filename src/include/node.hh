#pragma once

#include "messenger.hh"

class Node {
public:
  Node() = default;

  void
  startMessenger();
  void
  stopMessenger() const;

  void
  startReceiveLoop();

  int
  getNodeId() const;

private:

  Messenger m_messenger;

  int m_nodeId;
  int m_clusterSize;
  int m_leaderNodeId;
};
