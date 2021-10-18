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

#include <thread>

#include "receiver-manager.hh"
#include "client-manager.hh"
#include "election-manager.hh"
#include "messenger.hh"

class Node {
public:
  /**
   * @brief Defaulted contructor of the Node class.
   */
  Node() = default;

  void
  init(int argc, char** argv);

  void
  destroy() const;

  /**
   * @brief returns whether the current node is a leader or not.
   *
   * @return whether the current node is a leader.
   */
  bool
  isLeader() const;

  void
  enableClientCommunication();

  void
  disableClientCommunication();

private:
  ReceiverManager m_receiverManager;

  std::shared_ptr<ReplManager> m_replManager;
  std::shared_ptr<ClientManager> m_clientManager;
  std::shared_ptr<ElectionManager> m_electionManager;

  Messenger m_messenger;               /**< local messenger instance */

  // TODO remove
  int m_nodeId;       /**< id of the current node */
  int m_clusterSize;  /**< number of nodes in the cluster */

  std::thread m_acceptConnThread;
};
