#pragma once

#include <vector>
#include <thread>

#include "message-receiver.hh"

class ReceiverManager {
public:
  ReceiverManager() = default;

  void
  addReceiver(MessageReceiver& receiver);

  void
  startReceiverLoops();

  void
  waitForReceivers();

private:
  std::vector<MessageReceiver*> m_receivers;
  std::vector<std::thread> m_threads;
};
