#include "receiver-manager.hh"

void
ReceiverManager::addReceiver(MessageReceiver& receiver) {
  m_receivers.push_back(&receiver);
}

void
ReceiverManager::startReceiverLoops() {
  for (MessageReceiver* receiver : m_receivers) {
    m_threads.push_back(
        std::thread(&MessageReceiver::startReceiveLoop, receiver));
  }
}

void
ReceiverManager::waitForReceivers() {
  for (std::thread& thread : m_threads) {
    thread.join();
  }
}
