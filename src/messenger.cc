#include <cassert>
#include <json.hpp>

#include "messenger.hh"
#include "message-info.hh"

// TODO update this to the actual amount required.
#define MAX_MESSAGE_SIZE 1000
#define SERVER_NAME "server"

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
                   Message& message) {
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

void
Messenger::send(const int& dstNodeId,
                const Message& message,
                const Messenger::Connection& connection) const {
  assert(message.getIsValid());

  std::string messageString;
  messageString.resize(MAX_MESSAGE_SIZE);
  serializeMessage(message, messageString);

  int tag = message.getTagInt();
  MPI_Send(messageString.c_str(),
           MAX_MESSAGE_SIZE,
           MPI_CHAR,
           dstNodeId,
           tag,
           connection.connection);
}

void
receive(const MessagePassKey& passKey,
        const int& tag,
        const Messenger::Connection& connection,
        int& srcNodeId,
        Message& message) {
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
  deserializeMessage(passKey, messageString, status.MPI_TAG, message);

  srcNodeId = status.MPI_SOURCE;
}

void
Messenger::receiveBlock(int& srcNodeId,
                        Message& message,
                        const Messenger::Connection& connection) const {
  MessagePassKey passKey;
  receive(passKey, MPI_ANY_TAG, connection, srcNodeId, message);
}

void
Messenger::receiveWithTagBlock(const MessageTag& messageTag,
                               int& srcNodeId,
                               Message& message,
                               const Messenger::Connection& connection) const {
  int tag = static_cast<int>(messageTag);
  MessagePassKey passKey;
  receive(passKey, tag, connection, srcNodeId, message);
}

void
Messenger::hasPendingWithTag(const MessageTag& messageTag,
                             bool& hasPending,
                             const Messenger::Connection& connection) const {
  int tag = static_cast<int>(messageTag);
  MPI_Status status;
  int flag;
  MPI_Iprobe(MPI_ANY_SOURCE, tag, connection.connection, &flag, &status);

  hasPending = flag == 1;
}

void
Messenger::probeTagBlock(const Connection& connection,
                         MessageTag& messageTag) const {
  MPI_Status status;
  MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, connection.connection, &status);

  messageTag = static_cast<MessageTag>(status.MPI_TAG);
}

void
Messenger::receiveWithTag(const MessageTag& messageTag,
                          bool& messageReceived,
                          int& srcNodeId,
                          Message& message,
                          const Messenger::Connection& connection) const {
  hasPendingWithTag(messageTag, messageReceived, connection);
  if (messageReceived == true) {
    receiveWithTagBlock(messageTag, srcNodeId, message, connection);
  }
}

void
Messenger::start(int argc, char** argv, int& rank, int& clusterSize) {
  int provided;
  MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);

  if (provided < MPI_THREAD_MULTIPLE) {
    std::cerr << "messenger.cc: Multithreading not supported." << std::endl;
    MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
  }

  MPI_Comm_rank(MPI_COMM_WORLD, &m_rank);
  rank = m_rank;

  MPI_Comm_size(MPI_COMM_WORLD, &clusterSize);
  m_clusterSize = clusterSize;
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
Messenger::publishPort(const std::string& port) const {
  MPI_Info scopeInfo;
  MPI_Info_create(&scopeInfo);
  MPI_Info_set(scopeInfo, "ompi_global_scope", "true");

  MPI_Publish_name(SERVER_NAME, scopeInfo, port.c_str());

  std::cout << "messenger pusblished: " << m_rank << std::endl;
}

void
Messenger::unpublishPort(const std::string& port) const {
  MPI_Info scopeInfo;
  MPI_Info_create(&scopeInfo);
  MPI_Info_set(scopeInfo, "ompi_global_scope", "true");

  MPI_Unpublish_name(SERVER_NAME, scopeInfo, port.c_str());

  std::cout << "messenger unpusblished" << std::endl;
}

void
Messenger::acceptConnBlock(const std::string& port,
                           Messenger::Connection& connection) const {
  MPI_Comm_accept(
      port.c_str(), MPI_INFO_NULL, 0, MPI_COMM_SELF, &connection.connection);
  std::cout << "connection accepted" << std::endl;
}

void
Messenger::generateUniqueId(const int& nodeId, int& id) const {
  int time = static_cast<int>(MPI_Wtime());

  id = time | nodeId;
}

void
Messenger::lookupServerPort(std::string& port) const {
  MPI_Info scopeInfo;
  MPI_Info_create(&scopeInfo);
  MPI_Info_set(scopeInfo, "ompi_lookup_order", "global");

  char cPort[PORT_STRING_SIZE];
  MPI_Lookup_name(SERVER_NAME, scopeInfo, &cPort[0]);
  port = std::string(cPort);
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
