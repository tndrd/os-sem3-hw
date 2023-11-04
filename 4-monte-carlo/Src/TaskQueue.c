#include "TaskQueue.h"

TnStatus TaskQueueInit(TaskQueue* tq) {
  assert(tq);

  tq->Capacity = TQ_INITIAL_CAPACITY;

  tq->Tasks = (WorkerTask*)malloc(tq->Capacity * sizeof(WorkerTask));
  if (!tq->Tasks) return STATUS_BAD_ALLOC;

  tq->Size = 0;
  tq->Head = 0;
  tq->Tail = 0;

  return STATUS_SUCCESS;
}

TnStatus TaskQueueDestroy(TaskQueue* tq) {
  assert(tq);
  free(tq->Tasks);
}

TnStatus TaskQueuePush(TaskQueue* tq, const WorkerTask* task) {
  TnStatus status;

  assert(tq);
  assert(task);

  if (tq->Size == tq->Capacity) {
    status = TaskQueueResize(tq);
    assert(status == STATUS_SUCCESS);
  }

  assert(tq->Size < tq->Capacity);

  tq->Tasks[tq->Head] = *task;
  tq->Head = (tq->Head + 1) % tq->Capacity;
  tq->Size++;

  return STATUS_SUCCESS;
}

TnStatus TaskQueuePop(TaskQueue* tq, WorkerTask* task) {
  TnStatus status;

  assert(tq);
  assert(task);

  if (tq->Size == 0) return STATUS_UNDERFLOW;

  tq->Size--;
  tq->Tail = (tq->Head + 1) % tq->Capacity;

  return STATUS_SUCCESS;
}

static TnStatus TaskQueueResize(TaskQueue* tq) {
  assert(tq);
  assert(tq->Size == tq->Capacity);
  assert(tq->Size);
  assert(tq->Head == tq->Tail);

  size_t newCapacity = tq->Capacity * 2;
  WorkerTask* newTasks = (WorkerTask*)malloc(newCapacity * sizeof(WorkerTask));

  assert(newTasks);

  memcpy(newTasks, tq->Tasks + tq->Tail, tq->Capacity - tq->Tail);
  memcpy(newTasks + tq->Tail, tq->Tasks, tq->Head);

  tq->Capacity = newCapacity;
  tq->Tasks = newTasks;
  tq->Tail = 0;
  tq->Head = tq->Size;

  return STATUS_SUCCESS;
}

TnStatus TaskQueueSize(const TaskQueue* tq, size_t* size) {
  assert(tq);
  assert(size);

  *size = tq->Size;

  return STATUS_SUCCESS;
}