#include <iostream>
#include <thread>

#include <json.hpp>

#include "client-manager.hh"
#include "consensus-manager.hh"
#include "receiver-manager.hh"

#define LOOP_SLEEP_DURATION 50

ClientManager::ClientManager(Messenger& messenger,
                             std::shared_ptr<ReceiverManager> receiverManager)
    : MessageReceiver(messenger, managedTag, receiverManager) {
}

void
ClientManager::handleMessage(const int& srcNodeId,
                             const Message& receivedMessage,
                             const Messenger::Connection& connection) {
  ClientCode code = receivedMessage.getCode<ClientCode>();
  switch (code) {
  case ClientCode::REPLICATE: {
    const std::string& data = receivedMessage.getData();

    nlohmann::json dataJson = nlohmann::json::parse(data);
    std::string value;
    dataJson.at("value").get_to(value);

    bool consensusReached;
    ConsensusManager::startConsensus(m_messenger, value, consensusReached);

    if (consensusReached == true) {
      Message message;
      m_messenger.setMessage(ClientCode::SUCCESS, message);
      m_messenger.send(srcNodeId, message, connection);
    }

    break;
  }
  }
}

void
ClientManager::receivePendingMessages(bool& isUp) {
  std::unique_lock<std::mutex> lock(m_connectionMutex);

  int i = 0;

  auto connection = m_clientConnections.begin();

  // iterate over every client connection
  while (isUp == true && connection != m_clientConnections.end()) {
    int srcNodeId;
    Message receivedMessage;
    bool messageReceived;

    m_messenger.receiveWithTag(MessageTag::CLIENT,
                               messageReceived,
                               srcNodeId,
                               receivedMessage,
                               *connection);

    if (messageReceived == true) {
      // message code 0 is SHUTDOWN for all message tags
      isUp = receivedMessage.getCodeInt() != 0;

      if (isUp == true) {
        if (receivedMessage.getCode<ClientCode>() != ClientCode::DISCONNECT) {
          this->handleMessage(srcNodeId, receivedMessage, *connection);
        } else {
          m_messenger.disconnect(*connection);
          connection = m_clientConnections.erase(connection);
        }
      }
    }

    i += 1;
    connection++;
  }
}

void
acceptConnection(Messenger& messenger,
                 std::list<Messenger::Connection>& clientConnections,
                 std::mutex& connectionMutex) {
  messenger.publish();

  std::cout << "accepting connections" << std::endl;
  bool isUp = true;
  while (isUp == true) {
    Messenger::Connection connection;
    messenger.acceptConnection(connection);

    int srcNodeId;
    Message message;
    messenger.receiveBlock(srcNodeId, message, connection);

    {
      std::unique_lock<std::mutex> lock(connectionMutex);

      clientConnections.push_back(connection);
    }

    messenger.send(messenger.getRank(), message);

    isUp = message.getCode<ClientCode>() != ClientCode::SHUTDOWN;
  }

  messenger.unpublish();
}

void
ClientManager::startReceiver() {
  m_acceptConnThread = std::thread(acceptConnection,
                                   std::ref(m_messenger),
                                   std::ref(m_clientConnections),
                                   std::ref(m_connectionMutex));

  bool isUp = true;
  while (isUp == true) {
    this->receivePendingMessages(isUp);

    std::this_thread::sleep_for(std::chrono::milliseconds(LOOP_SLEEP_DURATION));
  }
}

void
ClientManager::stopReceiver() {
  Messenger::Connection connection;
  m_messenger.selfConnect(connection);

  int dstNodeId = 0;
  Message message;
  m_messenger.setMessage(ClientCode::SHUTDOWN, message);
  m_messenger.send(dstNodeId, message, connection);

  m_acceptConnThread.join();

  // TODO disconnect all connections
}
