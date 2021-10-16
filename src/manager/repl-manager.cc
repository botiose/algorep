#include <iostream>

#include "repl-manager.hh"

ReplManager::ReplManager(const Messenger& messenger)
    : MessageReceiver(messenger, MessageTag::REPL) {
}

void
ReplManager::handleMessage(const int& srcNodeId,
                            const Message& receivedMessage,
                            const Messenger::Connection& connection) {
}
