#include <sys/wait.h>

#include <iostream>
#include <string>

#include "Driver.hpp"
#include "IPC/ShMemRx.h"
#include "IPC/ShMemTx.h"

#define SHM_KEY 42

struct ShMemTxOpen {
  key_t Key;

  IPCStatus operator()(ShMemTransmitter* transmitter) {
    return TxOpen(transmitter, Key);
  }
};

struct ShMemRxOpen {
  key_t Key;

  IPCStatus operator()(ShMemReceiver* receiver) {
    return RxOpen(receiver, Key);
  }
};

IPCStatus CreateShm(key_t key, size_t size) {
  if (shmget(key, size, S_IWUSR | S_IWGRP | S_IWOTH | IPC_CREAT) < 0)
    return IPC_ERRNO_ERROR;

  return IPC_SUCCESS;
}

IPCStatus DeleteShm(key_t key, size_t size) {
  int id = shmget(key, size, 0);

  if (id < 0) return IPC_ERRNO_ERROR;

  if (shmctl(id, IPC_RMID, NULL) < 0) return IPC_ERRNO_ERROR;

  return IPC_SUCCESS;
}

int main(int argc, char* argv[]) {
  IPCStatus status;

  if (argc != 3) PrintMessageAndExit("Expected bufSize and srcFile arguments");

  size_t bufSize = std::stoul(argv[1]);
  const char* srcFile = argv[2];

  if ((status = CreateShm(SHM_KEY, bufSize)) != IPC_SUCCESS)
    PrintIPCErrorAndExit("Failed to create shared memory", status);

  ShMemTxOpen openTxFoo{SHM_KEY};
  ShMemRxOpen openRxFoo{SHM_KEY};

  pid_t pid = fork();

  if (pid < 0)
    PrintErrnoAndExit("Failed to fork");
  else if (pid > 0) {  // Parent
    TxDriver<ShMemTransmitter>(bufSize, srcFile, openTxFoo);
    wait(NULL);

    if ((status = DeleteShm(SHM_KEY, bufSize)) != IPC_SUCCESS)
      PrintIPCErrorAndExit("Failed to delete shared memory", status);
  } else  // Child
    RxDriver<ShMemReceiver>(bufSize, "out", openRxFoo);

  return 0;
}