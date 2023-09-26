#include <iostream>
#include <string>

#include "Driver.hpp"
#include "IPC/FIFOTx.h"
#include "IPC/FIFORx.h"

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

int main(int argc, char* argv[]) {
  if (argc != 4)
    PrintMessageAndExit("Expected bufSize, fifoFile and srcFile arguments");

  size_t bufSize = std::stoul(argv[1]);
  const char* fifoFile = argv[2];
  const char* srcFile = argv[3];

  FifoTxOpen openTxFoo{fifoFile};
  FifoRxOpen openRxFoo{fifoFile};

  pid_t pid = fork();

  if (pid < 0)
    PrintErrnoAndExit("Failed to fork");
  else if (pid > 0) // Parent
    TxDriver<FifoTransmitter>(bufSize, srcFile, openTxFoo);
  else // Child
    RxDriver<FifoReceiver>(bufSize, "out", openRxFoo);

  return 0;
}