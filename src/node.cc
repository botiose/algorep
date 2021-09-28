#include <iostream>
#include <unistd.h>

#include "node.hh"
#include "message-codes.hh"

#define WAIT_DURATION 3

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
    Message activeMessage{static_cast<int>(MessageTag::BULLY),
                          static_cast<int>(BullyCode::ALIVE)};

    m_messenger.sendMessage(srcNodeId, activeMessage);

    if (srcNodeId < m_nodeId) {
      startElection();
    }
    break;
  }
  case BullyCode::VICTORY: {
    m_isLeader = false;
    m_leaderNodeId = srcNodeId;
    break;
  }
  default: {
    std::cout
        << "node.cc: [I] Received an unexpected BULLY:ACTIVE message code."
        << std::endl;
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
    m_messenger.receiveMessage(srcNodeId, receivedMessage);

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
Node::startElection() const {
  bool gotVictor = false;
  while (gotVictor == false) {
    Message message;
    message.tag = static_cast<int>(MessageTag::BULLY);
    message.code = static_cast<int>(BullyCode::ELECTION);

    for (int dstNodeId = m_nodeId + 1; dstNodeId < m_clusterSize; dstNodeId++) {
      m_messenger.sendMessage(dstNodeId, message);
    }

    sleep(WAIT_DURATION);

    bool gotElectionResponse = m_messenger.hasMessageWithTag(MessageTag::BULLY);

    if (gotElectionResponse == false) {
      Message victoryMessage;
      victoryMessage.tag = static_cast<int>(MessageTag::BULLY);
      victoryMessage.code = static_cast<int>(BullyCode::VICTORY);

      for (int nodeId = 0; nodeId < m_clusterSize; nodeId++) {
        if (nodeId == m_nodeId) {
          continue;
        }

        m_messenger.sendMessage(nodeId, message);
      }

      gotVictor = true;
    } else {
      // otherwise check if there's a process with higher id in responses
      bool higherIdNodeResponded = false;
      bool responsesRemain;
      do {
        int srcNodeId;
        Message responseMessage;
        m_messenger.receiveMessageWithTag(
            MessageTag::BULLY, srcNodeId, responseMessage);

        higherIdNodeResponded |= srcNodeId > m_nodeId;

        responsesRemain = m_messenger.hasMessageWithTag(MessageTag::BULLY);
      } while (responsesRemain == true);

      // if yes then wait for a victory message for a given amount of time
      // otherwise restart election
      if (higherIdNodeResponded) {
        sleep(WAIT_DURATION);

        while (responsesRemain) {
          responsesRemain = m_messenger.hasMessageWithTag(MessageTag::BULLY);
          int srcNodeId;
          Message responseMessage;
          m_messenger.receiveMessageWithTag(
              MessageTag::BULLY, srcNodeId, responseMessage);

          gotVictor |=
              static_cast<BullyCode>(message.code) == BullyCode::VICTORY;
        }
      }
    }
  }
}

int
Node::getNodeId() const {
  return m_nodeId;
};
