#pragma once

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>
#include <string>

#include "BaseDriver.hpp"
#include "IPC/FIFORx.h"
#include "IPC/FIFOTx.h"

#define FIFO_FILE "FIFO"

struct FifoTxOpen {
  const char* FifoFile;

  IPCStatus operator()(FifoTransmitter* transmitter);
};

struct FifoRxOpen {
  const char* FifoFile;

  IPCStatus operator()(FifoReceiver* receiver);
};

static IPCStatus CreateFifo(const char* path);
static IPCStatus DeleteFifo(const char* path);

double RunFifoDriver(size_t bufSize, const char* srcFile);