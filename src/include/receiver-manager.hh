/**
 * @file   receiver-manager.hh
 * @author Otiose email
 * @date   Thu Oct 28 09:18:54 2021
 *
 * @brief  Defines the ReceiverManager class
 *
 * This class manages a list of instanciated receivers. The instanciated
 * receivers to manage are submited through the startReceiver() member function.
 * Each receiver is responsible for receiving and handling messages from a
 * single tag. More information on this can be found in the MessageReceiver base
 * class.
 *
 */
#pragma once

#include <array>
#include <thread>
#include <memory>

#include "message-info.hh"
#include "message-receiver.hh"

class ReceiverManager {
public:
  /** 
   * @brief Defaulted constructor
   * 
   */
  ReceiverManager() = default;

  /** 
   * @brief Logs and starts the receiver's receive loop.
   * 
   * @param receiver 
   */
  template <typename T>
  void
  startReceiver(std::shared_ptr<T> receiver);

  /** 
   * @brief Blocks until the receiver managing the given tag finishes executing.
   * 
   * @param[in] tag receiver tag
   */
  void
  waitForReceiver(const MessageTag& tag);

  /** 
   * @brief Gets the specified receiver.
   * 
   * @return shared pointer to the specified receiver.
   */
  template <typename T>
  std::shared_ptr<T>
  getReceiver();

  /** 
   * @brief Stops of the main loop of the given receiver.
   * 
   * @param tag tag of the receiver
   */
  void
  stopReceiver(const MessageTag& tag);

private:
  std::array<std::shared_ptr<MessageReceiver>,
             static_cast<int>(MessageTag::SIZE)>
      m_receivers;
  std::array<std::thread, static_cast<int>(MessageTag::SIZE)> m_threads;
  std::array<bool, static_cast<int>(MessageTag::SIZE)> m_isActive = {false};
};

#include "receiver-manager.hxx"
