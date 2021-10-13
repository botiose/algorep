#pragma once

#include <mutex>
#include <list>

#include "message-receiver.hh"
#include "messenger.hh"

class ClientReceiver : public MessageReceiver {
public:
  ClientReceiver(const Messenger& messenger);

  void
  startReceiveLoop() final;

  void
  handleMessage(const int& srcNodeId, const Message& receivedMessage) final;

  void
  addConnection(Messenger::Connection connection);

private:
  void
  receivePendingMessages(bool& isUp);

  std::mutex m_mutex;

  std::list<Messenger::Connection> m_clientConnections;
};
