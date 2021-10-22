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

  ReplManager(Messenger& messenger,
              std::shared_ptr<ReceiverManager> receiverManager, 
              const std::string& replFilePath);

  void
  startReceiver() final;

  void
  handleMessage(const int& srcNodeId,
                const Message& receivedMessage,
                const Messenger::Connection& connection = {
                    MPI_COMM_WORLD}) final;

  void
  sleep();
private:
  std::string m_replFilePath;

  std::mutex m_mutex;
  std::condition_variable m_blockConditional;

  bool m_shouldBlock = true;
  ReplCode m_speed = ReplCode::SPEED_HIGH;
};
