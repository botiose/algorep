#pragma once

#include "receiver-manager.hh"
#include "messenger.hh"

class Client {
public:
  Client() = default;

  void
  connect(int argc, char** argv);

  void
  replicateCommands();

  void
  shutdownServer(int argc, char* argv[]);
  
  void
  disconnect();

private:
  Messenger m_messenger;

  Messenger::Connection m_serverConnection;

  std::shared_ptr<ReceiverManager> m_receiverManager;

  std::string m_baseDir;
};
