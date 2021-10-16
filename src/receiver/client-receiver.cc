#include <iostream>
#include <thread>

#include <json.hpp>

#include "client-receiver.hh"
#include "consensus-manager.hh"

#define LOOP_SLEEP_DURATION 50

ClientReceiver::ClientReceiver(const Messenger& messenger)
    : MessageReceiver(messenger, MessageTag::CLIENT) {
}

void
ClientReceiver::handleMessage(const int& srcNodeId,
                              const Message& receivedMessage,
                              const Messenger::Connection& connection) {
  ClientCode code = receivedMessage.getCode<ClientCode>();
  switch(code) {
  case ClientCode::REPLICATE: {
    const std::string& data = receivedMessage.getData();

    nlohmann::json dataJson = nlohmann::json::parse(data);
    std::string value;
    dataJson.at("value").get_to(value);

    bool consensusReached;
    ConsensusManager::startConsensus(m_messenger, value, consensusReached);
    
    if(consensusReached == true) {
      Message message;
      m_messenger.setMessage(ClientCode::SUCCESS, message);
      m_messenger.send(srcNodeId, message, connection);
    }

    break;
  }
  }
}

void
ClientReceiver::receivePendingMessages(bool& isUp) {
  std::unique_lock<std::mutex> lock(m_mutex);

  int i = 0;

  auto connection = m_clientConnections.begin();

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
ClientReceiver::startReceiveLoop() {
  bool isUp = true;
  while (isUp == true) {
    this->receivePendingMessages(isUp);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(LOOP_SLEEP_DURATION));
  }
}

void
ClientReceiver::addConnection(Messenger::Connection connection) {
  std::unique_lock<std::mutex> lock(m_mutex);

  m_clientConnections.push_back(connection);
}
