#pragma once
#include "Transmitter.h"

typedef struct {
  Transmitter Transmitter;
  pid_t RxPid;
} TransmitterContext;

static TransmitterContext txCtx;

void TxSigHandler(int sigNum, siginfo_t* sigInfo, void* ctxPtr) {
  TransmitterContext* ctx = &txCtx;
  TnStatus status;
  assert(ctx);
  assert(sigInfo);

  int val = sigInfo->si_value.sival_int;
  pid_t pid = sigInfo->si_pid;

  if (pid != ctx->RxPid) {
    fprintf(stderr, "Error: process %d tried to access transmitter\n", pid);
    exit(1);
  }

  if (sigNum == CMD_SIGNUM) {
    status = TransmitterControlCallback(&ctx->Transmitter, val);
  } else {
    fprintf(stderr, "Error: unexpected signal received\n");
    exit(1);
  }

  AssertTnStatus(status);
}

void TxMain(pid_t rxPid) {
  fprintf(stderr, "Starting transmitter on pid %d -> %d\n...", getpid(), rxPid);

  txCtx.RxPid = rxPid;
  TnStatus status;

  status = TransmitterInit(&txCtx.Transmitter, STDIN_FILENO, rxPid, 1);
  AssertTnStatus(status);

  struct sigaction sigAction;

  sigset_t sigMask;
  sigemptyset(&sigMask);

  sigAction.sa_sigaction = TxSigHandler;
  sigAction.sa_mask = sigMask;
  sigAction.sa_flags = SA_SIGINFO;

  sigaction(CMD_SIGNUM, &sigAction, NULL);

  status = TransmitterStart(&txCtx.Transmitter);
  AssertTnStatus(status);

  status = TransmitterSpin(&txCtx.Transmitter);
  AssertTnStatus(status);
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Wrong arguments, expected <RX_PID>\n");
    exit(1);
  }

  pid_t rxPid = atoi(argv[1]);

  TxMain(rxPid);
}