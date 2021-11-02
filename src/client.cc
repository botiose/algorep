#include <chrono>
#include <thread>
#include <json.hpp>
#include <fstream>

#include "client.hh"
#include "repl-manager.hh"

#define RESPONSE_WAIT_DURATION 40
#define LOOP_SLEEP_DURATION 50
#define TURN_SLEEP_DURATION 1

#define REPL_MSG_BASE_FILEPATH "etc/client/"
#define TURN_FILEPATH "etc/turn.txt"
#define REPL_FILE "repl.txt"
#define COMMAND_FILE "command.txt"


void
Client::init(int argc, char* argv[]) {
  m_messenger.start(argc, argv);

  m_receiverManager = std::make_shared<ReceiverManager>();

  m_clientId = std::stoi(argv[1]);
  m_clientCount = std::stoi(argv[2]);

  m_baseDir = REPL_MSG_BASE_FILEPATH;
  m_baseDir.append(argv[1]);
  m_baseDir.append("/");

  std::string replFilePath(m_baseDir);
  replFilePath.append(REPL_FILE);

  std::shared_ptr<ReplManager> replManager = std::make_shared<ReplManager>(
      m_messenger, m_receiverManager, replFilePath);

  m_receiverManager->startReceiver(replManager);

  replManager->sleep();
}

void
connectMessenger(Messenger& messenger,
                 Messenger::Connection& serverConnection) {
  std::string port;
  messenger.lookupServerPort(port);
  messenger.connect(port, serverConnection);
}

void
connectToServer(Messenger& messenger, Messenger::Connection& serverConnection) {
  connectMessenger(messenger, serverConnection);
}

void
disconnect(Messenger& messenger, Messenger::Connection& serverConnection) {
  Message message;
  messenger.setMessage(ClientCode::DISCONNECT, message);

  messenger.send(0, message, serverConnection);

  messenger.disconnect(serverConnection);
}

void
replicate(Messenger& messenger,
          Messenger::Connection& serverConnection,
          const std::string& data) {
  bool replicated = false;
  while (replicated == false) {
    connectToServer(messenger, serverConnection);

    Message message;
    nlohmann::json dataJson = {{"value", data}};

    std::string dataJsonString = dataJson.dump();

    messenger.setMessage(ClientCode::REPLICATE, dataJsonString, message);

    messenger.send(0, message, serverConnection);

    std::string str("sent: ");
    str.append(data);
    print::printString(0, str);

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

    if (replicated == false) {
      disconnect(messenger, serverConnection);
    }
  }

  disconnect(messenger, serverConnection);
}

void
Client::destroy() {
  m_receiverManager->waitForReceiver(MessageTag::REPL);
  m_messenger.stop();
}


void
Client::shutdownServer(int argc, char* argv[]) {
  m_messenger.start(argc, argv);

  m_receiverManager = std::make_shared<ReceiverManager>();

  connectMessenger(m_messenger, m_serverConnection);

  Message message;
  m_messenger.setMessage(ClientCode::SHUTDOWN, message);

  m_messenger.send(0, message, m_serverConnection);

  disconnect(m_messenger, m_serverConnection);

  m_messenger.stop();
}

void
waitForTurn(std::ifstream& ifs, const int& clientId) {
  bool isUp = true;
  while (isUp == true) {
    std::string line;
    std::getline(ifs, line);
    ifs.clear();

    if (line.empty() == false) {
      isUp = line != "shutdown" && std::stoi(line) != clientId;
    }

    std::this_thread::sleep_for(std::chrono::seconds(TURN_SLEEP_DURATION));
  }
}

void
setTurn(const int& clientId, const int& clientCount) {
  std::ofstream ofs(TURN_FILEPATH, std::ios_base::app);  

  ofs << ((clientId + 1) % clientCount) << std::endl;

  ofs.close();
}

void
Client::replicateCommands() {
  std::string commandFilePath(m_baseDir);
  commandFilePath.append(COMMAND_FILE);

  std::ifstream commandIfs(commandFilePath);
  std::ifstream turnIfs(TURN_FILEPATH);

  std::shared_ptr<ReplManager> replManager =
      m_receiverManager->getReceiver<ReplManager>();

  std::string line;

  bool doneReading;

  do {
    std::getline(commandIfs, line);

    doneReading = line.empty();

    if (doneReading == false) {

      waitForTurn(turnIfs, m_clientId);

      replicate(m_messenger, m_serverConnection, line);

      setTurn(m_clientId, m_clientCount);

      replManager->sleep();
    }
  } while (doneReading == false);

  commandIfs.close();
  turnIfs.close();
}
