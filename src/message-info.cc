#include "message.hh"

template <>
MessageTag
getTagFromCode<LeaderElectionCode>() {
  return MessageTag::LEADER_ELECTION;
}

template <>
MessageTag
getTagFromCode<ConsensusCode>() {
  return MessageTag::CONSENSUS;
}
