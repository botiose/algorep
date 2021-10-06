#pragma once

#include <string>

#include "messenger.hh"

class ConsensusManager {
public:
  static void
  startConsensus(const Messenger& messenger,
                 const int& clusterSize,
                 const std::string& value);
  void
  handleConsensusMessage(const Messenger& messenger,
                         const int& clusterSize,
                         const int& srcNodeId,
                         const Message& receivedMessage);

private:
  struct ConsensusContext {
    int maxId = -1;
    bool valueAccepted = false;
    int acceptedId = -1;
    std::string acceptedValue;

    void
    reset();
  };
  
  ConsensusContext m_context;
};
