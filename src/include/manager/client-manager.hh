#pragma once

#include <mutex>
#include <list>

#include "message-receiver.hh"
#include "messenger.hh"
#include "repl-manager.hh"

class ClientManager : public MessageReceiver {
public:
  ClientManager(const Messenger& messenger,
                std::shared_ptr<ReplManager> replManager);

  void
  startReceiveLoop() final;

  void
  handleMessage(const int& srcNodeId,
                const Message& receivedMessage,
                const Messenger::Connection& connection) final;

  void
  addConnection(Messenger::Connection connection);

private:
  bool m_isUp = true;

  void
  receivePendingMessages(bool& isUp);

  std::mutex m_connectionMutex;
  std::mutex m_isUpMutex;

  std::list<Messenger::Connection> m_clientConnections;
};
