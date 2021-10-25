#include <iostream>

#include "node.hh"
#include "consensus-manager.hh"
#include "repl-manager.hh"
#include "election-manager.hh"
#include "failure-manager.hh"
#include "client-manager.hh"
#include "log-file-manager.hh"

#define REPL_MSG_FILEPATH "etc/server/repl.txt"

void
Node::init(int argc, char** argv) {
  m_messenger.start(argc, argv, m_nodeId, m_clusterSize);

  m_receiverManager = std::make_shared<ReceiverManager>();

  LogFileManager logFileManager(m_messenger.getRank());
  std::shared_ptr<ConsensusManager> consensusManager =
      std::make_shared<ConsensusManager>(
          m_messenger, m_receiverManager, logFileManager);
  std::shared_ptr<FailureManager> failureManager =
      std::make_shared<FailureManager>(
          m_messenger, m_receiverManager, logFileManager);

  std::shared_ptr<ReplManager> replManager = std::make_shared<ReplManager>(
      m_messenger, m_receiverManager, REPL_MSG_FILEPATH);

  std::shared_ptr<ElectionManager> electionManager =
      std::make_shared<ElectionManager>(
          m_messenger, m_receiverManager);
  std::shared_ptr<ClientManager> clientManager =
      std::make_shared<ClientManager>(m_messenger, m_receiverManager);

  m_receiverManager->startReceiver(replManager);
  m_receiverManager->startReceiver(electionManager);
  m_receiverManager->startReceiver(consensusManager);
  m_receiverManager->startReceiver(failureManager);
  m_receiverManager->startReceiver(clientManager);

  m_receiverManager->waitForReceiver(MessageTag::REPL);
  m_receiverManager->waitForReceiver(MessageTag::LEADER_ELECTION);
  m_receiverManager->waitForReceiver(MessageTag::CONSENSUS);
  m_receiverManager->waitForReceiver(MessageTag::CLIENT);
  m_receiverManager->waitForReceiver(MessageTag::FAILURE_DETECTION);
}

void
Node::destroy() const {
  m_messenger.stop();
}
