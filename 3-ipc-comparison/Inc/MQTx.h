#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <unistd.h>

#include "Helpers.h"
#include "IPCStatus.h"

typedef struct {
  long Type;
  char* Buf;
} MsgBuf;

typedef struct {
  int MqId;
  size_t Size;

  MsgBuf Msg;
} MQTransmitter;

IPCStatus Init(MQTransmitter* self, size_t size) {
  if (!self) return IPC_BAD_ARG_PTR;

  char* newBuf = (char*)malloc(size * sizeof(char));
  if (!newBuf) return IPC_BAD_ALLOC;

  self->Msg.Buf = newBuf;
  self->Msg.Type = 1;
  self->Size = size;
  return IPC_SUCCESS;
}

IPCStatus Open(MQTransmitter* self, key_t key) {
  if (!self) return IPC_BAD_ARG_PTR;

  int id = msgget(key, S_IWUSR | S_IWGRP | S_IWOTH);
  if (id < 0) return IPC_ERRNO_ERROR;

  self->MqId = id;

  return IPC_SUCCESS;
}

IPCStatus Close(MQTransmitter* self) {
  if (!self) return IPC_BAD_ARG_PTR;
  free(self->Msg.Buf);
  return IPC_SUCCESS;
}

IPCStatus Transmit(MQTransmitter* self, int srcFd) {
  if (!self) return IPC_BAD_ARG_PTR;

  int readDone = 0;
  size_t nRead;
  IPCStatus status;

  while (!readDone) {
    status = ReadToBuf(self->Msg.Buf, self->Size, srcFd, &nRead, &readDone);

    if (msgsnd(self->MqId, &self->Msg, nRead, 0) < 0)
      return IPC_ERRNO_ERROR;
  }

  if (msgsnd(self->MqId, &self->Msg, 0, 0) < 0)
      return IPC_ERRNO_ERROR;

  return IPC_SUCCESS;
}
