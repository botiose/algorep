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

class Node {
public:
  /**
   * @brief Defaulted contructor of the Node class.
   */
  Node() = default;

  /**
   * @brief Starts the node's messenger and initializes cluster info members.
   */
  void
  startMessenger();

  /**
   * @brief Stops the node's messenger.
   */
  void
  stopMessenger() const;

  /**
   * @brief Starts the receive loop.
   *
   * This is the main loop of node in which all incoming messages are received
   * and handled.
   *
   */
  void
  startReceiveLoop();

private:
  Messenger m_messenger; /**< local messenger instance */

  int m_nodeId;       /**< id of the current node */
  int m_clusterSize;  /**< number of nodes in the cluster */
  int m_leaderNodeId; /**< id of the leader node */
};
