#pragma once

#include "message-receiver.hh"
#include "messenger.hh"

class ReplReceiver : public MessageReceiver {
public:
  ReplReceiver(const Messenger& messenger);

  void
  handleMessage(const int& srcNodeId,
                const Message& receivedMessage,
                const Messenger::Connection& connection) final;
};
