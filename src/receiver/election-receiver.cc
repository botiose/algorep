#include <iostream>

#include "election-receiver.hh"

ElectionReceiver::ElectionReceiver(const Messenger& messenger)
    : MessageReceiver(messenger, MessageTag::LEADER_ELECTION) {
}

void
ElectionReceiver::handleMessage(const int& srcNodeId,
                                const Message& receivedMessage) {
}
