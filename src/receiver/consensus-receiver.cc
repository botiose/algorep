#include <iostream>

#include "consensus-receiver.hh"

ConsensusReceiver::ConsensusReceiver(const Messenger& messenger)
    : MessageReceiver(messenger, MessageTag::CONSENSUS) {
}

void
ConsensusReceiver::handleMessage(const int& srcNodeId,
                                 const Message& receivedMessage) {
}
