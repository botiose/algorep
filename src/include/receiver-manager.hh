#pragma once

#include <array>
#include <thread>
#include <memory>

#include "message-info.hh"
#include "message-receiver.hh"

class ReceiverManager {
public:
  ReceiverManager() = default;

  void
  startReceiver(std::shared_ptr<MessageReceiver> receiver);

  void
  waitForAllReceivers();

  void
  waitForReceiver(const MessageTag& tag);

private:
  std::array<bool, static_cast<int>(MessageTag::SIZE)> m_activeReceivers;
  std::array<std::shared_ptr<MessageReceiver>,
             static_cast<int>(MessageTag::SIZE)>
      m_receivers;
  std::array<std::thread, static_cast<int>(MessageTag::SIZE)> m_threads;
};
