#include <chrono>
#include <thread>
#include <optional>
#include <iostream>
#include <json.hpp>
#include <cstdio>
#include <fstream>

#include "consensus-manager.hh"

#define PROMISE_WAIT_DURATION 5
#define ACCEPT_WAIT_DURATION 5
#define LOG_WRITE_DURATION 3

ConsensusManager::ConsensusManager(
    Messenger& messenger,
    std::shared_ptr<ReceiverManager> receiverManager,
    LogFileManager& logFileManager)
    : MessageReceiver(messenger, managedTag, receiverManager),
      m_logFileManager(logFileManager) {
}

void
receiveAccepts(const Messenger& messenger,
               std::mutex& mutex,
               ConsensusManager::Context& context,
               bool& majorityAccepted) {
  // pause the current thread to let the consensus thread receive the accepts
  std::this_thread::sleep_for(std::chrono::seconds(ACCEPT_WAIT_DURATION));

  int clusterSize = messenger.getClusterSize();

  {
    std::unique_lock<std::mutex> lock(mutex);

    majorityAccepted = context.acceptCount >= ((clusterSize - 1) / 2);
  }

}

void
receivePromises(const Messenger& messenger,
                std::mutex& mutex,
                ConsensusManager::Context& context,
                bool& majorityPromised) {
  // pause the current thread to let the consensus thread receive the promises
  std::this_thread::sleep_for(std::chrono::seconds(PROMISE_WAIT_DURATION));

  int clusterSize = messenger.getClusterSize();


  {
    std::unique_lock<std::mutex> lock(mutex);

    majorityPromised = context.promiseCount >= ((clusterSize - 1) / 2);
  }

}

void
broadcastPrepare(const Messenger& messenger,
                 std::mutex& mutex,
                 ConsensusManager::Context& context) {
  Message prepare;
  messenger.setMessage(ConsensusCode::PREPARE, prepare);

  {
    std::unique_lock<std::mutex> lock(mutex);

    context.roundId = prepare.getId();
  }

  int clusterSize = messenger.getClusterSize();
  messenger.broadcast(prepare, 0, clusterSize, false);
}

void
broadcastPropose(const Messenger& messenger,
                 const std::string& value,
                 std::mutex& mutex,
                 ConsensusManager::Context& context) {
  std::string proposeValue;
  int roundId;

  {
    std::unique_lock<std::mutex> lock(mutex);
    proposeValue = context.maxAcceptedId == -1 ? value : context.acceptedValue;
    roundId = context.roundId;
  }

  nlohmann::json proposeDataJson = {{"roundId", roundId},
                                    {"value", proposeValue}};

  const std::string& proposeData = proposeDataJson.dump();
  Message propose;
  messenger.setMessage(ConsensusCode::PROPOSE, proposeData, propose);

  int clusterSize = messenger.getClusterSize();
  messenger.broadcast(propose, 0, clusterSize, false);
}

void
broadcastAccepted(const Messenger& messenger,
                  const std::string& value,
                  std::mutex& mutex,
                  ConsensusManager::Context& context) {
  int roundId;
  {
    std::unique_lock<std::mutex> lock(mutex);

    roundId = context.roundId;
  }

  nlohmann::json acceptedDataJson = {{"roundId", roundId}, {"value", value}};
  const std::string& acceptedData = acceptedDataJson.dump();

  Message accepted;
  messenger.setMessage(ConsensusCode::ACCEPTED, acceptedData, accepted);

  int clusterSize = messenger.getClusterSize();
  messenger.broadcast(accepted, 0, clusterSize, false);
}

void
ConsensusManager::startConsensus(const std::string& value,
                                 bool& consensusReached) {
  consensusReached = false;

  broadcastPrepare(m_messenger, m_mutex, m_context);

  bool majorityPromised;
  receivePromises(m_messenger, m_mutex, m_context, majorityPromised);

  if (majorityPromised == true) {
    broadcastPropose(m_messenger, value, m_mutex, m_context);

    bool majorityAccepted = false;
    receiveAccepts(m_messenger, m_mutex, m_context, majorityAccepted);

    if (majorityAccepted == true) {
      m_logFileManager.append(value);

      broadcastAccepted(m_messenger, value, m_mutex, m_context);

      consensusReached = true;
    }
  }
  
  // wait for the consensus thread to write down the log
  std::this_thread::sleep_for(std::chrono::seconds(LOG_WRITE_DURATION));

  // reset the consensus round context
  {
    std::unique_lock<std::mutex> lock(m_mutex);

    m_context = ConsensusManager::Context{};
  }
}

