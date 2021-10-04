#include <mpi.h>
#include <cassert>

#include <json.hpp>

#include "messenger.hh"
#include "message-info.hh"

// TODO update this to the actual amount required.
#define MAX_MESSAGE_SIZE 1000

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
Messenger::send(const int& dstNodeId, const Message& message) const {
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
           MPI_COMM_WORLD);
}

void
receive(const MessagePassKey& passKey,
        const int& tag,
        int& srcNodeId,
        Message& message) {
  MPI_Status status;

  char messageChar[MAX_MESSAGE_SIZE];

  MPI_Recv(&messageChar,
           MAX_MESSAGE_SIZE,
           MPI_CHAR,
           MPI_ANY_SOURCE,
           tag,
           MPI_COMM_WORLD,
           &status);

  std::string messageString(messageChar);
  deserializeMessage(passKey, messageString, status.MPI_TAG, message);

  srcNodeId = status.MPI_SOURCE;
}

void
Messenger::receiveBlock(int& srcNodeId, Message& message) const {
  MessagePassKey passKey;
  receive(passKey, MPI_ANY_TAG, srcNodeId, message);
}

void
Messenger::receiveWithTagBlock(const MessageTag& messageTag,
                               int& srcNodeId,
                               Message& message) const {
  int tag = static_cast<int>(messageTag);
  MessagePassKey passKey;
  receive(passKey, tag, srcNodeId, message);
}

void
Messenger::hasPendingWithTag(const MessageTag& messageTag,
                             bool& hasPending) const {
  int tag = static_cast<int>(messageTag);
  MPI_Status status;
  int flag;
  MPI_Iprobe(MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &flag, &status);

  hasPending = flag == 1;
}

void
Messenger::receiveWithTag(const MessageTag& messageTag,
                          bool& messageReceived,
                          int& srcNodeId,
                          Message& message) const {
  hasPendingWithTag(messageTag, messageReceived);
  if (messageReceived == true) {
    receiveWithTagBlock(messageTag, srcNodeId, message);
  }
}

void
Messenger::start(int& rank, int& clusterSize) {
  MPI_Init(0, 0);

  MPI_Comm_rank(MPI_COMM_WORLD, &m_rank);
  rank = m_rank;

  MPI_Comm_size(MPI_COMM_WORLD, &clusterSize);
}

void
Messenger::stop() const {
  MPI_Finalize();
}

void
Messenger::publish() const {
  // TODO implement
}

void
Messenger::acceptConnection() const {
  // TODO implement
}
