#include <thread>

#include "consensus.hh"

#define UNUSED(x) (void)(x)

#define PROMISE_WAIT_DURATION 60

namespace consensus {
void
getConsensus(const Messenger& messenger,
             const int& nodeId,
             const int& clusterSize,
             const std::string& value) {
  UNUSED(nodeId);
  UNUSED(clusterSize);
  UNUSED(value);
  Message prepare;
  messenger.setMessage(ConsensusCode::PREPARE, prepare);

  for (int i = 0; i < clusterSize; i++) {
    if (i == nodeId) {
      continue;
    }

    messenger.send(i, prepare);
  }

  // using namespace std::chrono_literals;
  std::this_thread::sleep_for(std::chrono::seconds(PROMISE_WAIT_DURATION));

  bool messageReceived;

  do {
    int srcNodeId;
    Message promise;
    messenger.receiveWithTag(
        MessageTag::CONSENSUS, srcNodeId, messageReceived, promise);

    if (messageReceived == true) {
      // TODO count promises with correct id
    }

  } while (messageReceived == true)
}

void
handleConsensusMessage() {
}
} // namespace consensus
