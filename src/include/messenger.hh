/**
 * @file   messenger.hh
 * @author Otiose email
 * @date   Thu Sep 30 14:33:41 2021
 *
 * @brief  Defines the Messenger class encapsulating the Open MPI API.
 *
 * To decrease the design complexity all calls to the Open MPI API are done
 * through this class.
 *
 */
#pragma once

#include "message.hh"
#include "message-info.hh"

class Messenger {
public:
  /**
   * @brief Default Messenger construct.
   *
   */
  Messenger() = default;

  /**
   * @brief Initializes the MPI API and gets process specific information.
   *
   * This function calls MPI_init() and queries the rank of the current process
   * and the size of the current universe.
   *
   * @param[out] rank rank of the process.
   * @param[out] clusterSize number of processes in the cluster.
   */
  void
  start(int& rank, int& clusterSize);

  /**
   * @brief Stops the MPI API by calling MPI_Finalize().
   *
   */
  void
  stop() const;

  /**
   * @brief Gets the rank of the current process.
   *
   * @param[out] rank rank of the process.
   */
  void
  getRank(int& rank) const;

  /**
   * @brief Gets the cluster size.
   *
   * @param[out] clusterSize number of processes in the cluster.
   */
  void
  getClusterSize(int& clusterSize) const;

  template <typename T>
  void
  setMessage(const T& code, Message& message) const;

  template <typename T>
  void
  setMessage(const T& code, const std::string& command, Message& message) const;

  /**
   * @brief Sends the given message to the specified node.
   *
   * @param[in] dstNodeId destination node.
   * @param[in] message message to send.
   */
  void
  send(const int& dstNodeId, const Message& message) const;

  /**
   * @brief Blocks until a message is received.
   *
   * @param[out] srcNodeId node id from which the message was received.
   * @param[out] message message received.
   */
  void
  receiveBlock(int& srcNodeId, Message& message) const;

  /**
   * @brief Blocks until a message of specified tag is received.
   *
   * @param[in] messageTag tag of the message.
   * @param[out] srcNodeId node id from which the message was received.
   * @param[out] message message received
   */
  void
  receiveWithTagBlock(const MessageTag& messageTag,
                      int& srcNodeId,
                      Message& message) const;

  /**
   * @brief Receives a pending message with given tag.
   *
   * An std::optional was not used for the sake of simplicity. The function
   * might be refactored if necessary.
   *
   * @param[in] messageTag messsage tag.
   * @param[out] messageReceived whether a message was received.
   * @param[out] srcNodeId node id from which the message originated.
   * @param[out] message optional message received.
   */
  void
  receiveWithTag(const MessageTag& messageTag,
                 bool& messageReceived,
                 int& srcNodeId,
                 Message& message) const;

  /**
   * @brief Check whether a message with given tag is pending.
   *
   * @param[in] messageTag message tag.
   * @param[out] hasPending whether there's a pending message.
   */
  void
  hasPendingWithTag(const MessageTag& messageTag, bool& hasPending) const;

private:
  int m_rank; /**< rank of the current process */
};

#include "messenger.hxx"
