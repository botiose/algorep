#pragma once

#include "message-info.hh"
#include "messenger.hh"
#include "message.hh"

class MessageReceiver {
public:
  MessageReceiver(const Messenger& messenger, const MessageTag& tag);

  virtual void
  startReceiveLoop();

  virtual void
  handleMessage(const int& srcNodeId,
                const Message& receivedMessage,
                const Messenger::Connection& connection = {MPI_COMM_WORLD}) = 0;

protected:
  const Messenger& m_messenger;

  MessageTag m_tag;
};
