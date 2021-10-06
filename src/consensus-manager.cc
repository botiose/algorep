#include <json.hpp>
#include <thread>
#include <optional>

#include "consensus-manager.hh"

#define UNUSED(x) (void)(x)

#define PROMISE_WAIT_DURATION 60
#define ACCEPT_WAIT_DURATION 60

void
ConsensusManager::startConsensus(const Messenger& messenger,
                                 const int& clusterSize,
                                 const std::string& value) {
  bool majorityAccepted = false;
  while (majorityAccepted == false) {
    Message prepare;
    messenger.setMessage(ConsensusCode::PREPARE, prepare);

    for (int i = 0; i < clusterSize; i++) {
      messenger.send(i, prepare);
    }

    std::this_thread::sleep_for(std::chrono::seconds(PROMISE_WAIT_DURATION));

    int roundId = prepare.getId();
    bool messageReceived;
    int promiseCount = 0;
    int maxAcceptedId = -1;
    std::string acceptedValue;

    do {
      int srcNodeId;
      Message promise;
      messenger.receiveWithTag(
          MessageTag::CONSENSUS, messageReceived, srcNodeId, promise);

      if (messageReceived == true) {
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

    } while (messageReceived == true);

    bool majorityPromised = promiseCount >= (clusterSize / 2);

    if (majorityPromised == true) {
      const std::string& proposeValue =
          maxAcceptedId == -1 ? value : acceptedValue;

      nlohmann::json proposeDataJson = {{"roundId", roundId},
                                        {"value", proposeValue}};
      const std::string& proposeData = proposeDataJson.dump();
      Message propose;
      messenger.setMessage(ConsensusCode::PROPOSE, proposeData, propose);

      for (int i = 0; i < clusterSize; i++) {
        messenger.send(i, propose);
      }

      std::this_thread::sleep_for(std::chrono::seconds(PROMISE_WAIT_DURATION));

      int acceptCount;
      do {
        int srcNodeId;
        Message promise;
        messenger.receiveWithTag(
            MessageTag::CONSENSUS, messageReceived, srcNodeId, promise);

        if (messageReceived == true) {
          acceptCount += 1;
        }

      } while (messageReceived == true);

      majorityAccepted = acceptCount >= (clusterSize / 2);
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
      } else {
        nlohmann::json promiseDataJson = {{"roundId", id}};
        const std::string& promiseData = promiseDataJson.dump();
        messenger.setMessage(ConsensusCode::PROMISE, promiseData, promise);
      }
    }
  }
  case ConsensusCode::ACCEPT: {
    const std::string& messageData = receivedMessage.getData();
    nlohmann::json messageJson = nlohmann::json::parse(messageData);
    int roundId = messageJson.at("roundId");
    if (roundId == m_context.maxId) {
      m_context.valueAccepted = true;
      m_context.acceptedId = roundId;
      
      std::string value = messageJson.at("value");
      m_context.acceptedValue = value;
      
      Message accepted;
      nlohmann::json acceptedDataJson = {
          {"roundId", id},
          {"value", m_context.acceptedValue}};
      const std::string& acceptedData = acceptedDataJson.dump();

      messenger.setMessage(ConsensusCode::ACCEPTED, acceptedData, accepted);

      for (int i = 0; i < clusterSize; i++) {
        messenger.send(i, accepted);
      }
    }
  }
  }
}
