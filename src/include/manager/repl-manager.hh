/**
 * @file   repl-manager.hh
 * @author Otiose email
 * @date   Thu Oct 28 10:40:10 2021
 *
 * @brief  Declares the ReplManager class.
 *
 * This class encapsulates all repl related calls. It derives from the
 * MessageReceiver class and handles messages with the MessageTag::REPL
 * tag.
 *
 * As opposed to other receivers the repl receiver reads messages from a file
 * given in the constructor instead of reading from the messenger. This was done
 * to decouple as much as possible the repl logic from the messaging logic. And
 * to ecapsulate repl logic as much as possible to allow for easier removal of
 * this one for a theoretical actual implementation.
 *
 */
#pragma once

#include <mutex>
#include <memory>
#include <condition_variable>
#include <string>

#include "message-receiver.hh"
#include "messenger.hh"

class ReplManager : public MessageReceiver {
public:
  inline static MessageTag managedTag = MessageTag::REPL;

  /** 
   * @breif ReplManager constructor.
   * 
   * @param[in] messenger node's messenger
   * @param receiverManager receiver manager
   * @param replFilePath repl file path
   * 
   * @return 
   */
  ReplManager(Messenger& messenger,
              std::shared_ptr<ReceiverManager> receiverManager, 
              const std::string& replFilePath);

  /** 
   * @brief Starts the receiver
   * 
   */
  void
  startReceiver() final;

  /** 
   * @brief Handles repl message.
   * 
   * @param[in] srcNodeId not set for the repl manager
   * @param[in] receivedMessage message read
   */
  void
  handleMessage(const int& srcNodeId,
                const Message& receivedMessage,
                const Messenger::Connection& connection = {
                    MPI_COMM_WORLD}) final;

  /** 
   * @brief Puts the calling thread to sleep.
   * 
   * This function is called on all receiver (except this one). Depending on
   * whether the repl receiver for the current node received a 'crash' or one of
   * the three speeds it will pause the calling thread until either 'recover' is
   * received or for a given amount of time, respectively.
   * 
   */
  void
  sleep();
private:
  std::string m_replFilePath;

  std::mutex m_mutex;
  std::condition_variable m_blockConditional;

  bool m_shouldBlock = true;
  ReplCode m_speed = ReplCode::SPEED_HIGH;
};
