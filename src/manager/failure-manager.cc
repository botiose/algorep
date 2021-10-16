#include <iostream>

#include "failure-manager.hh"

FailureManager::FailureManager(const Messenger& messenger)
    : MessageReceiver(messenger, MessageTag::FAILURE_DETECTION) {
}

void
FailureManager::handleMessage(const int& srcNodeId,
                              const Message& receivedMessage,
                              const Messenger::Connection& connection) {
}
