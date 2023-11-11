#pragma once
#include "Queue.h"
#include <pthread.h>
#include <assert.h>

typedef struct {
  Queue Queue;
  pthread_mutex_t Mutex;
  pthread_cond_t Cond;
} QMonitor;

TnStatus QMonitorInit(QMonitor* self, size_t capacity) {
  if (!self) return TNSTATUS(TN_BAD_ARG_PTR);
  if (!capacity) return TNSTATUS(TN_BAD_ARG_VAL);
  TnStatus status;

  status = QueueInit(&self->Queue, capacity);
  if(!TnStatusOk(status)) return status;

  pthread_mutex_init(&self->Mutex, NULL);
  pthread_cond_init(&self->Cond, NULL);

  return TN_OK;
}

TnStatus QMonitorDestroy(QMonitor* self) {
  if (!self) return TNSTATUS(TN_BAD_ARG_PTR);
  
  pthread_mutex_destroy(&self->Mutex);
  pthread_cond_destroy(&self->Cond);

  return QueueDestroy(&self->Queue);
}

static void QMonitorLock(QMonitor* self) {
  assert(self);
  pthread_mutex_lock(&self->Mutex);
}

static void QMonitorUnlock(QMonitor* self) {
  assert(self);
  pthread_mutex_unlock(&self->Mutex);
}

static void QMonitorSignal_Internal(QMonitor* self) {
  assert(self);
  pthread_cond_signal(&self->Cond);
}

static void QMonitorSignalIf(QMonitor* self, int cond) {
  assert(self);
  if (!cond) return;
  QMonitorSignal_Internal(self);
}

TnStatus QMonitorPush(QMonitor* self, const int* val) {
  if (!self) return TNSTATUS(TN_BAD_ARG_PTR);

  QMonitorLock(self);
  TnStatus status = QueuePush(&self->Queue, val);
  QMonitorSignalIf(self, status.Code == TN_OVERFLOW);
  QMonitorUnlock(self);

  return status;
}

TnStatus QMonitorPop(QMonitor* self, int* val) {
  if (!self) return TNSTATUS(TN_BAD_ARG_PTR);

  QMonitorLock(self);
  TnStatus status = QueuePop(&self->Queue, val);
  QMonitorSignalIf(self, status.Code == TN_UNDERFLOW);
  QMonitorUnlock(self);

  return status;
}

TnStatus QMonitorSleepUntilEvent(QMonitor* self) {
  if (!self) return TNSTATUS(TN_BAD_ARG_PTR);
  pthread_cond_wait(&self->Cond, &self->Mutex);
  
  return TN_OK;
}

TnStatus QMonitorSignal(QMonitor* self) {
  if (!self) return TNSTATUS(TN_BAD_ARG_PTR);
  QMonitorSignal_Internal(self);
  return TN_OK;
}

TnStatus QMonitorEmpty(QMonitor* self, int* empty) {
  if (!self) return TNSTATUS(TN_BAD_ARG_PTR);
  
  QMonitorLock(self);
  TnStatus status = QueueEmpty(&self->Queue, empty);
  QMonitorUnlock(self);

  return status;
}