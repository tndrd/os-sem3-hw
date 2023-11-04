#include "TQMonitor.h"

TnStatus TQMonitorInit(TQMonitor* tqm) {
  TnStatus status;
  int res;

  status = TaskQueueInit(&tqm->Tasks);
  if (status != STATUS_SUCCESS) return status;

  res = pthread_mutex_init(&tqm->Mutex, NULL);

  if (res != 0) {
    errno = res;
    status = TaskQueueDestroy(&tqm->Tasks);
    assert(status == STATUS_SUCCESS);

    return STATUS_ERRNO_ERROR;
  }

  res = pthread_cond_init(&tqm->CondEmpty, NULL);

  if (res != 0) {
    errno = res;
    status = TaskQueueDestroy(&tqm->Tasks);
    assert(status == STATUS_SUCCESS);
    res = pthread_mutex_destroy(&tqm->Mutex);
    assert(res == 0);

    return STATUS_ERRNO_ERROR;
  }

  return STATUS_SUCCESS;
}

TnStatus TQMonitorDestroy(TQMonitor* tqm) {
  TnStatus status;
  int res;

  assert(tqm);

  status = TaskQueueDestroy(&tqm->Tasks);
  assert(status == STATUS_SUCCESS);

  res = pthread_mutex_destroy(&tqm->Mutex);
  assert(res == 0);

  res = pthread_cond_destroy(&tqm->CondEmpty);
  assert(res == 0);

  return STATUS_SUCCESS;
}

static void TQMonitorLock(TQMonitor* tqm) {
  assert(tqm);
  pthread_mutex_lock(&tqm->Mutex);
}

static void TQMonitorUnlock(TQMonitor* tqm) {
  assert(tqm);
  pthread_mutex_unlock(&tqm->Mutex);
}

TnStatus TQMonitorAddTask(TQMonitor* tqm, const WorkerTask* task) {
  TnStatus status;
  assert(tqm);
  assert(task);

  TQMonitorLock(tqm);
  status = TaskQueuePush(&tqm->Tasks, task);
  TQMonitorUnlock(tqm);

  return status;
}

TnStatus TQMonitorGetTask(TQMonitor* tqm, WorkerTask* task) {
  TnStatus status;
  assert(tqm);
  assert(task);

  TQMonitorLock(tqm);
  status = TaskQueuePop(&tqm->Tasks, task);
  
  if (tqm->Tasks.Size == 0)
    pthread_cond_signal(&tqm->CondEmpty);
  
  TQMonitorUnlock(tqm);

  return status;
}

TnStatus TQMonitorWaitEmpty(TQMonitor* tqm) {
  assert(tqm);

  TQMonitorLock(tqm);
  while (tqm->Tasks.Size != 0)
    pthread_cond_wait(&tqm->CondEmpty, &tqm->Mutex);
  TQMonitorUnlock(tqm);

  return STATUS_SUCCESS;
}
