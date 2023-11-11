#pragma once
#include <errno.h>
#include <unistd.h>
#include <signal.h>

#include "Common.h"
#include "QMonitor.h"

typedef struct {
  QMonitor QMonitor;
  pthread_t Thread;

  TnStatus Status;
  int Errno;

  int DoStop;
  int Fd;
  pid_t TxPid;
} Receiver;

void* ReceiverMainLoop(void* selfPtr);

TnStatus ReceiverInit(Receiver* self, int fd, size_t queueCapacity) {
  if (!self) return TNSTATUS(TN_BAD_ARG_PTR);

  self->Fd = fd;
  self->DoStop = 0;
  self->Status = TN_OK;
  self->Errno = 0;

  return QMonitorInit(&self->QMonitor, queueCapacity);
}

TnStatus ReceiverDestroy(Receiver* self) {
  if (!self) return TNSTATUS(TN_BAD_ARG_PTR);

  return QMonitorDestroy(&self->QMonitor);
}

TnStatus ReceiverStart(Receiver* self, pid_t txPid) {
  if (!self) return TNSTATUS(TN_BAD_ARG_PTR);

  int ret = pthread_create(&self->Thread, NULL, ReceiverMainLoop, self);
  if (ret != 0) {
    errno = ret;
    return TNSTATUS(TN_ERRNO);
  }

  self->TxPid = txPid;

  return TN_OK;
}

TnStatus ReceiverStop(Receiver* self) {
  if (!self) return TNSTATUS(TN_BAD_ARG_PTR);

  self->DoStop = 1;
  QMonitorSignal(&self->QMonitor);

  return TN_OK;
}

static TnStatus WriteToFd(int fd, int val) {
  const char* buf = (const char*)(&val);
  size_t size = sizeof(int);
  size_t total = 0;

  while (total != size) {
    int ret = write(fd, buf + total, size - total);
    if (ret < 0) return TNSTATUS(TN_ERRNO);
    total += ret;
  }

  return TN_OK;
}

void* ReceiverMainLoop(void* selfPtr) {
  assert(selfPtr);
  Receiver* self = (Receiver*)self;
  sigval_t sigVal;

  sigVal.sival_int = CMD_CONNECT;
  sigqueue(self->TxPid, CMD_SIGNUM, sigVal);

  while (1) {
    int empty;
    while (1) {
      QMonitorEmpty(&self->QMonitor, &empty);
      if (!empty || self->DoStop) break;
      QMonitorSleepUntilEvent(&self->QMonitor);
    }

    if (self->DoStop && empty) break;

    int val;
    while (QMonitorPop(&self->QMonitor, &val).Code != TN_UNDERFLOW) {
      TnStatus status = WriteToFd(self->Fd, val);
      if (!TnStatusOk(status)) {
        self->Status = status;
        self->Errno = errno;
        return NULL;
      }
    }
  }

  sigVal.sival_int = CMD_FINISH;
  sigqueue(self->TxPid, CMD_SIGNUM, sigVal);

  self->Status = TN_OK;
  return NULL;
}

TnStatus ReceiverControlCallback(Receiver* self, int cmd, pid_t txPid) {
  assert(self);
  TnStatus status;

  switch (cmd) {
    case CMD_START:
      fprintf(stderr, "Receiver connecting to %d... \n", txPid);
      return ReceiverStart(self, txPid);
    case CMD_STOP:
      fprintf(stderr, "Receiver stopping...\n");
      return ReceiverStop(self);
    default:
      return TNSTATUS(TN_BAD_ARG_VAL);
  }
}

TnStatus ReceiverValueCallback(Receiver* self, int val) {
  assert(self);
  TnStatus status = QMonitorPush(&self->QMonitor, &val);
  assert(TnStatusOk(status));

  return status;
}

TnStatus ReceiverSpin(Receiver* self) {
  if (!self) return TNSTATUS(TN_BAD_ARG_PTR);

  int ret = pthread_join(self->Thread, NULL);

  if (ret != 0) {
    errno = ret;
    return TNSTATUS(TN_ERRNO);
  }

  errno = self->Errno;
  return self->Status;
}