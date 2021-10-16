#pragma once

#include <vector>
#include <thread>
#include <memory>

#include "message-receiver.hh"

class ReceiverManager {
public:
  ReceiverManager() = default;

  void
  startReceiver(std::shared_ptr<MessageReceiver> receiver);

  void
  waitForReceivers();

private:
  std::vector<std::shared_ptr<MessageReceiver>> m_receivers;
  std::vector<std::thread> m_threads;
};
