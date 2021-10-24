#include <iostream>
#include <thread>

#include <json.hpp>

#include "client-manager.hh"
#include "consensus-manager.hh"
#include "election-manager.hh"
#include "failure-manager.hh"
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
                 const std::string& port,
                 std::list<Messenger::Connection>& clientConnections,
                 std::mutex& connectionMutex) {
  bool isUp = true;
  while (isUp == true) {
    Messenger::Connection connection;
    messenger.acceptConnBlock(port, connection);

    int srcNodeId;
    Message message;
    messenger.receiveBlock(srcNodeId, message, connection);

    {
      std::unique_lock<std::mutex> lock(connectionMutex);

      clientConnections.push_back(connection);
    }

    isUp = message.getCode<ClientCode>() != ClientCode::SHUTDOWN;
  }
}

void
sendPort(const Messenger& messenger, const std::string& port) {
  int nodeId = messenger.getRank();
  int clusterSize = messenger.getClusterSize();

  int prevNodeId = nodeId == 0 ? clusterSize - 1: nodeId - 1; 

  nlohmann::json messageJson = {{"port", port}};
  std::string messageString = messageJson.dump();
  Message message;
  messenger.setMessage(ClientCode::PORT, messageString, message);

  messenger.send(prevNodeId, message);
}

void
receivePort(const Messenger& messenger, std::string& nextNodePort) {
  int srcNodeId;
  Message receivedMessage;
  messenger.receiveWithTagBlock(MessageTag::CLIENT, srcNodeId, receivedMessage);

  const std::string& messageString = receivedMessage.getData();
  nlohmann::json messageJson = nlohmann::json::parse(messageString);
  messageJson.at("port").get_to(nextNodePort);
}

void
exchangePorts(const Messenger& messenger,
              const std::string& port,
              std::string& nextNodePort) {
  if (messenger.getRank() == 0) {
    sendPort(messenger, port);
    receivePort(messenger, nextNodePort);
  } else {
    receivePort(messenger, nextNodePort);
    sendPort(messenger, port);
  }
}

void
shutdownNeighbor(const Messenger& messenger,
                 std::shared_ptr<ReceiverManager> receiverManager,
                 const std::string& nextNodePort) {
  std::shared_ptr<ElectionManager> electionManager =
      receiverManager->getReceiver<ElectionManager>();

  int nodeId = messenger.getRank();
  int clusterSize = messenger.getClusterSize();

  if (electionManager->getLeaderNodeId() != (nodeId + 1) % clusterSize) {
    Messenger::Connection connection;
    messenger.connect(nextNodePort, connection);

    Message message;
    messenger.setMessage(ClientCode::SHUTDOWN, message);
    messenger.send(0, message, connection);
    messenger.send(0, message, connection);
  }
}

void
ClientManager::startReceiver() {
  m_messenger.openPort(m_port);

  exchangePorts(m_messenger, m_port, m_nextNodePort);

  m_acceptConnThread = std::thread(acceptConnection,
                                   std::ref(m_messenger),
                                   std::ref(m_port),
                                   std::ref(m_clientConnections),
                                   std::ref(m_connectionMutex));

  std::shared_ptr<ReplManager> replManager =
      m_receiverManager->getReceiver<ReplManager>();
  std::shared_ptr<FailureManager> failureManager =
      m_receiverManager->getReceiver<FailureManager>();


  bool isUp = true;
  while (isUp == true) {
    replManager->sleep();
    failureManager->sleep();

    this->receivePendingMessages(isUp);

    std::this_thread::sleep_for(std::chrono::milliseconds(LOOP_SLEEP_DURATION));
  }

  m_acceptConnThread.join();

  m_messenger.closePort(m_port);

  shutdownNeighbor(m_messenger, m_receiverManager, m_nextNodePort);
}

void
ClientManager::enableClientConn() {
  m_messenger.publishPort(m_port);
}

void
ClientManager::disableClientConn() {
  m_messenger.unpublishPort(m_port);
}
