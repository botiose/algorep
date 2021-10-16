#pragma once

#include "message-receiver.hh"
#include "messenger.hh"

class FailReceiver : public MessageReceiver {
public:
  FailReceiver(const Messenger& messenger);

  void
  handleMessage(const int& srcNodeId,
                const Message& receivedMessage,
                const Messenger::Connection& connection) final;
};
