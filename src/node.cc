#include <iostream>

#include "node.hh"
#include "consensus-manager.hh"
#include "repl-manager.hh"
#include "election-manager.hh"
#include "failure-manager.hh"
#include "client-manager.hh"

void
acceptConnection(Messenger& messenger,
                 std::shared_ptr<ClientManager> clientManager) {
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
        clientManager->addConnection(connection);
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
  std::shared_ptr<ClientManager> clientManager =
      std::make_shared<ClientManager>(m_messenger, m_replManager);

  m_receiverManager.startReceiver(clientManager);

  m_acceptConnThread =
      std::thread(acceptConnection, std::ref(m_messenger), clientManager);
}

void
Node::disableClientCommunication() {
  Messenger::Connection connection;
  m_messenger.selfConnect(connection);

  int dstNodeId = 0;
  Message message;
  m_messenger.setMessage(ReplCode::SHUTDOWN, message);
  m_messenger.send(dstNodeId, message, connection);

  // TODO remove the client manager and related thread obj from the receiver
  // manager
}

void
Node::startMainLoops() {
  m_replManager = std::make_shared<ReplManager>(m_messenger);

  std::shared_ptr<ElectionManager> electionManager =
      std::make_shared<ElectionManager>(m_messenger, m_replManager);
  std::shared_ptr<ConsensusManager> consensusManager =
      std::make_shared<ConsensusManager>(m_messenger, m_replManager);
  std::shared_ptr<FailureManager> failureManager =
      std::make_shared<FailureManager>(m_messenger, m_replManager);

  m_receiverManager.startReceiver(m_replManager);

  // m_receiverManager.startReceiver(electionManager);
  m_receiverManager.startReceiver(consensusManager);
  // m_receiverManager.startReceiver(failureManager);

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
