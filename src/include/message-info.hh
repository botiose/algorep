#pragma once

#include <string>
#include <unordered_map>

enum class MessageTag {
  LEADER_ELECTION,
  CONSENSUS,
  REPL,
  FAILURE_DETECTION,
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

enum class FailDetectionCode { SHUTDOWN, SIZE };

enum class ClientCode {
  SHUTDOWN,
  PORT,
  CONNECT,
  DISCONNECT,
  REPLICATE,
  SUCCESS,
  SIZE
};

enum class ReplCode {
  SHUTDOWN,
  START,
  SPEED_LOW,
  SPEED_MEDIUM,
  SPEED_HIGH,
  CRASH,
  RECOVER,
  SIZE
};

static std::unordered_map<std::string, ReplCode> const replParseMap = {
    {"shutdown", ReplCode::SHUTDOWN},
    {"start", ReplCode::START},
    {"speed-low", ReplCode::SPEED_LOW},
    {"speed-medium", ReplCode::SPEED_MEDIUM},
    {"speed-high", ReplCode::SPEED_HIGH},
    {"speed-crash", ReplCode::CRASH},
    {"speed-recover", ReplCode::RECOVER}};

template <typename T>
MessageTag
getTagFromCode();
