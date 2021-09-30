/**
 * @file   message-codes.hh
 * @author Otiose email
 * @date   Thu Sep 30 14:31:58 2021
 * 
 * @brief  Defines all message tags and codes
 */
#pragma once

enum class MessageTag { LEADER_ELECTION, CONSENSUS, LOG, REPL, SIZE };

enum class LeaderElectionCode { ELECTION, ALIVE, VICTORY, SIZE };

enum class ReplCode { START, SHUTDOWN, SIZE };
