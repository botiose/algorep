#include "receiver-manager.hh"

void
ReceiverManager::waitForReceiver(const MessageTag& tag) {
  int receiverIndex = static_cast<int>(tag);

  m_threads[receiverIndex].join();
}

void
ReceiverManager::stopReceiver(const MessageTag& tag) {
  int receiverIndex = static_cast<int>(tag);

  m_receivers[receiverIndex]->stopReceiver();
}

void
ReceiverManager::stopReceiverWait(const MessageTag& tag) {
  int receiverIndex = static_cast<int>(tag);

  m_receivers[receiverIndex]->stopReceiver();
  m_threads[receiverIndex].join();
}
