#pragma once

enum class MessageTag {
  LEADER_ELECTION,
  CONSENSUS,
  LOG,
  REPL,
  SIZE
};

enum class LeaderElectionCode {
  ELECTION,
  ALIVE,
  VICTORY,
  SIZE
};

enum class RpcCode {
  START,
  SHUTDOWN,
  SIZE
};
