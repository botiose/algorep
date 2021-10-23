#pragma once

#include <mutex>
#include <chrono>

#include "message-receiver.hh"
#include "messenger.hh"
#include "repl-manager.hh"

using timePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

class ElectionManager : public MessageReceiver {
public:
  inline static MessageTag managedTag = MessageTag::LEADER_ELECTION;

  ElectionManager(Messenger& messenger,
                  std::shared_ptr<ReceiverManager> receiverManager);

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
  getLeaderNodeId();

  void
  startElection();

  void
  init() final;

private:
  std::mutex m_mutex;

  int m_leaderNodeId = -1;
  bool m_aliveReceived = false;

  timePoint m_start;
};
