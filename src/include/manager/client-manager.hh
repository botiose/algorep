/**
 * @file   client-manager.hh
 * @author Otiose email
 * @date   Thu Oct 28 11:25:53 2021
 * 
 * @brief  Declares the ClientManager class.
 * 
 * This class encapsulates all client related calls. It derives from the
 * MessageReceiver class and handles messages with the MessageTag::CLIENT
 * tag.
 * 
 */
#pragma once

#include <mutex>
#include <list>
#include <thread>
#include <string>

#include "message-receiver.hh"
#include "messenger.hh"
#include "repl-manager.hh"

class ClientManager : public MessageReceiver {
public:
  inline static MessageTag managedTag = MessageTag::CLIENT;
  /** 
   * @brief ClientManager constructor.
   * 
   * @param[in] messenger node's messenger
   * @param[in] receiverManager receiver manager
   * 
   * @return ClientManager instance
   */
  ClientManager(Messenger& messenger,
                std::shared_ptr<ReceiverManager> receiverManager);

  /** 
   * @brief Starts the receiver
   * 
   */
  void
  startReceiver() final;

  /** 
   * @brief Handles client related messages.
   * 
   * @param srcNodeId 
   * @param receivedMessage 
   * @param connection 
   */
  void
  handleMessage(const int& srcNodeId,
                const Message& receivedMessage,
                const Messenger::Connection& connection) final;

  /** 
   * @brief Enables client connection and communication.
   * 
   */
  void
  enableClientConn();
private:
  std::string m_port;
  std::string m_nextNodePort;

  void receivePendingMessages();

  Messenger::Connection m_clientConnection;
};
