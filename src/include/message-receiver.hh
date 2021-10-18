#pragma once

#include "message-info.hh"
#include "messenger.hh"
#include "message.hh"

class ReplManager;

class MessageReceiver {
public:
  MessageReceiver(const Messenger& messenger,
                  const MessageTag& tag);

  MessageReceiver(const Messenger& messenger,
                  const MessageTag& tag,
                  std::shared_ptr<ReplManager> replManager);

  virtual void
  startReceiveLoop();

  virtual void
  handleMessage(const int& srcNodeId,
                const Message& receivedMessage,
                const Messenger::Connection& connection = {MPI_COMM_WORLD}) = 0;

  MessageTag
  getTag() const;

protected:
  std::shared_ptr<ReplManager> m_replManager;

  const Messenger& m_messenger;

  MessageTag m_tag;
};
