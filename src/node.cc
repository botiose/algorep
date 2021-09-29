#include <iostream>
#include <unistd.h>
#include <chrono>

#include "node.hh"
#include "message-codes.hh"

#define ELECTION_WAIT_DURATION 3
#define VICTORY_WAIT_DURATION 60

Node::Node(const int& nodeId, const int& clusterSize)
    : m_nodeId(nodeId), m_clusterSize(clusterSize), m_messenger(nodeId) {
  // m_isLeader = nodeId == clusterSize - 1;
  // m_leaderNodeId = clusterSize - 1;
}

void
Node::HandleBullyMessage(const int& srcNodeId, const Message& receivedMessage) {
  BullyCode bullyCode = static_cast<BullyCode>(receivedMessage.code);
  switch (bullyCode) {
  case BullyCode::ELECTION: {
    std::cout << "node.cc: [I] Node " << m_nodeId
              << " received BullyCode::ELECTION from node "
              << srcNodeId << std::endl;
    Message activeMessage{static_cast<int>(MessageTag::BULLY),
                          static_cast<int>(BullyCode::ALIVE)};

    m_messenger.send(srcNodeId, activeMessage);

    if (srcNodeId < m_nodeId) {
      startElection();
    }
    break;
  }
  case BullyCode::VICTORY: {
    std::cout << "node.cc: [I] Node " << m_nodeId
              << " received BullyCode::VICTORY from node "
              << srcNodeId << std::endl;
    m_isLeader = false;
    m_leaderNodeId = srcNodeId;
    break;
  }
  default: {
    std::cout << "node.cc: [I] Node " << m_nodeId
              << " received unexpected BullyCode::ALIVE from node "
              << srcNodeId << std::endl;
    break;
  }
  }
}

void
Node::startReceiveLoop() {
  bool isUp = true;
  while (isUp == true) {
    int srcNodeId;
    Message receivedMessage;
    m_messenger.receiveBlock(srcNodeId, receivedMessage);

    MessageTag messageTag = static_cast<MessageTag>(receivedMessage.tag);
    switch (messageTag) {
    case MessageTag::BULLY: {
      HandleBullyMessage(srcNodeId, receivedMessage);
      break;
    }
    }
  }
}

void
busyBullyWait(const Messenger& messenger,
              const int& nodeId,
              const int& clusterSize,
              const int& waitSeconds,
              bool& gotVictor) {
  using namespace std::chrono;
  auto start = high_resolution_clock::now();
  auto cur = high_resolution_clock::now();
  int elapsed = 0;
  bool messageReceived = false;
  while ((messageReceived == false) && (elapsed < waitSeconds)) {
    int srcNodeId;
    Message receivedMessage;
    messenger.receiveWithTag(
        MessageTag::BULLY, messageReceived, srcNodeId, receivedMessage);

    if (messageReceived == true) {
      BullyCode messageCode = static_cast<BullyCode>(receivedMessage.code);

      switch (messageCode) {
      case BullyCode::ALIVE: {
        std::cout << "node.cc: [I] node " << nodeId
                  << " received BullyCode::ALIVE from node " << srcNodeId
                  << std::endl;
        busyBullyWait(messenger, nodeId, clusterSize, 50, gotVictor);
        break;
      }
      case BullyCode::ELECTION: {
        std::cout << "node.cc: [I] node " << nodeId
                  << " received BullyCode::ELECTION from node " << srcNodeId
                  << std::endl;
        Message activeMessage{static_cast<int>(MessageTag::BULLY),
                              static_cast<int>(BullyCode::ALIVE)};

        messenger.send(srcNodeId, activeMessage);
        break;
      }
      case BullyCode::VICTORY: {
        std::cout << "node.cc: [I] node " << nodeId
                  << " received BullyCode::VICTORY from node " << srcNodeId
                  << std::endl;
        gotVictor = true;
        break;
      }
      }
    }

    cur = high_resolution_clock::now();
    elapsed = duration_cast<std::chrono::seconds>(cur - start).count();
  }

  if (messageReceived == false) {
    // std::cout << nodeId << std::endl;
    Message victoryMessage;
    victoryMessage.tag = static_cast<int>(MessageTag::BULLY);
    victoryMessage.code = static_cast<int>(BullyCode::VICTORY);

    for (int i = 0; i < clusterSize; i++) {
      if (i == nodeId) {
        continue;
      }

      messenger.send(i, victoryMessage);
    }

    gotVictor = true;
  }
}

void
Node::startElection() const {
  std::cout << "node.cc: [I] node " << m_nodeId << " started an election."
            << std::endl;
  bool gotVictor = false;
  while (gotVictor == false) {
    Message message;
    message.tag = static_cast<int>(MessageTag::BULLY);
    message.code = static_cast<int>(BullyCode::ELECTION);

    for (int dstNodeId = m_nodeId + 1; dstNodeId < m_clusterSize; dstNodeId++) {
      m_messenger.send(dstNodeId, message);
    }

    busyBullyWait(m_messenger, m_nodeId, m_clusterSize, 5, gotVictor);
  }
}

int
Node::getNodeId() const {
  return m_nodeId;
};
