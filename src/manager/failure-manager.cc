#include <iostream>

#include "failure-manager.hh"

FailureManager::FailureManager(Messenger& messenger,
                               std::shared_ptr<ReceiverManager> receiverManager)
    : MessageReceiver(messenger, managedTag, receiverManager) {
}

void
FailureManager::handleMessage(const int& srcNodeId,
                              const Message& receivedMessage,
                              const Messenger::Connection& connection) {
}

void
FailureManager::stopReceiver() {
  Message message;
  m_messenger.setMessage(FailDetectionCode::SHUTDOWN, message);
  m_messenger.send(m_messenger.getRank(), message);
}
