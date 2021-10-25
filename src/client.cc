#include <chrono>
#include <thread>
#include <json.hpp>
#include <fstream>

#include "client.hh"
#include "repl-manager.hh"

#define RESPONSE_WAIT_DURATION 60
#define LOOP_SLEEP_DURATION 50

#define REPL_MSG_BASE_FILEPATH "etc/client/"
#define REPL_FILE "repl.txt"
#define COMMAND_FILE "command.txt"

void
connectMessenger(Messenger& messenger,
                 Messenger::Connection& serverConnection) {
  std::string port;
  messenger.lookupServerPort(port);

  messenger.connect(port, serverConnection);
}

void
Client::connect(int argc, char* argv[]) {
  int rank;
  int clusterSize;
  m_messenger.start(argc, argv, rank, clusterSize);

  m_receiverManager = std::make_shared<ReceiverManager>();

  m_baseDir = REPL_MSG_BASE_FILEPATH;
  m_baseDir.append(argv[1]);
  m_baseDir.append("/");

  std::string replFilePath(m_baseDir);
  replFilePath.append(REPL_FILE);

  std::shared_ptr<ReplManager> replManager = std::make_shared<ReplManager>(
      m_messenger, m_receiverManager, replFilePath);

  m_receiverManager->startReceiver(replManager);

  replManager->sleep();

  connectMessenger(m_messenger, m_serverConnection);

  Message message;
  m_messenger.setMessage(ClientCode::CONNECT, message);

  m_messenger.send(0, message, m_serverConnection);
}

void
Client::shutdownServer(int argc, char* argv[]) {
  int rank;
  int clusterSize;
  m_messenger.start(argc, argv, rank, clusterSize);

  m_receiverManager = std::make_shared<ReceiverManager>();

  connectMessenger(m_messenger, m_serverConnection);

  Message message;
  m_messenger.setMessage(ClientCode::SHUTDOWN, message);

  m_messenger.send(0, message, m_serverConnection);
  m_messenger.send(0, message, m_serverConnection);
}

void
replicate(const Messenger& messenger,
                  const Messenger::Connection serverConnection,
                  const std::string& data) {
  bool replicated = false;
  while (replicated == false) {
    Message message;
    nlohmann::json dataJson = {{"value", data}};

    std::string dataJsonString = dataJson.dump();

    messenger.setMessage(ClientCode::REPLICATE, dataJsonString, message);

    messenger.send(0, message, serverConnection);

    using namespace std::chrono;
    auto start = high_resolution_clock::now();
    auto cur = high_resolution_clock::now();
    int elapsed = 0;
    bool messageReceived = false;

    Message responseMessage;
    while ((messageReceived == false) && (elapsed < RESPONSE_WAIT_DURATION)) {
      int srcNodeId;
      messenger.receiveWithTag(MessageTag::CLIENT,
                                 messageReceived,
                                 srcNodeId,
                                 responseMessage,
                                 serverConnection);

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

  m_receiverManager->waitForReceiver(MessageTag::REPL);

  m_messenger.stop();
}

void
Client::replicateCommands() {
  std::string commandFilePath(m_baseDir);
  commandFilePath.append(COMMAND_FILE);

  std::ifstream ifs(commandFilePath);

  std::shared_ptr<ReplManager> replManager =
      m_receiverManager->getReceiver<ReplManager>();

  std::string line;

  bool doneReading;

  do {
    std::getline(ifs, line);

    doneReading = line.empty();

    if (doneReading == false) {
      replicate(m_messenger, m_serverConnection, line);

      replManager->sleep();
    }
  } while (doneReading == false);

  ifs.close();
}
