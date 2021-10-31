#include <fstream>
#include <cassert>
#include <thread>
#include <json.hpp>

#include "messenger.hh"
#include "message-info.hh"

// TODO update this to the actual amount required.
#define MAX_MESSAGE_SIZE 1000
#define SERVER_NAME "server"
#define SEND_WAIT_DURATION 100
#define PUBLISH_PORT_FILEPATH "etc/published-port.txt"

void
serializeMessage(const Message& message, std::string& messageString) {
  nlohmann::json messageJson = {{"code", message.getCode<int>()},
                                {"id", message.getId()},
                                {"data", message.getData()}};

  messageString = messageJson.dump();
}

void
deserializeMessage(const MessagePassKey& passKey,
                   const std::string& messageString,
                   const int& tag,
                   Message& message,
                   bool& isValid) {
  isValid = nlohmann::json::accept(messageString);

  if (isValid) {
    nlohmann::json messageJson = nlohmann::json::parse(messageString);

    int code;
    int id;
    std::shared_ptr<std::string> data = std::make_shared<std::string>();
    std::string& dataStr = *data.get();
    messageJson.at("code").get_to(code);
    messageJson.at("id").get_to(id);
    messageJson.at("data").get_to(dataStr);

    message = Message(passKey, tag, code, id, data);
  }
}

bool
messageShouldDrop(const std::vector<bool>& processIsAlive,
                  const int& nodeId,
                  const int& otherNodeId,
                  const Messenger::Connection& connection,
                  const MessageTag tag) {
  int nodeStatusIndex = otherNodeId < nodeId ? otherNodeId : otherNodeId - 1;

  return !(otherNodeId == nodeId || connection.connection != MPI_COMM_WORLD ||
           (processIsAlive[nodeStatusIndex] == true ||
            tag == MessageTag::FAILURE_DETECTION));
}

void
Messenger::send(const int& dstNodeId,
                const Message& message,
                const Messenger::Connection& connection) const {
  int nodeStatusIndex = dstNodeId < m_rank ? dstNodeId : dstNodeId - 1;

  bool shouldDrop = messageShouldDrop(
      m_processIsAlive, m_rank, dstNodeId, connection, message.getTag());

  if (shouldDrop == false) {
    assert(message.getIsValid());

    std::string messageString;
    messageString.resize(MAX_MESSAGE_SIZE);
    serializeMessage(message, messageString);

    int tag = message.getTagInt();

    if (tag != 3) {
      std::cout << "[" << tag << "]"
                << "[" << m_rank << "]"
                << "[" << dstNodeId << "]: " << messageString << std::endl
                << std::flush;
    }

    MPI_Send(messageString.c_str(),
             MAX_MESSAGE_SIZE,
             MPI_CHAR,
             dstNodeId,
             tag,
             connection.connection);
  }
}

void
Messenger::broadcast(const Message& message,
                     const int& start,
                     const int& end,
                     const bool& includeSelf) const {
  int iend = end == -1 ? m_clusterSize : end;

  for (int i = start; i < iend; i++) {
    if (i != m_rank || includeSelf == true) {
      this->send(i, message);
    }
  }
}

void
receive(const int& nodeId,
        const MessagePassKey& passKey,
        const int& tag,
        const Messenger::Connection& connection,
        int& srcNodeId,
        Message& message,
        bool& isValid) {
  MPI_Status status;

  char messageChar[MAX_MESSAGE_SIZE];


  MPI_Recv(&messageChar,
           MAX_MESSAGE_SIZE,
           MPI_CHAR,
           MPI_ANY_SOURCE,
           tag,
           connection.connection,
           &status);
  std::string messageString(messageChar);
  deserializeMessage(passKey, messageString, status.MPI_TAG, message, isValid);

  srcNodeId = status.MPI_SOURCE;

  // if (tag != 3) {
  //   std::cout << "[i]"
  //             << "[" << tag << "]"
  //             << "[" << srcNodeId << "]"
  //             << "[" << nodeId << "]: " << messageString << std::endl
  //             << std::flush;
  // }
}

