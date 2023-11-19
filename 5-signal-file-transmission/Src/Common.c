#include "Common.h"

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