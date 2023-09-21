#pragma once

typedef enum {
  IPC_SUCCESS,
  IPC_BAD_ARG_PTR,
  IPC_BAD_ALLOC,
  IPC_ERRNO_ERROR,
  IPC_ZERO_WRITE
} IPCStatus;