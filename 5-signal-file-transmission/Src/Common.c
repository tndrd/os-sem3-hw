#include "Common.h"

TnStatus SendSignal(pid_t pid, int sigNo, sigval_t value) {
  while (1) {
    int ret = sigqueue(pid, sigNo, value);
    if (ret == 0) return TN_OK;
    if (ret == -1 && errno != EAGAIN) return TNSTATUS(TN_ERRNO);
  }
}