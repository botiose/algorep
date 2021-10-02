#include "message.hh"

template <>
MessageTag
getTagFromCode<LeaderElectionCode>() {
  return MessageTag::LEADER_ELECTION;
}
