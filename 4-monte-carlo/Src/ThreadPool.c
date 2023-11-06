#include "ThreadPool.h"

static void WorkerCallback(Worker* worker, void* args) {
  assert(worker);
  assert(args);

  ThreadPool* tp = (ThreadPool*)args;
  WorkerState state = worker->State;
  TnStatus status;
  WorkerTask task;

  if (state == WORKER_FREE) {
    // Worker is free, try to get new task
    status = TQMonitorGetTask(&tp->Tasks, &task);

    if (status == STATUS_SUCCESS) {
      // Task available, assign
      status = WorkerAssignTask(worker, task);
      assert(status == STATUS_SUCCESS);

    } else if (status == STATUS_UNDERFLOW) {
      // No availble tasks, wait for new ones
      status = WQMonitorAddWorker(&tp->FreeWorkers, &worker->ID);
      assert(status == STATUS_SUCCESS);
    }
  } else if (state == WORKER_DONE) {
    // Worker has finished, so we can let it continue
    status = WorkerFinish(worker);
    assert(status == STATUS_SUCCESS);
  } else if (state == WORKER_BUSY) {
    // Worker is starting to work on task, do nothing
  } else {
    assert(0);  // Placeholder for error handling
  }
}

TnStatus ThreadPoolInit(ThreadPool* tp, size_t nWorkers) {
  TnStatus status;
  assert(tp);
  assert(nWorkers > 0);

  status = TQMonitorInit(&tp->Tasks);
  if (status != STATUS_SUCCESS) return status;

  status = WQMonitorInit(&tp->FreeWorkers, nWorkers);
  if (status != STATUS_SUCCESS) {
    assert(TQMonitorDestroy(&tp->Tasks) == STATUS_SUCCESS);
  }

  status = WorkerArrayInit(&tp->Workers, nWorkers);
  if (status != STATUS_SUCCESS) {
    assert(TQMonitorDestroy(&tp->Tasks) == STATUS_SUCCESS);
    assert(WQMonitorDestroy(&tp->FreeWorkers) == STATUS_SUCCESS);
    return status;
  }

  return STATUS_SUCCESS;
}

TnStatus ThreadPoolRun(ThreadPool* tp) {
  assert(tp);

  WorkerCallbackT callback;
  callback.Args = tp;
  callback.Function = WorkerCallback;

  return WorkerArrayRun(&tp->Workers, callback);
}

TnStatus ThreadPoolStop(ThreadPool* tp) {
  assert(tp);
  return WorkerArrayStop(&tp->Workers);
}

TnStatus ThreadPoolDestroy(ThreadPool* tp) {
  assert(tp);

  assert(TQMonitorDestroy(&tp->Tasks) == STATUS_SUCCESS);
  assert(WQMonitorDestroy(&tp->FreeWorkers) == STATUS_SUCCESS);
  assert(WorkerArrayDestroy(&tp->Workers) == STATUS_SUCCESS);

  return STATUS_SUCCESS;
}

TnStatus ThreadPoolAddTask(ThreadPool* tp, WorkerTask task) {
  assert(tp);
  TnStatus status;
  WorkerID workerID;
  Worker* worker;

  status = WQMonitorGetWorker(&tp->FreeWorkers, &workerID);

  if (status == STATUS_SUCCESS) {  // Has free worker, assign task
    status = WorkerArrayGet(&tp->Workers, workerID, &worker);
    assert(status == STATUS_SUCCESS);

    status = WorkerAssignTask(worker, task);

    if (status != STATUS_SUCCESS)  // Failed to assign
      assert(WQMonitorAddWorker(&tp->FreeWorkers, &workerID) == STATUS_SUCCESS);

  } else if (status == STATUS_UNDERFLOW) {  // No free workers, save task
    status = TQMonitorAddTask(&tp->Tasks, &task);
  }

  return status;
}

TnStatus ThreadPoolWaitAll(ThreadPool* tp) {
  TnStatus status;
  assert(tp);

  status =
      TQMonitorWaitEmpty(&tp->Tasks);  // Wait for all the tasks to be taken
  if (status != STATUS_SUCCESS) return status;

  status = WQMonitorWaitFull(
      &tp->FreeWorkers);  // Wait for all the workers to finish
  if (status != STATUS_SUCCESS) return status;

  return status;
}