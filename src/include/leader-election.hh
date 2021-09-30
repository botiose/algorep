#pragma once

#include "messenger.hh"

namespace leader_election {
void
startElection(const Messenger& messenger,
              const int& nodeId,
              const int& clusterSize,
              int& leaderNodeId);
};
