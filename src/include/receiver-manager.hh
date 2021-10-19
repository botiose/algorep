#pragma once

#include <array>
#include <thread>
#include <memory>

#include "message-info.hh"
#include "message-receiver.hh"

class ReceiverManager {
public:
  ReceiverManager() = default;

  // TODO change this to use MessageTags
  template <typename T>
  void
  startReceiver(std::shared_ptr<T> receiver);

  void
  waitForReceiver(const MessageTag& tag);

  template <typename T>
  std::shared_ptr<T>
  getReceiver();

  void
  stopReceiver(const MessageTag& tag);

  void
  stopReceiverWait(const MessageTag& tag);

private:
  std::array<std::shared_ptr<MessageReceiver>,
             static_cast<int>(MessageTag::SIZE)>
      m_receivers;
  std::array<std::thread, static_cast<int>(MessageTag::SIZE)> m_threads;
};

#include "receiver-manager.hxx"
