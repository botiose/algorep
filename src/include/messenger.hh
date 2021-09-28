#pragma once

#include "message.hh"

class Messenger {
public:
  Messenger(const int& nodeId);

  void
  sendMessage(const int& dstNodeId, const Message& message) const;
  void
  receiveMessage(int& srcNodeId, Message& message) const;

private:
  const int m_nodeId;
};
