#include <mpi.h>

#include "messenger.hh"

#define MESSAGE_CODE_COUNT 1

Messenger::Messenger(const int& nodeId) : m_nodeId(nodeId) {
}

void
Messenger::sendMessage(const int& dstNodeId, const Message& message) const {
  MPI_Send(&message.code,
           MESSAGE_CODE_COUNT,
           MPI_INT,
           dstNodeId,
           message.tag,
           MPI_COMM_WORLD);
}

void
Messenger::receiveMessage(int& srcNodeId, Message& message) const {
  MPI_Status status;

  MPI_Recv(&message.code,
           MESSAGE_CODE_COUNT,
           MPI_INT,
           MPI_ANY_SOURCE,
           MPI_ANY_TAG,
           MPI_COMM_WORLD,
           &status);

  message.tag = status.MPI_TAG;
  srcNodeId = status.MPI_SOURCE;
}

void
Messenger::receiveMessageWithTag(const MessageTag& messageTag,
                                 int& srcNodeId,
                                 Message& message) const {
  int tag = static_cast<int>(messageTag);
  MPI_Status status;

  MPI_Recv(&message.code,
           MESSAGE_CODE_COUNT,
           MPI_INT,
           MPI_ANY_SOURCE,
           tag,
           MPI_COMM_WORLD,
           &status);

  message.tag = status.MPI_TAG;
  srcNodeId = status.MPI_SOURCE;
}

bool
Messenger::hasMessageWithTag(const MessageTag& messageTag) const {
  int tag = static_cast<int>(messageTag);
  MPI_Status status;
  int flag;
  MPI_Iprobe(MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &flag, &status);

  return flag == 1;
}
