#pragma once

#include <errno.h>
#include <string.h>

// Error codes
typedef enum {
  SH_SUCCESS,
  SH_BAD_ARG_PTR,
  SH_BAD_ALLOC,
  SH_OUT_OF_RANGE,
  SH_STRING_LEN_MAX_SIZE,
  SH_ERRNO_ERROR,
  SH_PROGRAM_FAILURE
} ShellStatus;

const char* GetErrorDescription(ShellStatus status);