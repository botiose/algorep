#include <chrono>
#include <thread>
#include <json.hpp>

#include "client.hh"

#define RESPONSE_WAIT_DURATION 60
#define LOOP_SLEEP_DURATION 50

void
Client::connect(int argc, char* argv[]) {
  int rank;
  int clusterSize;
  m_messenger.start(argc, argv, rank, clusterSize);

  std::string port;
  m_messenger.lookupServerPort(port);

  m_messenger.connect(port, m_serverConnection);

  Message message;
  m_messenger.setMessage(ClientCode::CONNECT, message);

  m_messenger.send(0, message, m_serverConnection);
}

void
Client::shutdownServer(int argc, char* argv[]) {
  int rank;
  int clusterSize;
  m_messenger.start(argc, argv, rank, clusterSize);

  std::string port;
  m_messenger.lookupServerPort(port);

  m_messenger.connect(port, m_serverConnection);

  Message message;
  m_messenger.setMessage(ClientCode::SHUTDOWN, message);

  m_messenger.send(0, message, m_serverConnection);
  m_messenger.send(0, message, m_serverConnection);
}

void
Client::replicate(const std::string& data) const {
  bool replicated = false;
  while (replicated == false) {
    Message message;
    nlohmann::json dataJson = {{"value", data}};

    std::string dataJsonString = dataJson.dump();

    m_messenger.setMessage(ClientCode::REPLICATE, dataJsonString, message);

    m_messenger.send(0, message, m_serverConnection);

    using namespace std::chrono;
    auto start = high_resolution_clock::now();
    auto cur = high_resolution_clock::now();
    int elapsed = 0;
    bool messageReceived = false;

    Message responseMessage;
    while ((messageReceived == false) && (elapsed < RESPONSE_WAIT_DURATION)) {
      int srcNodeId;
      m_messenger.receiveWithTag(MessageTag::CLIENT,
                                 messageReceived,
                                 srcNodeId,
                                 responseMessage,
                                 m_serverConnection);

      std::this_thread::sleep_for(
          std::chrono::milliseconds(LOOP_SLEEP_DURATION));

      cur = high_resolution_clock::now();
      elapsed = duration_cast<std::chrono::seconds>(cur - start).count();
    }

    replicated = responseMessage.getCode<ClientCode>() == ClientCode::SUCCESS;
  }
}

void
Client::disconnect() {
  Message message;
  m_messenger.setMessage(ClientCode::DISCONNECT, message);

  m_messenger.send(0, message, m_serverConnection);

  m_messenger.disconnect(m_serverConnection);

  m_messenger.stop();
}
