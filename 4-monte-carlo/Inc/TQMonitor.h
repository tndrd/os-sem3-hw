#pragma once
#include "TaskQueue.h"
#include "errno.h"
#include "pthread.h"

typedef struct {
  TaskQueue Tasks;

  pthread_mutex_t Mutex;
  pthread_cond_t CondEmpty;
} TQMonitor;

TnStatus TQMonitorInit(TQMonitor* tqm);
TnStatus TQMonitorDestroy(TQMonitor* tqm);

static void TQMonitorLock(TQMonitor* tqm);
static void TQMonitorUnlock(TQMonitor* tqm);

TnStatus TQMonitorAddTask(TQMonitor* tqm, const WorkerTask* task);
TnStatus TQMonitorGetTask(TQMonitor* tqm, WorkerTask* task);
TnStatus TQMonitorWaitEmpty(TQMonitor* tqm);
