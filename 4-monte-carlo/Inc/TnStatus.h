#pragma once

#include <string.h>
#include <errno.h>
#include <stdio.h>

typedef enum
{
  TN_SUCCESS,
  TN_BAD_ARG_PTR,
  TN_WORKER_BAD_FOO,
  TN_WORKER_NOT_READY,
  TN_WORKER_HAS_ERROR,
  TN_BAD_VALUE,
  TN_ERRNO_ERROR,
  TILER_FINISHED
} TnStatus;

const char *TnStatusGetDescription(TnStatus status);