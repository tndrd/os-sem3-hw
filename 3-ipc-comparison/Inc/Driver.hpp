#pragma once

#include <IPC/IPCStatus.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <iostream>
#include <string>

void PrintErrnoAndExit(const std::string& msg) {
  perror(msg.c_str());
  exit(1);
}

void PrintIPCErrorAndExit(const std::string& msg, IPCStatus status) {
  std::cerr << msg + ": " + std::string{GetErrorDescription(status)} << std::endl;
  exit(1);
}

void PrintMessageAndExit(const std::string& msg) {
  std::cerr << msg << std::endl;
  exit(1);
}

template <typename Transmitter, typename F>
void TxDriver(size_t bufSize, const char* srcPath, F openFoo) {
  Transmitter transmitter;
  IPCStatus status;

  if ((status = TxInit(&transmitter, bufSize)) != IPC_SUCCESS)
    PrintIPCErrorAndExit("Failed to init", status);

  if ((status = openFoo(&transmitter)) != IPC_SUCCESS)
    PrintIPCErrorAndExit("Failed to open ipc class", status);

  int srcFd;
  if ((srcFd = open(srcPath, O_RDONLY)) < 0)
    PrintErrnoAndExit("Failed to open source file");

  std::cerr << "Running transmitter driver... \n" << std::endl;

  time_t startTime = clock();

  if ((status = TxTransmit(&transmitter, srcFd)) != IPC_SUCCESS)
    PrintIPCErrorAndExit("Failed to transmit", status);

  clock_t endTime = clock();
  double timeSpent = (double)(endTime - startTime) / CLOCKS_PER_SEC;

  std::cerr << "Transmission done in " << timeSpent << " seconds" << std::endl;
  std::cout << int(timeSpent) << std::endl;

  TxClose(&transmitter);
}

template <typename Receiver, typename F>
void RxDriver(size_t bufSize, const char* destPath, F openFoo) {
  Receiver receiver;
  IPCStatus status;

  if ((status = RxInit(&receiver, bufSize)) != IPC_SUCCESS)
    PrintIPCErrorAndExit("Failed to init", status);

  if ((status = openFoo(&receiver)) != IPC_SUCCESS)
    PrintIPCErrorAndExit("Failed to open ipc class", status);

  int destFd;
  if ((destFd = open(destPath, O_WRONLY | O_CREAT | O_TRUNC)) < 0)
    PrintErrnoAndExit("Failed to open destination file");

  printf("Running receiver driver...\n");

  time_t startTime = clock();

  if ((status = RxReceive(&receiver, destFd)) != IPC_SUCCESS)
    PrintIPCErrorAndExit("Failed to receive", status);

  clock_t endTime = clock();
  double timeSpent = (double)(endTime - startTime) / CLOCKS_PER_SEC;

  printf("Receiving done in %lf seconds\n", timeSpent);

  RxClose(&receiver);
}
