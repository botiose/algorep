#pragma once

#include <string>

#include "messenger.hh"

namespace consensus {
void
getConsensus(const Messenger& messenger,
             const int& clusterSize,
             const std::string& value);
void
handleConsensusMessage();
}; // namespace consensus
