#include "client.hh"

void
Client::connect(int argc, char* argv[]) {
  int rank;
  int clusterSize;
  m_messenger.start(argc, argv, rank, clusterSize);

  m_messenger.connect(m_serverConnection);

  // TODO send connect code
}

void
Client::disconnect() {
  // TODO send disconnect code
 
  m_messenger.disconnect(m_serverConnection);

  m_messenger.stop();
}
