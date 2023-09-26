#pragma once

#include <sys/wait.h>

#include <iostream>
#include <string>

#include "BaseDriver.hpp"
#include "IPC/ShMemRx.h"
#include "IPC/ShMemTx.h"

#define SHM_KEY 42

struct ShMemTxOpen {
  key_t Key;

  IPCStatus operator()(ShMemTransmitter* transmitter);
};

struct ShMemRxOpen {
  key_t Key;

  IPCStatus operator()(ShMemReceiver* receiver);
};

static IPCStatus CreateShm(key_t key, size_t size);
static IPCStatus DeleteShm(key_t key, size_t size);

double RunShmDriver(size_t bufSize, const char* srcFile);