#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>
#include <string>

#include "Driver.hpp"
#include "IPC/FIFORx.h"
#include "IPC/FIFOTx.h"

#define FIFO_FILE "FIFO"

struct FifoTxOpen {
  const char* FifoFile;

  IPCStatus operator()(FifoTransmitter* transmitter) {
    return TxOpen(transmitter, FifoFile);
  }
};

struct FifoRxOpen {
  const char* FifoFile;

  IPCStatus operator()(FifoReceiver* receiver) {
    return RxOpen(receiver, FifoFile);
  }
};

IPCStatus CreateFifo(const char* path) {
  if (!path) return IPC_BAD_ARG_PTR;

  int ret = mknod(path, S_IFIFO, 0);
  if (ret < 0 && errno != EEXIST) return IPC_ERRNO_ERROR;

  return IPC_SUCCESS;
}

IPCStatus DeleteFifo(const char* path) {
  if (!path) return IPC_BAD_ARG_PTR;

  return (unlink(path) < 0) ? IPC_ERRNO_ERROR : IPC_SUCCESS;
}

int main(int argc, char* argv[]) {
  IPCStatus status;

  if (argc != 3) PrintMessageAndExit("Expected bufSize and srcFile arguments");

  size_t bufSize = std::stoul(argv[1]);
  const char* srcFile = argv[2];

  if ((status = CreateFifo(FIFO_FILE)) != IPC_SUCCESS)
    PrintIPCErrorAndExit("Failed to create fifo", status);

  FifoTxOpen openTxFoo{FIFO_FILE};
  FifoRxOpen openRxFoo{FIFO_FILE};

  pid_t pid = fork();

  if (pid < 0)
    PrintErrnoAndExit("Failed to fork");
  else if (pid > 0) {  // Parent
    TxDriver<FifoTransmitter>(bufSize, srcFile, openTxFoo);
    wait(NULL);

    if ((status = DeleteFifo(FIFO_FILE)) != IPC_SUCCESS)
      PrintIPCErrorAndExit("Failed to delete fifo", status);
  } else  // Child
    RxDriver<FifoReceiver>(bufSize, "out", openRxFoo);

  return 0;
}