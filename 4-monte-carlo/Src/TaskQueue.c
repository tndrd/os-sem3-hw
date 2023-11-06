#include "TaskQueue.h"

/* Helper */
static void TaskQueueDump(const TaskQueue* tq) {
  assert(tq);
  fprintf(stderr,
      "TaskQueue: \n"
      "  Size: %lu\n"
      "  C-ty: %lu\n"
      "  Head: %lu\n"
      "  Tail: %lu\n"
      "  Data: { ",
      tq->Size, tq->Capacity, tq->Head, tq->Tail);

  for (int i = 0; i < tq->Capacity; ++i)
    fprintf(stderr, "%p ", tq->Tasks[(tq->Tail + i) % tq->Capacity].Args);

  fprintf(stderr, "}\n");
}

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

  return STATUS_SUCCESS;
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
  *task = tq->Tasks[tq->Tail];
  tq->Tail = (tq->Tail + 1) % tq->Capacity;

  return STATUS_SUCCESS;
}

static TnStatus TaskQueueResize(TaskQueue* tq) {
  assert(tq);
  assert(tq->Size == tq->Capacity);
  assert(tq->Size);
  assert(tq->Head == tq->Tail);

  size_t newCapacity = tq->Capacity * 2;
  WorkerTask* newTasks = (WorkerTask*)calloc(newCapacity, sizeof(WorkerTask));

  assert(newTasks);

  size_t center = tq->Head; // == tq->Tail;
  
  size_t nRight = tq->Capacity - center;
  size_t nLeft = center;

  memcpy(newTasks, tq->Tasks + center, nRight * sizeof(WorkerTask));
  memcpy(newTasks + nRight, tq->Tasks, nLeft * sizeof(WorkerTask));
  free(tq->Tasks);

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