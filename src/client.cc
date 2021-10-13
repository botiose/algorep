#include "client.hh"

void
Client::connect(int argc, char* argv[]) {
  int rank;
  int clusterSize;
  m_messenger.start(argc, argv, rank, clusterSize);

  m_messenger.connect(m_serverConnection);

  Message message;
  m_messenger.setMessage(ClientCode::CONNECT, message);

  m_messenger.send(0, message, m_serverConnection);
}

void
Client::sendData(const std::string& data) const {
  Message message;
  m_messenger.setMessage(ClientCode::REPLICATE, data, message);

  m_messenger.send(0, message, m_serverConnection);

  // wait for replication message
}

void
Client::disconnect() {
  Message message;
  m_messenger.setMessage(ClientCode::DISCONNECT, message);

  m_messenger.send(0, message, m_serverConnection);
 
  m_messenger.disconnect(m_serverConnection);

  m_messenger.stop();
}
