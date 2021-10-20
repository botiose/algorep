#pragma once

#include <mutex>
#include <list>
#include <thread>
#include <string>

#include "message-receiver.hh"
#include "messenger.hh"
#include "repl-manager.hh"

class ClientManager : public MessageReceiver {
public:
  inline static MessageTag managedTag = MessageTag::CLIENT;

  ClientManager(Messenger& messenger,
                std::shared_ptr<ReceiverManager> receiverManager);

  void
  startReceiver() final;

  void
  handleMessage(const int& srcNodeId,
                const Message& receivedMessage,
                const Messenger::Connection& connection) final;

  void
  enableClientConn();

  void
  disableClientConn();
private:
  std::string m_port;
  std::string m_nextNodePort;

  void receivePendingMessages(bool& isUp);

  std::mutex m_connectionMutex;

  std::list<Messenger::Connection> m_clientConnections;

  std::thread m_acceptConnThread;
};
