#pragma once
#include "Transmitter.h"

typedef struct {
  Transmitter Transmitter;
  pid_t RxPid;
} TransmitterContext;

static TransmitterContext txCtx;
static void TxSigHandler(int sigNum, siginfo_t* sigInfo, void*);

void TxDriver(pid_t rxPid);