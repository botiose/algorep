#include <iostream>

#include "node.hh"
#include "consensus-manager.hh"
#include "repl-receiver.hh"
#include "election-manager.hh"
#include "fail-receiver.hh"
#include "client-receiver.hh"

void
acceptConnection(Messenger& messenger,
                 std::shared_ptr<ClientReceiver> clientReceiver) {
  messenger.publish();
  std::cout << "accepting connections" << std::endl; 
  bool isUp = true;
  while (isUp == true) {
    Messenger::Connection connection;
    messenger.acceptConnection(connection);

    int srcNodeId;
    Message message;
    messenger.receiveBlock(srcNodeId, message, connection);

    MessageTag tag = message.getTag();

    switch (tag) {
    case MessageTag::CLIENT: {
      if (message.getCode<ClientCode>() == ClientCode::CONNECT) {
        clientReceiver->addConnection(connection);
      }
      break;
    }
    case MessageTag::REPL: {
      isUp = message.getCode<ReplCode>() != ReplCode::SHUTDOWN;
      break;
    }
    }
  }

  messenger.unpublish();
}

void
Node::enableClientCommunication() {
  std::shared_ptr<ClientReceiver> clientReceiver =
      std::make_shared<ClientReceiver>(m_messenger);
  m_receiverManager.startReceiver(clientReceiver);

  m_acceptConnThread =
      std::thread(acceptConnection, std::ref(m_messenger), clientReceiver);
}

void
Node::disableClientCommunication() {
  Messenger::Connection connection;
  m_messenger.selfConnect(connection);

  int dstNodeId = 0;
  Message message;
  m_messenger.setMessage(ReplCode::SHUTDOWN, message);
  m_messenger.send(dstNodeId, message, connection);
}

void
Node::startMainLoops() {
  std::shared_ptr<ReplReceiver> replReceiver =
      std::make_shared<ReplReceiver>(m_messenger);
  std::shared_ptr<ElectionManager> electionManager =
      std::make_shared<ElectionManager>(m_messenger);
  std::shared_ptr<ConsensusManager> consensusManager =
      std::make_shared<ConsensusManager>(m_messenger);
  std::shared_ptr<FailReceiver> failReceiver =
      std::make_shared<FailReceiver>(m_messenger);

  // m_receiverManager.startReceiver(replReceiver);
  // m_receiverManager.startReceiver(electionManager);
  m_receiverManager.startReceiver(consensusManager);
  // m_receiverManager.startReceiver(failReceiver);

  if (this->isLeader() == true) {
    this->enableClientCommunication();
  }

  m_receiverManager.waitForReceivers();

  if (this->isLeader() == true) {
    m_acceptConnThread.join();
  }
}

bool
Node::isLeader() const {
  return m_nodeId == m_leaderNodeId;
}

void
Node::init(int argc, char** argv) {
  m_messenger.start(argc, argv, m_nodeId, m_clusterSize);

  m_leaderNodeId = m_clusterSize - 1;
}

void
Node::destroy() const {
  m_messenger.stop();
}
