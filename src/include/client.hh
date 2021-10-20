#pragma once

#include "messenger.hh"

class Client {
public:
  Client() = default;

  void
  connect(int argc, char** argv);

  void
  shutdownServer(int argc, char* argv[]);
  
  void
  replicate(const std::string& data) const;

  void
  disconnect();

private:
  Messenger m_messenger;

  Messenger::Connection m_serverConnection;
};
