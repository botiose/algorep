/**
 * @file   message-info.hh
 * @author Otiose email
 * @date   Thu Oct 28 09:17:24 2021
 *
 * @brief  Defines message tag and message codes enums and utilities.
 */
#pragma once

#include <string>
#include <unordered_map>
#include <vector>

enum class MessageTag {
  LEADER_ELECTION = 0,
  CONSENSUS = 1,
  REPL = 2,
  FAILURE_DETECTION = 3,
  CLIENT = 4,
  SIZE = 5
};

static std::vector<std::string> const messageTagMap = {
    "ELECTION", "CONSENSUS", "REPL", "FAILURE", "CLIENT"};

enum class LeaderElectionCode {
  SHUTDOWN = 0,
  ELECTION = 1,
  ALIVE = 2,
  VICTORY = 3
};

static std::vector<std::string> const electionMap = {
    "SHUTDOWN", "ELECTION", "ALIVE", "VICTORY"};

enum class ConsensusCode {
  SHUTDOWN = 0,
  PREPARE = 1,
  PROMISE = 2,
  PROPOSE = 3,
  ACCEPT = 4,
  ACCEPTED = 5
};

static std::vector<std::string> const consensusMap = {
    "SHUTDOWN", "PREPARE", "PROMISE", "PROPOSE", "ACCEPT", "ACCEPTED"};

enum class FailureCode {
  SHUTDOWN = 0,
  PING = 1,
  STATE = 2,
  STATE_UPDATED = 3,
  RECOVERED = 4
};

static std::vector<std::string> const failureMap = {
    "SHUTDOWN", "PING", "STATE", "STATE_UPDATED", "RECOVERED"};

enum class ClientCode {
  SHUTDOWN = 0,
  PORT = 1,
  DISCONNECT = 2,
  REPLICATE = 3,
  SUCCESS = 4
};

static std::vector<std::string> const clientMap = {
    "SHUTDOWN", "PORT", "DISCONNECT", "REPLICATE", "SUCCESS"};

enum class ReplCode {
  SHUTDOWN = 0,
  START = 1,
  SPEED_LOW = 2,
  SPEED_MEDIUM = 3,
  SPEED_HIGH = 4,
  CRASH = 5,
  RECOVER = 6
};

static std::vector<std::string> const replMap = {"SHUTDOWN",
                                                 "START",
                                                 "SPEED_LOW",
                                                 "SPEED_MEDIUM",
                                                 "SPEED_HIGH",
                                                 "CRASH",
                                                 "RECOVER"};

static std::vector<std::vector<std::string>> const codeMap = {
    electionMap, consensusMap, replMap, failureMap, clientMap};

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
