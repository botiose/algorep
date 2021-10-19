#include <iostream>

#include "message-receiver.hh"

MessageReceiver::MessageReceiver(
    Messenger& messenger,
    const MessageTag& tag,
    std::shared_ptr<ReceiverManager> receiverManager)
    : m_messenger(messenger), m_tag(tag), m_receiverManager(receiverManager) {
}

void
MessageReceiver::startReceiver() {
  bool isUp = true;
  while (isUp == true) {
    int srcNodeId;
    Message receivedMessage;

    m_messenger.receiveWithTagBlock(m_tag, srcNodeId, receivedMessage);

    // message code 0 is SHUTDOWN for all message tags
    isUp = receivedMessage.getCodeInt() != 0;

    if (isUp == true) {
      this->handleMessage(srcNodeId, receivedMessage);
    }
  }
}
