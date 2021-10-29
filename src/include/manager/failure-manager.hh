/**
 * @file   failure-manager.hh
 * @author Otiose email
 * @date   Thu Oct 28 10:24:29 2021
 *
 * @brief  Declares the FailureManager class.
 *
 * This class encapsulates all failure related calls. It derives from the
 * MessageReceiver class and handles messages with the
 * MessageTag::FAILURE_DETECTION tag.
 *
 */
#pragma once

#include <vector>
#include <chrono>
#include <thread>
#include <condition_variable>

#include "message-receiver.hh"
#include "messenger.hh"
#include "repl-manager.hh"
#include "log-file-manager.hh"

using timePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

class FailureManager : public MessageReceiver {
public:
  inline static MessageTag managedTag = MessageTag::FAILURE_DETECTION;

  /** 
   * @brief FailureManager constructor.
   * 
   * @param[in] messenger node's messenger
   * @param[in] receiverManager receiver manager
   * @param[in] logFileManager log file manager
   * 
   * @return FailureManager instance.
   */
  FailureManager(Messenger& messenger,
                 std::shared_ptr<ReceiverManager> receiverManager,
                 LogFileManager& logFileManager);

  /** 
   * @brief Handles failure related messages
   * 
   * Failure detections is done through all-to-all heartbeating.
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
   * @brief Stops the receiver.
   * 
   */
  void
  stopReceiver() final;

  /** 
   * @brief Puts the current thread to sleep until notified otherwise.
   * 
   * This functions is used for the sake of pausing client connections while
   * a node's recovery is in process. This is done for the sake of preventing 
   * further modification of the cluster's log state while this one is being 
   * transfered to the recovering node.
   * 
   */
  void
  allowRecovery();

  void
  disallowRecovery();

  struct Context {
    std::mutex mutex; // TODO rename to nodeStateMutex
    std::vector<timePoint> timeStamps;
    std::vector<bool> isAlive;
    int curRecoveryId = -1;
    std::mutex curRecoveryIdMutex;
    std::mutex clientConnMutex;
    bool allowRecovery = true;
  };

private:
  void
  init() final;

  LogFileManager& m_logFileManager;

  Context m_context;

  bool m_pingThreadIsUp = true;
  std::thread m_pingThread;
};
