#pragma once
#include "Receiver.h"

typedef struct {
  Receiver Receiver;
  pid_t TxPid;
} ReceiverContext;

static ReceiverContext rxCtx;
static void RxSigHandler(int sigNum, siginfo_t* sigInfo, void*);

void RxDriver();