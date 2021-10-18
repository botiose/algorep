#include <iostream>

#include "message-receiver.hh"

MessageReceiver::MessageReceiver(const Messenger& messenger,
                                 const MessageTag& tag,
                                 std::shared_ptr<ReplManager> replManager)
    : m_messenger(messenger), m_tag(tag), m_replManager(replManager) {
}

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

    // message code 0 is SHUTDOWN for all message tags
    isUp = receivedMessage.getCodeInt() != 0;

    if (isUp == true) {
      this->handleMessage(srcNodeId, receivedMessage);
    }
  }
}

MessageTag
MessageReceiver::getTag() const {
  return m_tag;
}
