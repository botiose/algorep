#pragma once

#include "receiver-manager.hh"
#include "messenger.hh"

class Client {
public:
  Client() = default;

  void
  init(int argc, char** argv);

  void
  destroy();

  void
  connect();

  void
  replicateCommands();

  void
  shutdownServer(int argc, char* argv[]);

private:
  Messenger m_messenger;

  Messenger::Connection m_serverConnection;

  std::shared_ptr<ReceiverManager> m_receiverManager;

  std::string m_baseDir;
};