void
handlePrepareMessage(const Messenger& messenger,
                     const int& srcNodeId,
                     const int& id,
                     int& maxRoundId,
                     std::mutex& mutex,
                     ConsensusManager::Context& context) {
  std::unique_lock<std::mutex> lock(mutex);

  if (id > maxRoundId) {
    maxRoundId = id;

    Message promise;
    if (context.valueAccepted == true) {
      nlohmann::json promiseDataJson = {
          {"roundId", id},
          {"acceptedId", context.acceptedId},
          {"acceptedValue", context.acceptedValue}};
      const std::string& promiseData = promiseDataJson.dump();
      messenger.setMessage(ConsensusCode::PROMISE, promiseData, promise);

      // send back the accepted value if a proposal was already accepted
      messenger.send(srcNodeId, promise);
    } else {
      nlohmann::json promiseDataJson = {{"roundId", id}};
      const std::string& promiseData = promiseDataJson.dump();
      messenger.setMessage(ConsensusCode::PROMISE, promiseData, promise);

      // otherwise respond with a normal promise
      messenger.send(srcNodeId, promise);
    }
  }
}

void
handleProposeMessage(const Messenger& messenger,
                     const int& srcNodeId,
                     const Message& receivedMessage,
                     const int& id,
                     const int& maxRoundId,
                     std::mutex& mutex,
                     ConsensusManager::Context& context) {
  std::unique_lock<std::mutex> lock(mutex);

  const std::string& messageData = receivedMessage.getData();
  nlohmann::json messageJson = nlohmann::json::parse(messageData);

  int roundId = messageJson.at("roundId");
  if (roundId == maxRoundId) {
    context.valueAccepted = true;
    context.acceptedId = roundId;

    std::string value = messageJson.at("value");
    context.acceptedValue = value;

    Message accept;
    nlohmann::json acceptDataJson = {{"roundId", id}};
    const std::string& acceptData = acceptDataJson.dump();

    messenger.setMessage(ConsensusCode::ACCEPT, acceptData, accept);

    // send accept if the previous condition is met
    messenger.send(srcNodeId, accept);
  }
}

void
handleAcceptedMessage(const Messenger& messenger,
                      const Message& receivedMessage,
                      LogFileManager& logFileManager,
                      std::mutex& mutex,
                      ConsensusManager::Context& context) {
  const std::string& messageData = receivedMessage.getData();
  nlohmann::json messageJson = nlohmann::json::parse(messageData);

  std::string value = messageJson.at("value");

  // write down the value to the log
  logFileManager.append(value);

  {
    std::unique_lock<std::mutex> lock(mutex);
    context = ConsensusManager::Context{};
  }
}

void
handlePromiseMessage(const Message& promise,
                     std::mutex& mutex,
                     ConsensusManager::Context& context) {
  const std::string& messageData = promise.getData();
  nlohmann::json messageJson = nlohmann::json::parse(messageData);

  int id = messageJson.at("roundId");

  {
    std::unique_lock<std::mutex> lock(mutex);

    if (id == context.roundId) {
      int acceptedId = messageJson.value("acceptedId", -1);

      if (acceptedId > context.maxAcceptedId) {
        context.acceptedValue = messageJson.value("acceptedValue", "");
        context.maxAcceptedId = acceptedId;
      }

      // increase the accept count. Checked on the thread that started consensus
      context.promiseCount += 1;
    }
  }
}

void
handleAcceptMessage(const Message& accept,
                    std::mutex& mutex,
                    ConsensusManager::Context& context) {
  std::unique_lock<std::mutex> lock(mutex);

  // increase the accept count. Checked on the thread that started consensus
  context.acceptCount += 1;
}

void
ConsensusManager::handleMessage(const int& srcNodeId,
                                const Message& receivedMessage,
                                const Messenger::Connection& connection) {
  int clusterSize = m_messenger.getClusterSize();
  ConsensusCode code = receivedMessage.getCode<ConsensusCode>();
  int id = receivedMessage.getId();
  switch (code) {
  case ConsensusCode::PREPARE: {
    handlePrepareMessage(
        m_messenger, srcNodeId, id, m_maxRoundId, m_mutex, m_context);
    break;
  }
  case ConsensusCode::PROMISE: {
    handlePromiseMessage(receivedMessage, m_mutex, m_context);
    break;
  }
  case ConsensusCode::PROPOSE: {
    handleProposeMessage(m_messenger,
                         srcNodeId,
                         receivedMessage,
                         id,
                         m_maxRoundId,
                         m_mutex,
                         m_context);
    break;
  }
  case ConsensusCode::ACCEPT: {
    handleAcceptMessage(receivedMessage, m_mutex, m_context);
    break;
  }
  case ConsensusCode::ACCEPTED: {
    handleAcceptedMessage(
        m_messenger, receivedMessage, m_logFileManager, m_mutex, m_context);
    break;
  }
  }
}

void
ConsensusManager::stopReceiver() {
  Message message;
  m_messenger.setMessage(ConsensusCode::SHUTDOWN, message);

  m_messenger.send(m_messenger.getRank(), message);
}
