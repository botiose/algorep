#pragma once

enum class MessageTag {
  BULLY,
  PAXOS,
  LOG,
  RPC,
  SIZE
};

enum class MessageCode {
  BULLY_ELECTION,
  BULLY_ACTIVE,
  BULLY_VICTORY,
  SIZE
};
