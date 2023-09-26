#include <iostream>
#include <string>

#include "Driver.hpp"
#include "IPC/ShMemTx.h"
#include "IPC/ShMemRx.h"

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

int main(int argc, char* argv[]) {
  if (argc != 4)
    PrintMessageAndExit("Expected bufSize, key and srcFile arguments");

  size_t bufSize = std::stoul(argv[1]);
  key_t key = std::stoi(argv[2]);
  const char* srcFile = argv[3];

  ShMemTxOpen openTxFoo{key};
  ShMemRxOpen openRxFoo{key};

  pid_t pid = fork();

  if (pid < 0)
    PrintErrnoAndExit("Failed to fork");
  else if (pid > 0) // Parent
    TxDriver<ShMemTransmitter>(bufSize, srcFile, openTxFoo);
  else // Child
    RxDriver<ShMemReceiver>(bufSize, "out", openRxFoo);

  return 0;
}