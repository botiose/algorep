#pragma once

#include "message-receiver.hh"
#include "messenger.hh"

class ElectionManager : public MessageReceiver {
public:
  ElectionManager(const Messenger& messenger);

  /**
   * @brief Handles messages tagged for leader election.
   *
   * This function encapsulates all leader election logic. Leader election is
   * done through the usage of the Bully Algorithm. In this algorithm the leader
   * is set to be the live node with the highest ID. Please visit
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

private:
  int m_leaderNodeId;
};
