#pragma once

#include "message-receiver.hh"
#include "messenger.hh"

class ClientReceiver : public MessageReceiver {
public:
  ClientReceiver(const Messenger& messenger);

  // void
  // startReceiveLoop() final;

  void
  handleMessage(const int& srcNodeId, const Message& receivedMessage) final;
};
