#pragma once

#include "messenger.hh"

class Node {
public:
  Node(const int& nodeId, const int& clusterSize);

  // TODO make private
  void
  startElection() const;

  void
  startReceiveLoop();

  int
  getNodeId() const;

private:
  void
  HandleBullyMessage(const int& srcNodeId, const Message& recievedMessage);

  const Messenger m_messenger;

  const int m_nodeId;
  const int m_clusterSize;
  bool m_isLeader;
  int m_leaderNodeId;
};
