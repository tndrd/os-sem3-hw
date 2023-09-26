#pragma once

#include <sys/wait.h>

#include <iostream>
#include <string>

#include "BaseDriver.hpp"
#include "IPC/MQRx.h"
#include "IPC/MQTx.h"

#define MSG_KEY 69

struct MQTxOpen {
  key_t Key;

  IPCStatus operator()(MQTransmitter* transmitter);
};

struct MQRxOpen {
  key_t Key;

  IPCStatus operator()(MQReceiver* receiver);
};

static IPCStatus CreateMsg(key_t key);
static IPCStatus DeleteMsg(key_t key);

double RunMsgDriver(size_t bufSize, const char* srcFile);