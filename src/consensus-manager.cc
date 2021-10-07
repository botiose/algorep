#include <chrono>
#include <thread>
#include <optional>
#include <iostream>
#include <json.hpp>

#include "consensus-manager.hh"

#define UNUSED(x) (void)(x)

#define PROMISE_WAIT_DURATION 10
#define ACCEPT_WAIT_DURATION 10

void
ConsensusManager::startConsensus(const Messenger& messenger,
                                 const int& nodeId, // TODO remove
                                 const int& clusterSize,
                                 const std::string& value) {
  bool majorityAccepted = false;
  while (majorityAccepted == false) {
    std::cout << "start" << std::endl; 
    Message prepare;
    messenger.setMessage(ConsensusCode::PREPARE, prepare);

    for (int i = 0; i < clusterSize; i++) {
      if (i == nodeId) { // TODO remove
        continue;
      }
      messenger.send(i, prepare);

      std::cout << "PREPARE: sent" << std::endl; 
    }

    int roundId = prepare.getId();
    bool messageReceived = false;
    int promiseCount = 0;
    int maxAcceptedId = -1;
    std::string acceptedValue;

    using namespace std::chrono;
    auto start = high_resolution_clock::now();
    auto cur = high_resolution_clock::now();
    int elapsed = 0;

    while(elapsed < PROMISE_WAIT_DURATION) {
      int srcNodeId;
      Message promise;
      messenger.receiveWithTag(
          MessageTag::CONSENSUS, messageReceived, srcNodeId, promise);

      if (messageReceived == true) {
        ConsensusCode code = promise.getCode<ConsensusCode>();
        if (code == ConsensusCode::PROMISE) {
          std::cout << "PROMISE: received" << std::endl;
          const std::string& messageData = promise.getData();
          nlohmann::json messageJson = nlohmann::json::parse(messageData);

          int id = messageJson.at("roundId");

          if (id == roundId) {
            int acceptedId = messageJson.value("acceptedId", -1);

            if (acceptedId > maxAcceptedId) {
              acceptedValue = messageJson.value("acceptedValue", "");
            }

            promiseCount += 1;
          }
        }
      }

      cur = high_resolution_clock::now();
      elapsed = duration_cast<std::chrono::seconds>(cur - start).count();
    }

    bool majorityPromised = promiseCount >= (clusterSize / 2);

    if (majorityPromised == true) {
      std::cout << "majority promised: " << promiseCount << std::endl; 
      const std::string& proposeValue =
          maxAcceptedId == -1 ? value : acceptedValue;

      nlohmann::json proposeDataJson = {{"roundId", roundId},
                                        {"value", proposeValue}};
      const std::string& proposeData = proposeDataJson.dump();
      Message propose;
      messenger.setMessage(ConsensusCode::PROPOSE, proposeData, propose);

      for (int i = 0; i < clusterSize; i++) {
        if (i == nodeId) { // TODO remove
          continue;
        }
        messenger.send(i, propose);
        std::cout << "PROPOSE: sent" << std::endl; 
      }

      int acceptCount = 0;

      messageReceived = false;
      start = high_resolution_clock::now();
      cur = high_resolution_clock::now();
      elapsed = 0;

      while (elapsed < ACCEPT_WAIT_DURATION) {
        int srcNodeId;
        Message accept;
        messenger.receiveWithTag(
            MessageTag::CONSENSUS, messageReceived, srcNodeId, accept);

        if (messageReceived == true) {
          ConsensusCode code = accept.getCode<ConsensusCode>();
          if (code == ConsensusCode::ACCEPT) {
            std::cout << "ACCEPT: received" << std::endl; 
            acceptCount += 1;
          }
        }
        cur = high_resolution_clock::now();
        elapsed = duration_cast<std::chrono::seconds>(cur - start).count();
      }

      majorityAccepted = acceptCount >= (clusterSize / 2);

      if (majorityAccepted == true) {
        std::cout << "CONSENSUS REACHED" << std::endl; 
      }
    }
  }
}

void
ConsensusManager::handleConsensusMessage(const Messenger& messenger,
                                         const int& clusterSize,
                                         const int& srcNodeId,
                                         const Message& receivedMessage) {
  ConsensusCode code = receivedMessage.getCode<ConsensusCode>();
  int id = receivedMessage.getId();
  switch (code) {
  case ConsensusCode::PREPARE: {
    std::cout << "PREPARE: received" << std::endl;
    if (id > m_context.maxId) {
      m_context.maxId = id;

      Message promise;
      if (m_context.valueAccepted == true) {
        nlohmann::json promiseDataJson = {
            {"roundId", id},
            {"acceptedId", m_context.acceptedId},
            {"acceptedValue", m_context.acceptedValue}};
        const std::string& promiseData = promiseDataJson.dump();

        messenger.setMessage(ConsensusCode::PROMISE, promiseData, promise);

        messenger.send(srcNodeId, promise);
        std::cout << "PROMISE: sent" << std::endl;
      } else {
        nlohmann::json promiseDataJson = {{"roundId", id}};
        const std::string& promiseData = promiseDataJson.dump();
        messenger.setMessage(ConsensusCode::PROMISE, promiseData, promise);

        messenger.send(srcNodeId, promise);
        std::cout << "PROMISE: sent" << std::endl;
      }
    }

    break;
  }
  case ConsensusCode::PROPOSE: {
    std::cout << "PROPOSE: received" << std::endl;
    const std::string& messageData = receivedMessage.getData();
    nlohmann::json messageJson = nlohmann::json::parse(messageData);

    int roundId = messageJson.at("roundId");
    if (roundId == m_context.maxId) {
      m_context.valueAccepted = true;
      m_context.acceptedId = roundId;
      
      std::string value = messageJson.at("value");
      m_context.acceptedValue = value;
      
      Message accept;
      nlohmann::json acceptDataJson = {
          {"roundId", id},
          {"value", m_context.acceptedValue}};
      const std::string& acceptData = acceptDataJson.dump();

      messenger.setMessage(ConsensusCode::ACCEPT, acceptData, accept);

      messenger.send(srcNodeId, accept);
      std::cout << "ACCEPT: sent" << std::endl;
    }
    break;
  }
  }
}
