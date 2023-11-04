#include "WorkerArray.h"

TnStatus WorkerArrayInit(WorkerArray* workers, size_t size) {
  TnStatus status;
  assert(workers);
  assert(size);

  size_t newSize = size;
  Worker* newWorkers = (Worker*)malloc(newSize * sizeof(Worker));

  if (!newWorkers) return STATUS_BAD_ALLOC;

  int created = 0;
  for (; created < newSize; ++created) {
    status = WorkerInit(newWorkers + created, created);
    if (status != STATUS_SUCCESS) break;
  }

  if (status == STATUS_SUCCESS) {
    workers->Workers = newWorkers;
    workers->Size = newSize;
    return status;
  }

  TnStatus oldStatus = status;

  for (int i = created - 1; i >= 0; --i) {
    status = WorkerDestroy(newWorkers + i);
    assert(status == STATUS_SUCCESS);
  }

  return oldStatus;
}

TnStatus WorkerArrayDestroy(WorkerArray* workers) {
  TnStatus status;
  assert(workers);

  for (int i = 0; i < workers->Size; ++i) {
    status = WorkerDestroy(workers->Workers + i);
    assert(status == STATUS_SUCCESS);
  }

  free(workers->Workers);

  return STATUS_SUCCESS;
}

TnStatus WorkerArrayGet(WorkerArray* workers, WorkerID id, Worker** workerPtr) {
  assert(workers);
  assert(workerPtr);

  if (id + 1 > workers->Size) return STATUS_NOT_FOUND;

  *workerPtr = &workers->Workers[id];

  return STATUS_SUCCESS;
}

TnStatus WorkerArrayRun(WorkerArray* workers, WorkerCallbackT callback) {
  TnStatus status;
  Worker* worker;
  assert(workers);
  
  int i = 0;
  for (; i < workers->Size; ++i) {
    status = WorkerArrayGet(workers, i, &worker);
    assert(status == STATUS_SUCCESS);

    status = WorkerRun(worker, callback);
    if (status != STATUS_SUCCESS) break;
  }

  if (status == STATUS_SUCCESS) return status;
  TnStatus oldStatus = status;

  for (i = i - 1; i >= 0; --i) {
    status = WorkerArrayGet(workers, i, &worker);
    assert(status == STATUS_SUCCESS);

    status = WorkerStop(worker);
    assert(status == STATUS_SUCCESS);
  }

  return oldStatus;
}

TnStatus WorkerArrayStop(WorkerArray* workers) {
  TnStatus status;
  Worker* worker;
  assert(workers);
  
  
  for (int i = 0; i < workers->Size; ++i) {
    status = WorkerArrayGet(workers, i, &worker);
    assert(status == STATUS_SUCCESS);

    status = WorkerStop(worker);
    assert(status == STATUS_SUCCESS);
  }

  return STATUS_SUCCESS;
}
