#pragma once
#include <signal.h>
#include "TnStatus.h"

#define CMD_STOP 0xFF
#define CMD_CONNECT 0xEE
#define CMD_FINISH 0xBB
#define CMD_START 0xAA

#define DATA_SIGNUM SIGRTMIN
#define CMD_SIGNUM SIGRTMAX

void AssertTnStatus(TnStatus status) {
  if (TnStatusOk(status)) return;

  fprintf(stderr, "Error: ");
  TnStatusPrintDescription(status);
  fprintf(stderr, "\n");
  exit(1);
}