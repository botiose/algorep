#pragma once

#include "message-receiver.hh"
#include "messenger.hh"

class FailureManager : public MessageReceiver {
public:
  FailureManager(const Messenger& messenger);

  void
  handleMessage(const int& srcNodeId,
                const Message& receivedMessage,
                const Messenger::Connection& connection) final;
};
