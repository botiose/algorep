#pragma once

#include <mutex>

#include "message-receiver.hh"
#include "messenger.hh"
#include "repl-manager.hh"

class ElectionManager : public MessageReceiver {
public:
  inline static MessageTag managedTag = MessageTag::LEADER_ELECTION;

  ElectionManager(Messenger& messenger,
                  std::shared_ptr<ReceiverManager> receiverManager);

  void
  triggerElection();

  void
  waitForVictor();

  /**
   * @brief Handles messages tagged for leader election.
   *
   * This function encapsulates all leader election logic. Leader election
   * is done through the usage of the Bully Algorithm. In this algorithm the
   * leader is set to be the live node with the highest ID. Please visit
   * https://en.wikipedia.org/wiki/Bully_algorithm for more information.
   *
   * @param[in] messenger Messenger of the node starting the election.
   * @param[in] nodeId Id of the node starting the election.
   * @param[in] clusterSize Number of nodes in the current cluster.
   * @param[in] receivedMessage Message to handle
   * @param[out] leaderNodeId Elected leader.
   */
  void
  handleMessage(const int& srcNodeId,
                const Message& receivedMessage,
                const Messenger::Connection& connection);

  void
  stopReceiver() final;

  // thread safety left to the user
  int
  getLeaderNodeId() const;

private:
  void
  startElection();

  int m_leaderNodeId = -1;
};
