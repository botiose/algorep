#include <iostream>

#include "node.hh"
#include "leader-election.hh"
#include "consensus-manager.hh"
#include "receiver-manager.hh"
#include "repl-receiver.hh"
#include "election-receiver.hh"
#include "consensus-receiver.hh"
#include "fail-receiver.hh"
#include "client-receiver.hh"

void
Node::startMainLoops() {
  if (this->isLeader() == true) {
    // node.startAcceptThread();
  }  

  ReceiverManager receiverManager{};

  ReplReceiver replReceiver(m_messenger);
  ElectionReceiver electionReceiver(m_messenger);
  ConsensusReceiver consensusReceiver(m_messenger);
  FailReceiver failReceiver(m_messenger);
  ClientReceiver clientReceiver(m_messenger);

  receiverManager.addReceiver(replReceiver);
  receiverManager.addReceiver(electionReceiver);
  receiverManager.addReceiver(consensusReceiver);
  receiverManager.addReceiver(failReceiver);
  receiverManager.addReceiver(clientReceiver);

  receiverManager.startReceiverLoops();

  receiverManager.waitForReceivers();
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
