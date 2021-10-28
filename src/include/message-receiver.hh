/**
 * @file   message-receiver.hh
 * @author Otiose email
 * @date   Thu Oct 28 07:28:37 2021
 *
 * @brief  Defines the MessageReceiver class
 *
 * The MessageReceiver class is a pure virtual class which handles all logic
 * related to receiving and handling messages of a given tag. It is currently
 * derived by the 5 message tag managers located in src/manager.
 *
 */
#pragma once

#include "message-info.hh"
#include "messenger.hh"
#include "message.hh"

class ReceiverManager;

class MessageReceiver {
public:
  /**
   * @brief MessageReceiver contructor
   *
   * @param[in] messenger node's messenger
   * @param[in] tag tag from which to receive messages from
   * @param[in] receiverManager receiver manager
   */
  MessageReceiver(Messenger& messenger,
                  const MessageTag& tag,
                  std::shared_ptr<ReceiverManager> receiverManager);

  /**
   * @brief Starts the receiving loop.
   *
   */
  virtual void
  startReceiver();

  /**
   * @brief Handles the given message passed in argument.
   *
   * @param[in] srcNodeId node id from which the message originated
   * @param receivedMessage the message received
   * @param connection the connection from which is was received
   */
  virtual void
  handleMessage(const int& srcNodeId,
                const Message& receivedMessage,
                const Messenger::Connection& connection = {MPI_COMM_WORLD}) = 0;

  /**
   * @brief Stops the receiving loop
   *
   */
  virtual void
  stopReceiver(){};

protected:
  virtual void
  init(){};

  std::shared_ptr<ReceiverManager> m_receiverManager;

  Messenger& m_messenger;

  MessageTag m_tag;
};
