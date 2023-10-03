#include "IPC/ShMemRx.h"

IPCStatus RxInit(ShMemReceiver* self, size_t size) {
  if (!self) return IPC_BAD_ARG_PTR;
  self->Size = size;

  return IPC_SUCCESS;
}

IPCStatus RxOpen(ShMemReceiver* self, key_t key) {
  if (!self) return IPC_BAD_ARG_PTR;

  int id = shmget(key, self->Size, 0);
  if (id < 0) return IPC_ERRNO_ERROR;

  char* newPtr = (char*)shmat(id, NULL, 0);
  if (newPtr == (void*)(-1)) return IPC_ERRNO_ERROR;

  self->Ptr = newPtr;
  return IPC_SUCCESS;
}

IPCStatus RxClose(ShMemReceiver* self) {
  if (!self) return IPC_BAD_ARG_PTR;

  if (shmdt(self->Ptr) < 0) return IPC_ERRNO_ERROR;

  return IPC_SUCCESS;
}

IPCStatus RxReceive(ShMemReceiver* self, int destFd) {
  if (!self) return IPC_BAD_ARG_PTR;

  size_t nWrite;
  int readDone = 0;
  IPCStatus status;

  while (GetShmState(self->Ptr) != SHM_SYNC_FINISH) {
    while (GetShmState(self->Ptr) == SHM_SYNC_WRITING)
      ;
    status =
        WriteToFd(GetShmBuf(self->Ptr), GetShmSize(self->Ptr), destFd, &nWrite);
    if (GetShmState(self->Ptr) == SHM_SYNC_READING)
      SetShmState(self->Ptr, SHM_SYNC_WRITING);
  }

  return IPC_SUCCESS;
}
