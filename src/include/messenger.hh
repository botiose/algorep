#pragma once

#include "message.hh"
#include "message-codes.hh"

class Messenger {
public:
  Messenger(const int& nodeId);

  void
  send(const int& dstNodeId, const Message& message) const;

  void
  receiveBlock(int& srcNodeId, Message& message) const;
  void
  receiveWithTagBlock(const MessageTag& messageTag,
                      int& srcNodeId,
                      Message& message) const;
  void
  receiveWithTag(const MessageTag& messageTag,
                 bool& messageReceived,
                 int& srcNodeId,
                 Message& message) const;

  void
  hasPendingWithTag(const MessageTag& messageTag, bool& hasPending) const;

private:
  const int m_nodeId;
};
