#include <iostream>

#include "client-receiver.hh"

ClientReceiver::ClientReceiver(const Messenger& messenger)
    : MessageReceiver(messenger, MessageTag::CLIENT) {
}

void
ClientReceiver::handleMessage(const int& srcNodeId,
                                 const Message& receivedMessage) {
}
