#pragma once
#include <signal.h>

#include "TnStatus.h"

#define CMD_STOP 0xFF
#define CMD_CONNECT 0xEE
#define CMD_FINISH 0xBB
#define CMD_START 0xAA

#define BASE_SIGNUM SIGRTMIN

#define DATA_INT_SIGNUM BASE_SIGNUM
#define DATA_CHAR_SIGNUM BASE_SIGNUM + 1
#define CMD_SIGNUM BASE_SIGNUM + 2

void AssertTnStatus(TnStatus status) {
  if (TnStatusOk(status)) return;

  fprintf(stderr, "Error: ");
  TnStatusPrintDescription(status);
  fprintf(stderr, "\n");
  exit(1);
}

TnStatus SendSignal(pid_t pid, int sigNo, sigval_t value) {
  while (1) {
    int ret = sigqueue(pid, sigNo, value);
    if (ret == 0) return TN_OK;
    if (ret == -1 && errno != EAGAIN) return TNSTATUS(TN_ERRNO);
  }
}