#include "TnStatus.h"

const char *TnStatusGetDescription(TnStatus status) {
  switch (status) {
  case TN_SUCCESS:
    return "Success";
  case TN_BAD_ARG_PTR:
    return "Bad argument pointer";
  case TN_WORKER_BAD_FOO:
    return "Worker: bad function argument";
  case TN_WORKER_NOT_READY:
    return "Worker: not ready";
  case TN_WORKER_HAS_ERROR:
    return "Worker: has error";
  case TN_BAD_VALUE:
    return "Worker: bad value";
  case TN_ERRNO_ERROR:
    return "Check errno";
  case TILER_FINISHED:
    return "Tiler finished";
  default:
    return "Unknown error";
  }
}