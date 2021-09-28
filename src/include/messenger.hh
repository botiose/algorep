#pragma once

#include "message.hh"
#include "message-codes.hh"

class Messenger {
public:
  Messenger(const int& nodeId);

  void
  sendMessage(const int& dstNodeId, const Message& message) const;

  void
  receiveMessage(int& srcNodeId, Message& message) const;
  void
  receiveMessageWithTag(const MessageTag& messageTag,
                        int& srcNodeId,
                        Message& message) const;

  bool
  hasMessageWithTag(const MessageTag& messageTag) const;

private:
  const int m_nodeId;
};
