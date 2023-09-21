#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <unistd.h>

#include "Helpers.h"
#include "IPCStatus.h"

typedef struct {
  char* Ptr;
  size_t Size;
} ShMemTransmitter;

IPCStatus Init(ShMemTransmitter* self, size_t size) {
  if (!self) return IPC_BAD_ARG_PTR;
  self->Size = size;

  return IPC_SUCCESS;
}

IPCStatus Open(ShMemTransmitter* self, key_t key) {
  if (!self) return IPC_BAD_ARG_PTR;

  int id = shmget(key, self->Size, 666);
  if (id < 0) return IPC_ERRNO_ERROR;

  char* newPtr = (char*)shmat(id, NULL, 0);
  if (newPtr == (void*)(-1)) return IPC_ERRNO_ERROR;

  self->Ptr = newPtr;
  return IPC_SUCCESS;
}

IPCStatus Close(ShMemTransmitter* self) {
  if (!self) return IPC_BAD_ARG_PTR;

  if (shmdt(self->Ptr) < 0)
    return IPC_ERRNO_ERROR;

  return IPC_SUCCESS;
}

IPCStatus Transmit(ShMemTransmitter* self, int srcFd) {
  if (!self) return IPC_BAD_ARG_PTR;
  
  size_t nRead;
  int readDone = 0;
  IPCStatus status;

  self->Ptr[0] = 1;
  while (self->Ptr[0] != 2) {
    while(self->Ptr[0] == 1);
    status = ReadToBuf(self->Ptr + 1 + sizeof(size_t), self->Size - 1 - sizeof(size_t), srcFd, &nRead, &readDone);
    *((size_t*)(self->Ptr + 1)) = nRead;
    if (readDone)
      self->Ptr[0] = 2;
    else
      self->Ptr[0] = 1;
  }

  return IPC_SUCCESS;
}
