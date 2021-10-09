/**
 * @file   node.hh
 * @author Otiose email
 * @date   Thu Sep 30 11:14:57 2021
 *
 * @brief  Defines the Node class.
 *
 * The Node class encapsulates all logic relating to computational nodes.
 * Nodes communicate through the m_messenger member which is defined in the
 * messenger.hh header file.
 *
 */
#pragma once

#include "messenger.hh"
#include "consensus-manager.hh"

class Node {
public:
  /**
   * @brief Defaulted contructor of the Node class.
   */
  Node() = default;

  void
  init();

  void
  destroy() const;

  /**
   * @brief Starts the receive loop.
   *
   * This is the main loop of node in which all incoming messages are received
   * and handled.
   *
   */
  void
  startMainLoops();

  /**
   * @brief returns whether the current node is a leader or not.
   *
   * @return whether the current node is a leader.
   */
  bool
  isLeader() const;

  /**
   * @brief replicates the given data along all nodes in the cluster.
   *
   * @param[in] data data to replicate.
   */
  void
  replicateData(const std::string& data) const;

private:
  Messenger m_messenger;               /**< local messenger instance */
  ConsensusManager m_consensusManager; /**< manager for consensus logic */

  int m_nodeId;       /**< id of the current node */
  int m_clusterSize;  /**< number of nodes in the cluster */
  int m_leaderNodeId; /**< id of the leader node */
};