void
Messenger::receiveWithTagBlock(const MessageTag& messageTag,
                               int& srcNodeId,
                               Message& message,
                               const Messenger::Connection& connection) const {
  int tag = static_cast<int>(messageTag);
  MessagePassKey passKey;

  bool messageReceived = false;
  while (messageReceived == false) {
    bool isValid;
    receive(m_rank, passKey, tag, connection, srcNodeId, message, isValid);

    bool shouldDrop = messageShouldDrop(
        m_processIsAlive, m_rank, srcNodeId, connection, messageTag);
    messageReceived = shouldDrop == false || isValid == false;
  }
}

void
hasPendingWithTag(const MessageTag& messageTag,
                  bool& hasPending,
                  int& srcNodeId,
                  const Messenger::Connection& connection) {
  int tag = static_cast<int>(messageTag);
  MPI_Status status;
  int flag;
  MPI_Iprobe(MPI_ANY_SOURCE, tag, connection.connection, &flag, &status);

  hasPending = flag == 1;

  if (hasPending == true) {
    srcNodeId = status.MPI_SOURCE;
  }
}

void
Messenger::receiveWithTag(const MessageTag& messageTag,
                          bool& messageReceived,
                          int& srcNodeId,
                          Message& message,
                          const Messenger::Connection& connection) const {
  hasPendingWithTag(messageTag, messageReceived, srcNodeId, connection);
  if (messageReceived == true) {
    bool shouldDrop = messageShouldDrop(
        m_processIsAlive, m_rank, srcNodeId, connection, messageTag);
    if (shouldDrop == false) {
      receiveWithTagBlock(messageTag, srcNodeId, message, connection);
    } else {
      srcNodeId = -1;
      messageReceived = false;
    }
  }
}

void
Messenger::start(int argc, char** argv) {
  int provided;
  MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);

  if (provided < MPI_THREAD_MULTIPLE) {
    std::cerr << "messenger.cc: Multithreading not supported." << std::endl;
    MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
  }

  MPI_Comm_rank(MPI_COMM_WORLD, &m_rank);

  MPI_Comm_size(MPI_COMM_WORLD, &m_clusterSize);

  m_processIsAlive.resize(m_clusterSize - 1);

  for (int i = 0; i < m_clusterSize - 1; i++) {
    m_processIsAlive[i] = true;
  }
}

void
Messenger::stop() const {
  MPI_Finalize();
}

void
Messenger::openPort(std::string& port) const {
  port.resize(PORT_STRING_SIZE);
  char cPort[PORT_STRING_SIZE];
  MPI_Open_port(MPI_INFO_NULL, cPort);

  port = std::string(cPort);  
}

void
Messenger::closePort(const std::string& port) const {
  MPI_Close_port(port.c_str());
}

void
Messenger::publishPort(const std::string& port) {
  std::ofstream ofs(PUBLISH_PORT_FILEPATH);

  ofs << port;

  ofs.close();
}

void
Messenger::acceptConnBlock(const std::string& port,
                           Messenger::Connection& connection) const {
  MPI_Comm_accept(
      port.c_str(), MPI_INFO_NULL, 0, MPI_COMM_SELF, &connection.connection);
  std::cout << "accepted" << std::endl;
}

void
Messenger::generateUniqueId(const int& nodeId, int& id) const {
  int time = static_cast<int>(MPI_Wtime());

  id = time;
}

void
Messenger::lookupServerPort(std::string& port) const {
  std::ifstream ifs(PUBLISH_PORT_FILEPATH);

  std::getline(ifs, port);

  ifs.close();
}

void
Messenger::connect(const std::string& port,
                   Messenger::Connection& connection) const {
  MPI_Comm_connect(
      port.c_str(), MPI_INFO_NULL, 0, MPI_COMM_SELF, &connection.connection);
  std::cout << "connected" << std::endl;
}

void
Messenger::disconnect(Messenger::Connection& connection) const {
  std::cout << "disconnected" << std::endl;
  MPI_Comm_disconnect(&connection.connection);
}

int
Messenger::getClusterSize() const {
  return m_clusterSize;
}

int
Messenger::getRank() const {
  return m_rank;
}

void
Messenger::setNodeStatus(const int& nodeIndex, const bool& isAlive) {
  std::unique_lock<std::mutex> lock(m_mutex);

  m_processIsAlive[nodeIndex] = isAlive;
}
