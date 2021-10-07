/**
 * @file   consensus-manager.hh
 * @author Otiose email
 * @date   Thu Oct  7 12:40:35 2021
 *
 * @brief  Declares the ConsensusManager class.
 *
 * This class encapsulates all consensus related calls.
 *
 */
#pragma once

#include <string>

#include "messenger.hh"

class ConsensusManager {
public:
  /**
   * @brief Start on consensus for the given value.
   *
   *
   * Along with handleConsensusMessage() this function implements the Paxos
   * algorithm. For more information on this algorithm please visit
   * https://en.wikipedia.org/wiki/Paxos_(computer_science) . This function is
   * only meant to be called by Proposer.
   *
   * @param[in] messenger messenger of the current node
   * @param[in] nodeId node id of the current node
   * @param[in] clusterSize number of nodes in the cluster
   * @param[in] value value get a consensus on
   */
  static void
  startConsensus(const Messenger& messenger,
                 const int& nodeId, // TODO remove
                 const int& clusterSize,
                 const std::string& value);

  /**
   * @brief Handles consensus related messages.
   *
   * Along with startConsensus() this function implements the Paxos
   * algorithm. For more information on this algorithm please visit
   * https://en.wikipedia.org/wiki/Paxos_(computer_science) . This function is
   * only meant to be called by Acceptors.
   *
   * @param[in] messenger
   * @param[in] clusterSize
   * @param[in] srcNodeId
   * @param[in] receivedMessage
   */
  void
  handleConsensusMessage(const Messenger& messenger,
                         const int& clusterSize,
                         const int& srcNodeId,
                         const Message& receivedMessage);

private:
  struct ConsensusContext {
    /**
     * @brief Constructor for the ConsensusContext() struct.
     *
     *
     * @return ConsensusContext() instance.
     */
    ConsensusContext();

    int maxId = -1;             /**< max id encoutered in the current round */
    bool valueAccepted = false; /**< whether a value was accepted this round */
    int acceptedId = -1;        /**< id of the associated accepted round */
    std::string acceptedValue;  /**< value of the associated accepted round */
  };

  ConsensusContext m_context; /**< current round state/context */
};