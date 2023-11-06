#include "WQMonitor.h"

TnStatus WQMonitorInit(WQMonitor* wqm, size_t capacity) {
  assert(wqm);
  TnStatus status;
  int res;

  status = WorkerQueueInit(&wqm->Workers, capacity);
  if (status != STATUS_SUCCESS) return status;

  res = pthread_mutex_init(&wqm->Mutex, NULL);

  if (res != 0) {
    errno = res;
    status = WorkerQueueDestroy(&wqm->Workers);
    assert(status == STATUS_SUCCESS);

    return STATUS_ERRNO_ERROR;
  }

  res = pthread_cond_init(&wqm->CondFull, NULL);

  if (res != 0) {
    errno = res;
    status = WorkerQueueDestroy(&wqm->Workers);
    assert(status == STATUS_SUCCESS);
    res = pthread_mutex_destroy(&wqm->Mutex);
    assert(res == 0);

    return STATUS_ERRNO_ERROR;
  }

  wqm->HasError = 0;

  return STATUS_SUCCESS;
}

TnStatus WQMonitorDestroy(WQMonitor* wqm) {
  TnStatus status;
  int res;

  assert(wqm);

  status = WorkerQueueDestroy(&wqm->Workers);
  assert(status == STATUS_SUCCESS);

  res = pthread_mutex_destroy(&wqm->Mutex);
  assert(res == 0);

  res = pthread_cond_destroy(&wqm->CondFull);
  assert(res == 0);

  return STATUS_SUCCESS;
}

static void WQMonitorLock(WQMonitor* wqm) {
  assert(wqm);
  pthread_mutex_lock(&wqm->Mutex);
}

static void WQMonitorUnlock(WQMonitor* wqm) {
  assert(wqm);
  pthread_mutex_unlock(&wqm->Mutex);
}

TnStatus WQMonitorAddWorker(WQMonitor* wqm, const WorkerID* id) {
  TnStatus status;
  assert(wqm);
  assert(id);

  WQMonitorLock(wqm);
  status = WorkerQueuePush(&wqm->Workers, id);

  if (wqm->Workers.Size == wqm->Workers.Capacity)
    pthread_cond_signal(&wqm->CondFull);

  WQMonitorLock(wqm);

  return status;
}

TnStatus WQMonitorGetWorker(WQMonitor* wqm, WorkerID* id) {
  TnStatus status;
  assert(wqm);
  assert(id);

  WQMonitorLock(wqm);
  status = WorkerQueuePop(&wqm->Workers, id);
  WQMonitorUnlock(wqm);

  return status;
}

TnStatus WQMonitorWaitFull(WQMonitor* wqm) {
  assert(wqm);
  TnStatus status;

  WQMonitorLock(wqm);
  while (!wqm->HasError && wqm->Workers.Size != wqm->Workers.Capacity)
    pthread_cond_wait(&wqm->CondFull, &wqm->Mutex);

  if (wqm->HasError)
    status = STATUS_ERROR_SIGNAL;
  else
    status = STATUS_SUCCESS;
  WQMonitorUnlock(wqm);

  return status;
}

TnStatus WQMonitorSignalError(WQMonitor* wqm) {
  assert(wqm);

  WQMonitorLock(wqm);
  wqm->HasError = 1;
  pthread_cond_signal(&wqm->CondFull);
  WQMonitorUnlock(wqm);

  return STATUS_SUCCESS;
}
