#pragma once

enum class MessageTag {
  LEADER_ELECTION,
  CONSENSUS,
  REPL,
  FAIL_DETECTION,
  CLIENT,
  SIZE
};

enum class LeaderElectionCode { SHUTDOWN, ELECTION, ALIVE, VICTORY, SIZE };

enum class ConsensusCode {
  SHUTDOWN,
  PREPARE,
  PROMISE,
  PROPOSE,
  ACCEPT,
  ACCEPTED,
  SIZE
};

enum class ReplCode { SHUTDOWN, START, SPEED, CRASH, RECOVER, SIZE };

enum class FailDetectionCode { SHUTDOWN, SIZE };

enum class ClientCode {
  SHUTDOWN,
  CONNECT,
  DISCONNECT,
  REPLICATE,
  SUCCESS,
  SIZE
};

template <typename T>
MessageTag
getTagFromCode();
