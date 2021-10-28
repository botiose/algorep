/**
 * @file   client.hh
 * @author Otiose email
 * @date   Thu Oct 28 07:54:09 2021
 * 
 * @brief  Defines the Client class.
 * 
 * This class encapsulates all logic related to clients.
 * 
 */


#pragma once

#include "receiver-manager.hh"
#include "messenger.hh"

class Client {
public:
  /** 
   * @brief Client class' default constructor
   * 
   */
  Client() = default;

  /** 
   * @brief Initializes the client for communication with the server.
   * 
   * The id of the current client is passed through the command line arguments.
   * This id is used for reading the correct generated command entries from the 
   * clients respective command file located in etc/client.
   * 
   * @param[in] argc number of command line arguments
   * @param[in] argv command line arguments
   */
  void
  init(int argc, char** argv);

  /** 
   * @brief Closes communications with the server and destroys the instance.
   * 
   */
  void
  destroy();

  /** 
   * @brief Starts sending the respective command for the client to the server.
   * 
   * This function exists only once all random commands of the clients have been
   * replicated on the server.
   * 
   */
  void
  replicateCommands();

  /**
   * @brief This functions is used for the shutdown-client binary.
   *
   * This functions connects to the blocking accept call of the server and
   * issues a shutdown.
   *
   * @param argc
   * @param argv
   */
  void
  shutdownServer(int argc, char* argv[]);

private:
  Messenger m_messenger;

  Messenger::Connection m_serverConnection;

  std::shared_ptr<ReceiverManager> m_receiverManager;

  std::string m_baseDir;
};
