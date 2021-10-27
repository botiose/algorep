#pragma once

#include <string>
#include <unordered_map>

enum class MessageTag {
  LEADER_ELECTION = 0,
  CONSENSUS = 1,
  REPL = 2,
  FAILURE_DETECTION = 3,
  CLIENT = 4,
  SIZE = 5
};

enum class LeaderElectionCode {
  SHUTDOWN = 0,
  ELECTION = 1,
  ALIVE = 2,
  VICTORY = 3
};

enum class ConsensusCode {
  SHUTDOWN = 0,
  PREPARE = 1,
  PROMISE = 2,
  PROPOSE = 3,
  ACCEPT = 4,
  ACCEPTED = 5
};

enum class FailureCode {
  SHUTDOWN = 0,
  PING = 1,
  STATE = 2,
  STATE_UPDATED = 3,
  RECOVERED = 4
};

enum class ClientCode {
  SHUTDOWN = 0,
  PORT = 1,
  CONNECT = 2,
  DISCONNECT = 3,
  REPLICATE = 4,
  SUCCESS = 5
};

enum class ReplCode {
  SHUTDOWN = 0,
  START = 1,
  SPEED_LOW = 2,
  SPEED_MEDIUM = 3,
  SPEED_HIGH = 4,
  CRASH = 5,
  RECOVER = 6
};

static std::unordered_map<std::string, ReplCode> const replParseMap = {
    {"shutdown", ReplCode::SHUTDOWN},
    {"start", ReplCode::START},
    {"speed-low", ReplCode::SPEED_LOW},
    {"speed-medium", ReplCode::SPEED_MEDIUM},
    {"speed-high", ReplCode::SPEED_HIGH},
    {"crash", ReplCode::CRASH},
    {"recover", ReplCode::RECOVER}};

template <typename T>
MessageTag
getTagFromCode();
