#pragma once

#include "message-receiver.hh"
#include "messenger.hh"

class ElectionReceiver : public MessageReceiver {
public:
  ElectionReceiver(const Messenger& messenger);

  void
  handleMessage(const int& srcNodeId, const Message& receivedMessage) final;
};
