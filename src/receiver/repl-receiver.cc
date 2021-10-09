#include <iostream>

#include "repl-receiver.hh"

ReplReceiver::ReplReceiver(const Messenger& messenger)
    : MessageReceiver(messenger, MessageTag::REPL) {
}

void
ReplReceiver::handleMessage(const int& srcNodeId,
                            const Message& receivedMessage) {
}
