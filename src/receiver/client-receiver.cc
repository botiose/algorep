#include <iostream>

#include "client-receiver.hh"

ClientReceiver::ClientReceiver(const Messenger& messenger)
    : MessageReceiver(messenger, MessageTag::CLIENT) {
}

void
ClientReceiver::handleMessage(const int& srcNodeId,
                              const Message& receivedMessage) {
}

void
ClientReceiver::addConnection(Messenger::Connection connection) {
  std::unique_lock<std::mutex> lock(m_mutex);

  m_clientConnections.push_back(connection);
}
