#include <iostream>

#include "fail-receiver.hh"

FailReceiver::FailReceiver(const Messenger& messenger)
    : MessageReceiver(messenger, MessageTag::FAIL_DETECTION) {
}

void
FailReceiver::handleMessage(const int& srcNodeId,
                            const Message& receivedMessage,
                            const Messenger::Connection& connection) {
}
