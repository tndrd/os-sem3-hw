#include "WorkerQueue.h"

TnStatus WorkerQueueInit(WorkerQueue* wq, size_t capacity) {
  assert(wq);
  assert(capacity > 0);

  WorkerID* newBuffer = (WorkerID*)malloc(capacity * sizeof(WorkerID));
  if (!newBuffer) return STATUS_BAD_ALLOC;

  wq->Buffer = newBuffer;
  wq->Size = 0;
  wq->Tail = 0;
  wq->Head = 0;
  wq->Capacity = capacity;

  return STATUS_SUCCESS;
}

TnStatus WorkerQueueDestroy(WorkerQueue* wq) {
  assert(wq);

  free(wq->Buffer);
  return STATUS_SUCCESS;
}

TnStatus WorkerQueuePush(WorkerQueue* wq, const WorkerID* id) {
  assert(wq);
  assert(id);

  if (wq->Size == wq->Capacity)
    return STATUS_OVERFLOW;

  wq->Buffer[wq->Head] = *id;
  wq->Head = (wq->Head + 1) % wq->Capacity;
  wq->Size++;

  return STATUS_SUCCESS;
}

TnStatus WorkerQueuePop(WorkerQueue* wq, WorkerID* id) {
  assert(wq);
  assert(id);

  if (wq->Size == 0)
    return STATUS_UNDERFLOW;

  wq->Size--;
  *id = wq->Tail;
  wq->Tail = (wq->Tail + 1) % wq->Capacity;

  return STATUS_SUCCESS;
}

TnStatus WorkerQueueSize(const WorkerQueue* wq, size_t* size) {
  assert(wq);
  assert(size);

  *size = wq->Size;

  return STATUS_SUCCESS;
}