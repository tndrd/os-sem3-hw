#include <iostream>
#include <string>

#include "Driver.hpp"
#include "IPC/MQTx.h"
#include "IPC/MQRx.h"

struct MQTxOpen {
  key_t Key;

  IPCStatus operator()(MQTransmitter* transmitter) {
    return TxOpen(transmitter, Key);
  }
};

struct MQRxOpen {
  key_t Key;

  IPCStatus operator()(MQReceiver* receiver) {
    return RxOpen(receiver, Key);
  }
};

int main(int argc, char* argv[]) {
  if (argc != 4)
    PrintMessageAndExit("Expected bufSize, key and srcFile arguments");

  size_t bufSize = std::stoul(argv[1]);
  key_t key = std::stoi(argv[2]);
  const char* srcFile = argv[3];

  MQTxOpen openTxFoo{key};
  MQRxOpen openRxFoo{key};

  pid_t pid = fork();

  if (pid < 0)
    PrintErrnoAndExit("Failed to fork");
  else if (pid > 0) // Parent
    TxDriver<MQTransmitter>(bufSize, srcFile, openTxFoo);
  else // Child
    RxDriver<MQReceiver>(bufSize, "out", openRxFoo);

  return 0;
}