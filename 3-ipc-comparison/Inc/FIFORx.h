#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include "IPCStatus.h"
#include "Helpers.h"

typedef struct {
  int Fd;
  char* Buf;
  size_t BufSize;
} FifoReceiver;

IPCStatus Init(FifoReceiver* self, size_t bufSize) {
  if (!self) return IPC_BAD_ARG_PTR;

  char* newBuf = (char*)malloc(bufSize * sizeof(char));
  if (!newBuf) return IPC_BAD_ALLOC;

  self->Buf = newBuf;
  self->BufSize = bufSize;

  return IPC_SUCCESS;
}

IPCStatus Open(FifoReceiver* self, const char* path) {
  if (!self || !path) return IPC_BAD_ARG_PTR;

  int newFd;
  if ((newFd = open(path, O_RDONLY)) < 0) return IPC_ERRNO_ERROR;

  self->Fd = newFd;

  return IPC_SUCCESS;
}


IPCStatus Close(FifoReceiver* self) {
  if (!self) return IPC_BAD_ARG_PTR;
  free(self->Buf);
  close(self->Fd);
  return IPC_SUCCESS;
}

IPCStatus Receive(FifoReceiver* self, int destFd) {
  if (!self) return IPC_BAD_ARG_PTR;

  IPCStatus status;
  size_t nRead;
  int readDone = 0;

  while (!readDone) {
    status = ReadToBuf(self->Buf, self->BufSize, self->Fd, &nRead, &readDone);
    if (status != IPC_SUCCESS) return status;

    size_t nSend;
    status = WriteToFd(self->Buf, nRead, destFd, &nSend);
    if (status != IPC_SUCCESS) return status;
  }

  return IPC_SUCCESS;
}
