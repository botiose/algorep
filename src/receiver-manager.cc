#include "receiver-manager.hh"

void
ReceiverManager::startReceiver(std::shared_ptr<MessageReceiver> receiver) {
  int receiverIndex = static_cast<int>(receiver->getTag());

  m_activeReceivers[receiverIndex] = true;

  m_receivers[receiverIndex] = receiver;

  m_threads[receiverIndex] = std::thread(&MessageReceiver::startReceiveLoop,
                                         m_receivers[receiverIndex]);
}

void
ReceiverManager::waitForAllReceivers() {
  for (int i = 0; i < static_cast<int>(MessageTag::SIZE); i++) {
    // trading off additional cast for the sake of reducing duplicated code
    MessageTag tag = static_cast<MessageTag>(i);
    this->waitForReceiver(tag);
  }
}

void
ReceiverManager::waitForReceiver(const MessageTag& tag) {
  int receiverIndex = static_cast<int>(tag);

  if (m_activeReceivers[receiverIndex] == true) {

    m_receivers[receiverIndex].reset();
    m_threads[receiverIndex].join();
  }
}
