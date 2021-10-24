#pragma once

#include <vector>
#include <chrono>
#include <thread>
#include <condition_variable>

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

  void
  sleep();
private:
  void
  init() final;

  std::mutex m_mutex;;

  std::vector<timePoint> m_timeStamps;
  std::vector<bool> m_isAlive;

  std::mutex m_curRecoveryIdMutex;
  int m_curRecoveryId = -1;

  bool m_blockClientConn = false;
  std::condition_variable m_blockClientConnCond;

  bool m_clientConnBlocked = false;
  std::condition_variable m_clientConnBlockedCond;
};
