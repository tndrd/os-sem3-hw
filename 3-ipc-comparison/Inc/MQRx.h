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
} MQReceiver;

IPCStatus Init(MQReceiver* self, size_t size) {
  if (!self) return IPC_BAD_ARG_PTR;

  char* newBuf = (char*)malloc(size * sizeof(char));
  if (!newBuf) return IPC_BAD_ALLOC;

  self->Msg.Buf = newBuf;
  self->Msg.Type = 1;
  self->Size = size;
  return IPC_SUCCESS;
}

IPCStatus Open(MQReceiver* self, key_t key) {
  if (!self) return IPC_BAD_ARG_PTR;

  int id = msgget(key, S_IRUSR | S_IRGRP | S_IROTH);
  if (id < 0) return IPC_ERRNO_ERROR;

  self->MqId = id;

  return IPC_SUCCESS;
}

IPCStatus Close(MQReceiver* self) {
  if (!self) return IPC_BAD_ARG_PTR;
  free(self->Msg.Buf);
  return IPC_SUCCESS;
}

IPCStatus Receive(MQReceiver* self, int destFd) {
  if (!self) return IPC_BAD_ARG_PTR;

  int readDone = 0;
  size_t nRead;
  IPCStatus status;

  while (1) {
    int ret = msgrcv(self->MqId, &self->Msg, self->Size, -2, 0); 
    if (ret < 0)
      return IPC_ERRNO_ERROR;
    if (ret == 0) break;

    size_t nWrite;
    status = WriteToFd(self->Msg.Buf, ret, destFd, &nWrite);
    if (status != IPC_SUCCESS) return status;
  }

  return IPC_SUCCESS;
}
