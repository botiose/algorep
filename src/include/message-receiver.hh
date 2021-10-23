#pragma once

#include "message-info.hh"
#include "messenger.hh"
#include "message.hh"

class ReceiverManager;

class MessageReceiver {
public:
  MessageReceiver(Messenger& messenger,
                  const MessageTag& tag,
                  std::shared_ptr<ReceiverManager> receiverManager);

  virtual void
  startReceiver();

  virtual void
  handleMessage(const int& srcNodeId,
                const Message& receivedMessage,
                const Messenger::Connection& connection = {MPI_COMM_WORLD}) = 0;

  virtual void
  stopReceiver(){};

protected:
  virtual void
  init(){};

  std::shared_ptr<ReceiverManager> m_receiverManager;

  Messenger& m_messenger;

  MessageTag m_tag;
};
