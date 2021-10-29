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
#include <string>
#include <vector>
#include <mutex>

#include "message.hh"
#include "message-info.hh"

#define PORT_STRING_SIZE MPI_MAX_PORT_NAME

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
  start(int argc, char** argv);

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
  int
  getRank() const;

  /** 
   * @brief Gets the cluster size
   * 
   * 
   * @return cluster size
   */
  int
  getClusterSize() const;

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
   * @brief Send the given message to all nodes in the given range.
   * 
   * @param[in] message message to send
   * @param start start node id
   * @param end end node id (included)
   * @param includeSelf whether to also send the message to self
   */
  void
  broadcast(const Message& message,
            const int& start,
            const int& end = -1,
            const bool& includeSelf = false) const;

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
   * @brief Opens the given port for communication.
   * 
   * @param[in] port port to open
   */
  void
  openPort(std::string& port) const;

  /** 
   * @brief Closes the given port
   * 
   * @param[in] port port to close
   */
  void
  closePort(const std::string& port) const;

  /** 
   * @brief Publishes the given port for lookup.
   * 
   * @param[in] port port to publish
   */
  void
  publishPort(const std::string& port);

  // void
  // unpublishPort();

  /**
   * @brief Blocks until a connection is accepted.
   *
   * @param[in] port port in which to accept connections
   * @param[out] connection accepted connection
   */
  void
  acceptConnBlock(const std::string& port, Connection& connection) const;

  /** 
   * @brief Looks up the published port by the server.
   * 
   * This function is meant to be used by clients.
   * 
   * @param[out] port retrieved port
   */
  void
  lookupServerPort(std::string& port) const;

  /**
   * @brief Connects to the given port.
   *
   * This function is meant to be used by clients. This function will block
   * until a matching accept call is found on the given port.
   *
   * @param[in] port port in which to connect
   * @param[out] connection connection
   */
  void
  connect(const std::string& port, Connection& connection) const;

  /** 
   * @brief Disconnects from the given connection.
   * 
   * This function will block until a matching 'disconnect' call is found in the
   * given connection.
   * 
   * @param[in] connection connection to disconnect
   */
  void
  disconnect(Connection& connection) const;

  /** 
   * @brief Sets whether communication for the given node should be dropped.
   * 
   * This functions is currently only used in the FailureManager.
   * 
   * @param[in] nodeId node id
   * @param[in] isAlive whether the node is alive
   */
  void
  setNodeStatus(const int& nodeId, const bool& isAlive);

private:
  void
  generateUniqueId(const int& nodeId, int& id) const;

  int m_rank;
  int m_clusterSize;

  std::mutex m_mutex;
  std::vector<bool> m_processIsAlive;
};

#include "messenger.hxx"
