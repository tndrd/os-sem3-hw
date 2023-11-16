#include "Receiver.h"

typedef struct {
  Receiver Receiver;
  pid_t TxPid;
} ReceiverContext;

static ReceiverContext rxCtx;

void RxSigHandler(int sigNum, siginfo_t* sigInfo, void* ctxPtr) {
  // ReceiverContext* ctx = (ReceiverContext*)(ctxPtr);
  ReceiverContext* ctx = &rxCtx;
  TnStatus status;
  assert(ctx);
  assert(sigInfo);

  int val = sigInfo->si_value.sival_int;
  pid_t pid = sigInfo->si_pid;

  if (ctx->TxPid && ctx->TxPid != pid) {
    fprintf(stderr, "Error: process %d tried to access receiver\n", pid);
    exit(1);
  }

  if (sigNum == CMD_SIGNUM) {
    if (ctx->TxPid == 0) ctx->TxPid = pid;
    status = ReceiverControlCallback(&ctx->Receiver, val, pid);
    if (TnStatusOk(status)) fprintf(stderr, "OK\n");

  } else if (sigNum == DATA_INT_SIGNUM) {
    if (ctx->TxPid == 0) {
      fprintf(stderr, "Error: receiver not started but data received\n");
      exit(1);
    }
    status = ReceiverIntCallback(&ctx->Receiver, val);
  } else if (sigNum == DATA_CHAR_SIGNUM) {
    if (ctx->TxPid == 0) {
      fprintf(stderr, "Error: receiver not started but data received\n");
      exit(1);
    }
    status = ReceiverCharCallback(&ctx->Receiver, (char)val);
  } else {
    fprintf(stderr, "Error: unexpected signal received\n");
    exit(1);
  }

  AssertTnStatus(status);
}

void RxMain() {
  fprintf(stderr, "Starting receiver on pid %d...\n", getpid());

  rxCtx.TxPid = 0;
  TnStatus status;

  int sigQueueCapacity = 1;//sysconf(_SC_SIGQUEUE_MAX) * 1000;
  status = ReceiverInit(&rxCtx.Receiver, STDOUT_FILENO, sigQueueCapacity);
  AssertTnStatus(status);

  struct sigaction sigAction;
  sigAction.sa_sigaction = RxSigHandler;
  sigemptyset(&sigAction.sa_mask);
  sigaddset(&sigAction.sa_mask, DATA_INT_SIGNUM);
  sigaddset(&sigAction.sa_mask, DATA_CHAR_SIGNUM);
  sigaddset(&sigAction.sa_mask, CMD_SIGNUM);
  sigAction.sa_flags = SA_SIGINFO;

  if (sigaction(DATA_INT_SIGNUM, &sigAction, NULL) != 0) {
    perror("sigaction");
    exit(1);
  }

  if (sigaction(DATA_CHAR_SIGNUM, &sigAction, NULL) != 0) {
    perror("sigaction");
    exit(1);
  }

  if (sigaction(CMD_SIGNUM, &sigAction, NULL) != 0) {
    perror("sigaction");
    exit(1);
  }

  AssertTnStatus(status);

  status = ReceiverSpin(&rxCtx.Receiver);
  AssertTnStatus(status);
}

int main() { RxMain(); }