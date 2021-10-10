#include <iostream>

#include "node.hh"
#include "leader-election.hh"
#include "consensus-manager.hh"
#include "repl-receiver.hh"
#include "election-receiver.hh"
#include "consensus-receiver.hh"
#include "fail-receiver.hh"
#include "client-receiver.hh"

void
acceptConnection(Messenger& messenger,
                 std::shared_ptr<ClientReceiver> clientReceiver) {
  messenger.publish();

  bool isUp = true;
  while (isUp == true) {
    Messenger::Connection connection;
    std::cout << "accepting" << std::endl; 
    messenger.acceptConnection(connection);
    std::cout << "connection accepted" << std::endl; 

    MessageTag tag;
    messenger.probeTagBlock(connection, tag);

    if (tag != MessageTag::REPL) {
      clientReceiver->addConnection(connection);
    } else {
      int srcNodeId;
      Message message;
      messenger.receiveWithTagBlock(tag, srcNodeId, message, connection);

      isUp = message.getCode<ReplCode>() != ReplCode::SHUTDOWN;
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
  std::shared_ptr<ElectionReceiver> electionReceiver =
      std::make_shared<ElectionReceiver>(m_messenger);
  std::shared_ptr<ConsensusReceiver> consensusReceiver =
      std::make_shared<ConsensusReceiver>(m_messenger);
  std::shared_ptr<FailReceiver> failReceiver =
      std::make_shared<FailReceiver>(m_messenger);

  m_receiverManager.startReceiver(replReceiver);
  m_receiverManager.startReceiver(electionReceiver);
  m_receiverManager.startReceiver(consensusReceiver);
  m_receiverManager.startReceiver(failReceiver);

  if (this->isLeader() == true) {
    this->enableClientCommunication();
  }  

  m_receiverManager.waitForReceivers();

  if (this->isLeader() == true) {
    m_acceptConnThread.join();
  }
}

void
Node::replicateData(const std::string& data) const {
  ConsensusManager::startConsensus(m_messenger,
                                   m_nodeId, // TODO remove
                                   m_clusterSize,
                                   data);
}

bool
Node::isLeader() const {
  return m_nodeId == m_leaderNodeId;
}

void
Node::init() {
  m_messenger.start(m_nodeId, m_clusterSize);

  m_leaderNodeId = m_clusterSize - 1;
}

void
Node::destroy() const {
  m_messenger.stop();
}
