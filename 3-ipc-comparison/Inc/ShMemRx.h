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
} ShMemReceiver;

IPCStatus Init(ShMemReceiver* self, size_t size) {
  if (!self) return IPC_BAD_ARG_PTR;
  self->Size = size;

  return IPC_SUCCESS;
}

IPCStatus Open(ShMemReceiver* self, key_t key) {
  if (!self) return IPC_BAD_ARG_PTR;

  int id = shmget(key, self->Size, 666);
  if (id < 0) return IPC_ERRNO_ERROR;

  char* newPtr = (char*)shmat(id, NULL, 0);
  if (newPtr == (void*)(-1)) return IPC_ERRNO_ERROR;

  self->Ptr = newPtr;
  return IPC_SUCCESS;
}

IPCStatus Close(ShMemReceiver* self) {
  if (!self) return IPC_BAD_ARG_PTR;

  if (shmdt(self->Ptr) < 0)
    return IPC_ERRNO_ERROR;

  return IPC_SUCCESS;
}

IPCStatus Receive(ShMemReceiver* self, int destFd) {
  if (!self) return IPC_BAD_ARG_PTR;
  
  size_t nWrite;
  int readDone = 0;
  IPCStatus status;

  while (1) {
    while(self->Ptr[0] == 0);
    size_t sz = *((size_t*)(self->Ptr + 1));
    status = WriteToFd(self->Ptr + 1 + sizeof(size_t), sz, destFd, &nWrite);
    if (self->Ptr[0] == 1)
      self->Ptr[0] = 0;
    else break;
  }

  return IPC_SUCCESS;
}
