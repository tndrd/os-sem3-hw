#pragma once
#include "WorkerQueue.h"
#include "errno.h"
#include "pthread.h"

typedef struct {
  WorkerQueue Workers;

  pthread_mutex_t Mutex;
  pthread_cond_t CondFull;

  int HasError;
} WQMonitor;

TnStatus WQMonitorInit(WQMonitor* wqm, size_t capacity);
TnStatus WQMonitorDestroy(WQMonitor* wqm);

static void WQMonitorLock(WQMonitor* wqm);
static void WQMonitorUnlock(WQMonitor* wqm);

TnStatus WQMonitorAddWorker(WQMonitor* wqm, const WorkerID* id);
TnStatus WQMonitorGetWorker(WQMonitor* wqm, WorkerID* id);
TnStatus WQMonitorWaitFull(WQMonitor* wqm);
TnStatus WQMonitorSignalError(WQMonitor* wq);
