template <typename T>
void
ReceiverManager::startReceiver(std::shared_ptr<T> receiver) {
  int receiverIndex = static_cast<int>(T::managedTag);

  m_receivers[receiverIndex] = receiver;

  m_threads[receiverIndex] =
      std::thread(&MessageReceiver::startReceiver, m_receivers[receiverIndex]);
}

template <typename T>
std::shared_ptr<T>
ReceiverManager::getReceiver() {
  return std::dynamic_pointer_cast<T>(
      m_receivers[static_cast<int>(T::managedTag)]);
}
