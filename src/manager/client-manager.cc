#include <iostream>
#include <thread>

#include <json.hpp>

#include "client-manager.hh"
#include "consensus-manager.hh"
#include "election-manager.hh"
#include "failure-manager.hh"
#include "receiver-manager.hh"

#define LOOP_SLEEP_DURATION 100

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
    
    std::shared_ptr<ConsensusManager> consensusManager =
        m_receiverManager->getReceiver<ConsensusManager>();

    // block the thread until the consensus is reached or timeout occurs
    bool consensusReached;
    consensusManager->startConsensus(value, consensusReached);

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
acceptConnection(Messenger& messenger,
                 std::shared_ptr<FailureManager>& failureManager,
                 const std::string& port,
                 Messenger::Connection& clientConnection) {
  // notify the failure manager that there is no curernt client connection and
  // that recovery can take place
  failureManager->allowRecovery();

  messenger.acceptConnBlock(port, clientConnection);

  failureManager->disallowRecovery();
}

void
ClientManager::receivePendingMessages() {
  std::shared_ptr<ReplManager> replManager =
      m_receiverManager->getReceiver<ReplManager>();

  bool isUp = true;
  while (isUp == true) {
    int srcNodeId;
    Message receivedMessage;

    m_messenger.receiveWithTagBlock(
        MessageTag::CLIENT, srcNodeId, receivedMessage, m_clientConnection);

    // message code 0 is SHUTDOWN for all message tags
    isUp = receivedMessage.getCodeInt() != 0;
    if (isUp == true) {
      if (receivedMessage.getCode<ClientCode>() != ClientCode::DISCONNECT) {
        
        replManager->sleep();

        this->handleMessage(srcNodeId, receivedMessage, m_clientConnection);
      } else {
        m_messenger.disconnect(m_clientConnection);

        std::shared_ptr<FailureManager> failureManager =
          m_receiverManager->getReceiver<FailureManager>();

        acceptConnection(
            m_messenger, failureManager, m_port, m_clientConnection);
      }
    }
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

// the two functions above and this one are for the sake of proper shutdown of
// the system
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

// this function is used to properly shutdown the system
void
shutdownNeighbor(const Messenger& messenger,
                 std::shared_ptr<ElectionManager> electionManager,
                 const std::string& nextNodePort) {
  int nodeId = messenger.getRank();
  int clusterSize = messenger.getClusterSize();

  if (electionManager->getLeaderNodeId() != (nodeId + 1) % clusterSize) {
    Messenger::Connection connection;
    messenger.connect(nextNodePort, connection);

    Message message;
    messenger.setMessage(ClientCode::SHUTDOWN, message);
    messenger.send(0, message, connection);

    messenger.disconnect(connection);
  }
}

void
ClientManager::startReceiver() {
  m_messenger.openPort(m_port);

  exchangePorts(m_messenger, m_port, m_nextNodePort);

  std::shared_ptr<FailureManager> failureManager =
      m_receiverManager->getReceiver<FailureManager>();

  acceptConnection(m_messenger, failureManager, m_port, m_clientConnection);

  this->receivePendingMessages();

  m_messenger.disconnect(m_clientConnection);

  std::shared_ptr<ElectionManager> electionManager =
      m_receiverManager->getReceiver<ElectionManager>();

  m_messenger.closePort(m_port);

  shutdownNeighbor(m_messenger, electionManager, m_nextNodePort);

}

void
ClientManager::enableClientConn() {
  m_messenger.publishPort(m_port);
}
