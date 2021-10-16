#include "receiver-manager.hh"

void
ReceiverManager::startReceiver(std::shared_ptr<MessageReceiver> receiver) {
  m_receivers.push_back(receiver);
  m_threads.push_back(
      std::thread(&MessageReceiver::startReceiveLoop, receiver));
}

void
ReceiverManager::waitForReceivers() {
  for (std::thread& thread : m_threads) {
    thread.join();
  }
}
