#include <iostream>
#include <thread>

#include "message-receiver.hh"
#include "repl-manager.hh"
#include "receiver-manager.hh"

MessageReceiver::MessageReceiver(
    Messenger& messenger,
    const MessageTag& tag,
    std::shared_ptr<ReceiverManager> receiverManager)
    : m_messenger(messenger), m_tag(tag), m_receiverManager(receiverManager) {
}

void
MessageReceiver::startReceiver() {
  std::shared_ptr<ReplManager> replManager =
      m_receiverManager->getReceiver<ReplManager>();

  replManager->sleep();

  this->init();

  bool isUp = true;
  while (isUp == true) {
    int srcNodeId;
    Message receivedMessage;

    m_messenger.receiveWithTagBlock(m_tag, srcNodeId, receivedMessage);

    // message code 0 is SHUTDOWN for all message tags
    isUp = receivedMessage.getCodeInt() != 0;

    if (isUp == true) {

      replManager->sleep();

      this->handleMessage(srcNodeId, receivedMessage);
    }
  }
}
