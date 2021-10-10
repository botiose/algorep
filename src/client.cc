#include "client.hh"

void
Client::connect() {
  int rank;
  int clusterSize;
  m_messenger.start(rank, clusterSize);

  m_messenger.connect(m_serverConnection);

  // TODO send connect code
}

void
Client::disconnect() {
  // TODO send disconnect code
 
  m_messenger.disconnect(m_serverConnection);

  m_messenger.stop();
}
