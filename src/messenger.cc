#include <mpi.h>
#include <cassert>

#include <json.hpp>

#include "messenger.hh"
#include "message-info.hh"

// TODO update this to the actual amount required.
#define MESSAGE_SIZE 1000

void
serializeMessage(const Message& message, std::string& messageString) {
  nlohmann::json messageJson = {{"code", message.getCode<int>()},
               {"id", message.getId()},
               {"command", message.getCommand()}};

  messageString = messageJson.dump();
}

void
deserializeMessage(const MessagePassKey& passKey,
                   const std::string& messageString,
                   Message& message) {
  nlohmann::json messageJson = nlohmann::json::parse(messageString);

  message.setCode<int>(passKey, messageJson["code"]);
  message.setId(passKey, messageJson["id"]);
  message.setCommand(passKey, messageJson["command"]);

  message.setToValid(passKey);
}

void
Messenger::send(const int& dstNodeId, const Message& message) const {
  assert(message.getIsValid());

  std::string messageString;
  serializeMessage(message, messageString);

  messageString.resize(MESSAGE_SIZE);

  int tag = message.getTagInt();
  MPI_Send(messageString.c_str(),
           MESSAGE_SIZE,
           MPI_CHAR,
           dstNodeId,
           tag,
           MPI_COMM_WORLD);
}

void
Messenger::receiveBlock(int& srcNodeId, Message& message) const {
  MPI_Status status;

  char messageChar[MESSAGE_SIZE];

  MPI_Recv(&messageChar,
           MESSAGE_SIZE,
           MPI_CHAR,
           MPI_ANY_SOURCE,
           MPI_ANY_TAG,
           MPI_COMM_WORLD,
           &status);

  MessagePassKey passKey;
  std::string messageString(messageChar);
  deserializeMessage(passKey, messageString, message);

  message.setTag(passKey, status.MPI_TAG);

  srcNodeId = status.MPI_SOURCE;
}

// TODO make a third inner function to be used in with the receive function
// to reduce duplicated code
void
Messenger::receiveWithTagBlock(const MessageTag& messageTag,
                               int& srcNodeId,
                               Message& message) const {
  int tag = static_cast<int>(messageTag);
  MPI_Status status;

  char messageChar[MESSAGE_SIZE];

  MPI_Recv(&messageChar,
           MESSAGE_SIZE,
           MPI_CHAR,
           MPI_ANY_SOURCE,
           tag,
           MPI_COMM_WORLD,
           &status);

  MessagePassKey passKey;
  std::string messageString(messageChar);
  deserializeMessage(passKey, messageString, message);

  srcNodeId = status.MPI_SOURCE;
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
