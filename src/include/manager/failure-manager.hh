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

  struct Context {
    std::mutex mutex; // TODO rename to nodeStateMutex
    std::vector<timePoint> timeStamps;
    std::vector<bool> isAlive;
    int curRecoveryId = -1;
    std::mutex curRecoveryIdMutex;
    std::mutex clientConnMutex;
    bool blockClientConn = false;
    std::condition_variable blockClientConnCond;
    bool clientConnBlocked = false;
    std::condition_variable clientConnBlockedCond;
  };

private:
  void
  init() final;

  Context m_context;
};
