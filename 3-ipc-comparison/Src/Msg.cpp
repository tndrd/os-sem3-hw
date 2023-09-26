#include <sys/wait.h>

#include <iostream>
#include <string>

#include "Driver.hpp"
#include "IPC/MQRx.h"
#include "IPC/MQTx.h"

#define MSG_KEY 69

struct MQTxOpen {
  key_t Key;

  IPCStatus operator()(MQTransmitter* transmitter) {
    return TxOpen(transmitter, Key);
  }
};

struct MQRxOpen {
  key_t Key;

  IPCStatus operator()(MQReceiver* receiver) { return RxOpen(receiver, Key); }
};

IPCStatus CreateMsg(key_t key) {
  if (msgget(key, S_IWUSR | S_IWGRP | S_IWOTH | IPC_CREAT) < 0)
    return IPC_ERRNO_ERROR;

  return IPC_SUCCESS;
}

IPCStatus DeleteMsg(key_t key) {
  int id = msgget(key, 0);
  if (id < 0) return IPC_ERRNO_ERROR;

  if (msgctl(id, IPC_RMID, NULL) < 0) return IPC_ERRNO_ERROR;

  return IPC_SUCCESS;
}

int main(int argc, char* argv[]) {
  IPCStatus status;

  if (argc != 3) PrintMessageAndExit("Expected bufSize and srcFile arguments");

  size_t bufSize = std::stoul(argv[1]);
  const char* srcFile = argv[2];

  if ((status = CreateMsg(MSG_KEY)) != IPC_SUCCESS)
    PrintIPCErrorAndExit("Failed to create msgqueue", status);

  MQTxOpen openTxFoo{MSG_KEY};
  MQRxOpen openRxFoo{MSG_KEY};

  pid_t pid = fork();

  if (pid < 0)
    PrintErrnoAndExit("Failed to fork");
  else if (pid > 0) {  // Parent
    TxDriver<MQTransmitter>(bufSize, srcFile, openTxFoo);
    wait(NULL);

    if ((status = DeleteMsg(MSG_KEY)) != IPC_SUCCESS)
      PrintIPCErrorAndExit("Failed to delete msgqueue", status);
  } else  // Child
    RxDriver<MQReceiver>(bufSize, "out", openRxFoo);

  return 0;
}