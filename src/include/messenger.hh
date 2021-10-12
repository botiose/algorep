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

#include <mpi.h>

#include "message.hh"
#include "message-info.hh"

class Messenger {
public:
  struct Connection {
    MPI_Comm connection;
  };

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
  start(int argc, char** argv, int& rank, int& clusterSize);

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

  /**
   * @brief Initializes the message with the given code.
   *
   * This function initializes messages with additional information useful
   * during communication.
   *
   * @param code message code
   * @param message message to initialize
   */
  template <typename T>
  void
  setMessage(const T& code, Message& message) const;

  /**
   * @brief Initializes the message with the given code and data string.
   *
   * @param code message code
   * @param data message data
   * @param message message to initialize
   */
  template <typename T>
  void
  setMessage(const T& code, const std::string& data, Message& message) const;

  /**
   * @brief Sends the given message to the specified node.
   *
   * @param[in] dstNodeId destination node.
   * @param[in] message message to send.
   */
  void
  send(const int& dstNodeId,
       const Message& message,
       const Connection& connection = {MPI_COMM_WORLD}) const;

  /**
   * @brief Blocks until a message is received.
   *
   * @param[out] srcNodeId node id from which the message was received.
   * @param[out] message message received.
   */
  void
  receiveBlock(int& srcNodeId,
               Message& message,
               const Connection& connection = {MPI_COMM_WORLD}) const;

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
                      Message& message,
                      const Connection& connection = {MPI_COMM_WORLD}) const;

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
                 Message& message,
                 const Connection& connection = {MPI_COMM_WORLD}) const;

  /**
   * @brief Check whether a message with given tag is pending.
   *
   * @param[in] messageTag message tag.
   * @param[out] hasPending whether there's a pending message.
   */
  void
  hasPendingWithTag(const MessageTag& messageTag,
                    bool& hasPending,
                    const Connection& connection = {MPI_COMM_WORLD}) const;

  void
  probeTagBlock(const Connection& connection, MessageTag& messageTag) const;
  
  void
  publish();

  void
  unpublish();

  void
  acceptConnection(Connection& connection) const;

  bool
  getIsPublished() const;

  void
  selfConnect(Connection& connection) const;

  void
  connect(Connection& connection);

  void
  disconnect(Connection& connection);

private:
  void
  generateUniqueId(const int& nodeId, int& id) const;

  int m_rank; /**< rank of the current process */
  bool m_isPublished = false;
  char m_port[MPI_MAX_PORT_NAME];
};

#include "messenger.hxx"
