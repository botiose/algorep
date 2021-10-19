#pragma once

#include "message-receiver.hh"
#include "messenger.hh"
#include "repl-manager.hh"

class FailureManager : public MessageReceiver {
public:
  inline static MessageTag managedTag = MessageTag::FAILURE_DETECTION;

  FailureManager(Messenger& messenger,
                 std::shared_ptr<ReceiverManager> receiverManager);

  void
  handleMessage(const int& srcNodeId,
                const Message& receivedMessage,
                const Messenger::Connection& connection) final;

  void
  stopReceiver() final;
};
