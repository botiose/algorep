#include <iostream>
#include <fstream>
#include <string>
#include <thread>

#include <json.hpp>

#include "repl-manager.hh"

#define REPL_MSG_FILEPATH "etc/repl-msg.txt"

#define LOOP_SLEEP_DURATION 100

ReplManager::ReplManager(const Messenger& messenger)
    : MessageReceiver(messenger, MessageTag::REPL) {
}

void
ReplManager::handleMessage(const int& srcNodeId,
                           const Message& receivedMessage,
                           const Messenger::Connection& connection) {
  // TODO 
}

void
getTrailingLine(std::ifstream& ifs, std::string& line) {
  std::getline(ifs, line);
  ifs.clear();
}

void
parseLine(const std::string& line,
          const Messenger& messenger,
          Message& receivedMessage,
          bool& messageReceived,
          int& dstNodeId) {
  if (line.empty() == false) {
    size_t delimiterPos = line.find(",");

    if (delimiterPos != std::string::npos) {
      std::string dstNodeIdStr = line.substr(0, delimiterPos);
      std::string codeStr = line.substr(delimiterPos + 1);

      dstNodeId = std::stoi(dstNodeIdStr);

      if (0 <= dstNodeId && dstNodeId < messenger.getClusterSize()) {
        auto codeIte = replParseMap.find(codeStr);

        if (codeIte != replParseMap.end()) {
          ReplCode code = codeIte->second;
          messenger.setMessage(code, receivedMessage);

          messageReceived = true;
        }
      }
    }
  }
}

void
fetchMessageFromFile(std::ifstream& ifs,
                     const Messenger& messenger,
                     Message& receivedMessage,
                     bool& messageReceived,
                     int& dstNodeId) {
  std::string line;
  getTrailingLine(ifs, line);

  parseLine(line, messenger, receivedMessage, messageReceived, dstNodeId);
}

void
ReplManager::startReceiveLoop() {
  std::ifstream ifs(REPL_MSG_FILEPATH);

  // TODO check file validity

  bool isUp = true;
  while (isUp == true) {
    int srcNodeId = m_messenger.getRank();
    Message receivedMessage;
    bool messageReceived = false;
    int dstNodeId;

    fetchMessageFromFile(
        ifs, m_messenger, receivedMessage, messageReceived, dstNodeId);

    if (messageReceived == true && dstNodeId == m_messenger.getRank()) {
      this->handleMessage(srcNodeId, receivedMessage);

      isUp = receivedMessage.getCodeInt() != 0;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(LOOP_SLEEP_DURATION));
  }

  ifs.close();
}
