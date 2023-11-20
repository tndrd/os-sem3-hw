#include "TxDriver.h"

static void TxSigHandler(int sigNum, siginfo_t* sigInfo, void* ctxPtr) {
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

  TnStatusAssert(status);
}

void TxDriver(pid_t rxPid) {
  
  fprintf(stderr, "Starting transmitter on pid %d -> %d...\n", getpid(), rxPid);
  
  txCtx.RxPid = rxPid;
  TnStatus status;
  
  status = TransmitterInit(&txCtx.Transmitter, STDIN_FILENO, rxPid, 1);
  TnStatusAssert(status);

  struct sigaction sigAction;

  sigAction.sa_sigaction = TxSigHandler;
  sigAction.sa_flags = SA_SIGINFO;

  sigaction(CMD_SIGNUM, &sigAction, NULL);

  sigset_t set;
  sigfillset(&set);
  pthread_sigmask(SIG_UNBLOCK, &set, NULL);

  status = TransmitterStart(&txCtx.Transmitter);
  TnStatusAssert(status);

  status = TransmitterSpin(&txCtx.Transmitter);
  TnStatusAssert(status);
}