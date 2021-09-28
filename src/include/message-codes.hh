#pragma once

enum class MessageTag {
  BULLY,
  PAXOS,
  LOG,
  RPC,
  SIZE
};

enum class BullyCode {
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
