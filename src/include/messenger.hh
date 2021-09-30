#pragma once

#include "message.hh"
#include "message-codes.hh"

class Messenger {
public:
  Messenger() = default;

  void
  start(int& rank, int& clusterSize);
  void
  stop() const;

  void
  getRank(int& rank) const;
  void
  getClusterSize(int& clusterSize) const;

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
  int m_rank;
};
