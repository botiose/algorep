#pragma once

#include <vector>
#include <chrono>
#include <thread>

#include "message-receiver.hh"
#include "messenger.hh"
#include "repl-manager.hh"

using timePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

class FailureManager : public MessageReceiver {
public:
  inline static MessageTag managedTag = MessageTag::FAILURE_DETECTION;

  FailureManager(Messenger& messenger,
                 std::shared_ptr<ReceiverManager> receiverManager);

  void
  handleMessage(const int& srcNodeId,
                const Message& receivedMessage,
                const Messenger::Connection& connection) final;

  void
  stopReceiver() final;

private:
  void
  init() final;

  std::mutex m_mutex;

  std::thread m_pingThread;

  std::vector<timePoint> m_timeStamps;
  std::vector<bool> m_isAlive;
};
