#include <iostream>

#include "node.hh"
#include "consensus-manager.hh"
#include "repl-manager.hh"
#include "election-manager.hh"
#include "failure-manager.hh"
#include "client-manager.hh"

// TODO move accept logic to a given namespace
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

    clientManager->addConnection(connection);

    messenger.send(messenger.getRank(), message);

    if (tag == MessageTag::REPL) {
      isUp = message.getCode<ReplCode>() != ReplCode::SHUTDOWN;
    }
  }

  messenger.unpublish();
}

void
Node::enableClientCommunication() {
  m_clientManager = std::make_shared<ClientManager>(m_messenger, m_replManager);

  m_receiverManager.startReceiver(m_clientManager);

  m_acceptConnThread =
      std::thread(acceptConnection, std::ref(m_messenger), m_clientManager);
}

void
Node::disableClientCommunication() {
  Messenger::Connection connection;
  m_messenger.selfConnect(connection);

  int dstNodeId = 0;
  Message message;
  m_messenger.setMessage(ReplCode::SHUTDOWN, message);
  m_messenger.send(dstNodeId, message, connection);

  m_receiverManager.waitForReceiver(MessageTag::CLIENT);
}

void
Node::init(int argc, char** argv) {
  m_messenger.start(argc, argv, m_nodeId, m_clusterSize);

  m_replManager = std::make_shared<ReplManager>(m_messenger);
  m_electionManager =
      std::make_shared<ElectionManager>(m_messenger, m_replManager);
  std::shared_ptr<ConsensusManager> consensusManager =
      std::make_shared<ConsensusManager>(m_messenger, m_replManager);
  std::shared_ptr<FailureManager> failureManager =
      std::make_shared<FailureManager>(m_messenger, m_replManager);

  m_receiverManager.startReceiver(m_replManager);
  m_receiverManager.startReceiver(m_electionManager);
  m_receiverManager.startReceiver(consensusManager);
  // m_receiverManager.startReceiver(failureManager);

  m_electionManager->triggerElection();

  m_electionManager->waitForVictor();
  
  if (m_electionManager->isLeader() == true) {
    this->enableClientCommunication();
  }

  m_receiverManager.waitForAllReceivers();

  if (m_electionManager->isLeader() == true) {
    m_acceptConnThread.join();
  }  
}

void
Node::destroy() const {
  m_messenger.stop();
}
