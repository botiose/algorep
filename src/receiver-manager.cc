#include "receiver-manager.hh"

void
ReceiverManager::waitForReceiver(const MessageTag& tag) {
  int receiverIndex = static_cast<int>(tag);

  if (m_isActive[receiverIndex] == true) {
    m_threads[receiverIndex].join();
  }
}

void
ReceiverManager::stopReceiver(const MessageTag& tag) {
  int receiverIndex = static_cast<int>(tag);

  if (m_isActive[receiverIndex] == true) {
    m_receivers[receiverIndex]->stopReceiver();

    m_isActive[receiverIndex] = false;
  }
}

void
ReceiverManager::stopReceiverWait(const MessageTag& tag) {
  int receiverIndex = static_cast<int>(tag);

  if (m_isActive[receiverIndex] == true) {
    m_receivers[receiverIndex]->stopReceiver();
    m_threads[receiverIndex].join();

    m_isActive[receiverIndex] = false;
  }
}
