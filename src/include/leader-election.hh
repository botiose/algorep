/**
 * @file   leader-election.hh
 * @author Otiose email
 * @date   Thu Sep 30 14:54:37 2021
 * 
 * @brief  Defines the leader_election encapsulating leader election logic.
 * 
 */
#pragma once

#include "messenger.hh"

namespace leader_election {
/**
 * @brief Elects a leader from a nodes in the cluster.
 *
 * Leader election is done through the usage of the Bully Algorithm. In this
 * algorithm the leader is set to be the live node with the highest ID. Please
 * visit https://en.wikipedia.org/wiki/Bully_algorithm for more information.
 *
 * @param[in] messenger Messenger of the node starting the election.
 * @param[in] nodeId Id of the node starting the election.
 * @param[in] clusterSize Number of nodes in the current cluster.
 * @param[out] leaderNodeId Elected leader.
 */
void
startElection(const Messenger& messenger,
              const int& nodeId,
              const int& clusterSize,
              int& leaderNodeId);
}; // namespace leader_election
