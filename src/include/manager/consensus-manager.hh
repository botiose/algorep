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
#include <mutex>

#include "messenger.hh"
#include "message-receiver.hh"
#include "repl-manager.hh"
#include "log-file-manager.hh"

class ConsensusManager : public MessageReceiver {
public:
  inline static MessageTag managedTag = MessageTag::CONSENSUS;

  ConsensusManager(Messenger& messenger,
                   std::shared_ptr<ReceiverManager> receiverManager,
                   LogFileManager& logFileManager);
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
  void
  startConsensus(const std::string& value, 
                 bool& consensusReached);

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
  handleMessage(const int& srcNodeId,
                const Message& receivedMessage,
                const Messenger::Connection& connection) final;

  struct Context {
    Context() = default;

    int roundId = -1;
    int maxAcceptedId = -1;     /**< max id found in promise responses */
    bool valueAccepted = false; /**< whether a value was accepted this round */
    int acceptedId = -1;        /**< id of the associated accepted round */
    std::string acceptedValue = "";  /**< value of the associated accepted round */
    int promiseCount = 0;
    int acceptCount = 0;
  };

  void
  stopReceiver() final;

private:
  LogFileManager& m_logFileManager;

  std::mutex m_mutex;
  Context m_context; /**< current round state/context */

  int m_maxRoundId = -1;
};
