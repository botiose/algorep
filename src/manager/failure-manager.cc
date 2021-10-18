#include <iostream>

#include "failure-manager.hh"

FailureManager::FailureManager(const Messenger& messenger,
                               std::shared_ptr<ReplManager> replManager)
  : MessageReceiver(messenger, MessageTag::FAILURE_DETECTION, replManager) {
}

void
FailureManager::handleMessage(const int& srcNodeId,
                              const Message& receivedMessage,
                              const Messenger::Connection& connection) {
}
