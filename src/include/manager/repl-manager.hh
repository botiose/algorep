#pragma once

#include <mutex>
#include <memory>

#include "message-receiver.hh"
#include "messenger.hh"

class ReplManager : public MessageReceiver {
public:
  inline static MessageTag managedTag = MessageTag::REPL;

  ReplManager(Messenger& messenger,
              std::shared_ptr<ReceiverManager> receiverManager);

  void
  startReceiver() final;

  void
  handleMessage(const int& srcNodeId,
                const Message& receivedMessage,
                const Messenger::Connection& connection = {
                    MPI_COMM_WORLD}) final;

  bool
  hasStarted();

  bool
  hasCrashed();

  ReplCode
  getSpeed();
private:
  std::mutex m_mutex;

  bool m_hasStarted = false;
  bool m_hasCrashed = false;
  ReplCode m_speed = ReplCode::SPEED_HIGH;
};
