#pragma once

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
};
