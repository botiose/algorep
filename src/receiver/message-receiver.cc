#include <iostream>

#include "message-receiver.hh"

MessageReceiver::MessageReceiver(const Messenger& messenger,
                                 const MessageTag& tag)
    : m_messenger(messenger), m_tag(tag) {
}

void
MessageReceiver::startReceiveLoop() {
  bool isUp = true;
  while (isUp == true) {
    int srcNodeId;
    Message receivedMessage;

    m_messenger.receiveWithTagBlock(m_tag, srcNodeId, receivedMessage);

    isUp = receivedMessage.getCodeInt() != 0;

    if (isUp == true) {
      this->handleMessage(srcNodeId, receivedMessage);
    }
  }
}
