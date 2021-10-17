#pragma once

#include <mutex>

#include "message-receiver.hh"
#include "messenger.hh"

class ReplManager : public MessageReceiver {
public:
  ReplManager(const Messenger& messenger);

  void
  startReceiveLoop() final;

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
