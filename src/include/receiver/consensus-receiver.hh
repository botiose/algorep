#pragma once

#include "message-receiver.hh"
#include "messenger.hh"

class ConsensusReceiver : public MessageReceiver {
public:
  ConsensusReceiver(const Messenger& messenger);

  void
  handleMessage(const int& srcNodeId, const Message& receivedMessage) final;
};
