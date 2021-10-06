#pragma once

enum class MessageTag { LEADER_ELECTION, CONSENSUS, REPL, SIZE };

enum class LeaderElectionCode { ELECTION, ALIVE, VICTORY, SIZE };

enum class ConsensusCode { PREPARE, PROMISE, PROPOSE, ACCEPT, ACCEPTED, SIZE };

template <typename T>
MessageTag
getTagFromCode();
