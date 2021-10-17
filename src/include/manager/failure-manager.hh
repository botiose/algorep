#pragma once

#include "message-receiver.hh"
#include "messenger.hh"
#include "repl-manager.hh"

class FailureManager : public MessageReceiver {
public:
  FailureManager(const Messenger& messenger,
                 std::shared_ptr<ReplManager> replManager);

  void
  handleMessage(const int& srcNodeId,
                const Message& receivedMessage,
                const Messenger::Connection& connection) final;

private:
  std::shared_ptr<ReplManager> m_replManager;
};
