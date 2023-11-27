#include "StageQueue.h"

TnStatus StageQueueInit(StageQueue* self, Logger* logger) {
  if (!self) return TNSTATUS(TN_BAD_ARG_PTR);

  Stage* newBuffer = (Stage*)malloc(STAGE_QUEUE_INITIAL_CAPACITY * sizeof(Stage));
  if (!newBuffer) return TNSTATUS(TN_BAD_ALLOC);

  self->Buffer = newBuffer;
  self->Logger = logger;
  self->Capacity = STAGE_QUEUE_INITIAL_CAPACITY;
  self->Size = 0;
  self->Head = 0;
  self->Tail = 0;

  return TN_OK;
}

TnStatus StageQueueDestroy(StageQueue* self) {
  if (!self) return TNSTATUS(TN_BAD_ARG_PTR);

  free(self->Buffer);

  return TN_OK;
}

static TnStatus StageQueueResize(StageQueue* self) {
  assert(self);
  assert(self->Size == self->Capacity);
  assert(self->Size);
  assert(self->Head == self->Tail);

  size_t newCapacity = self->Capacity * 2;

  Stage* newBuf = (Stage*)calloc(newCapacity, sizeof(Stage));
  if (!newBuf) return TNSTATUS(TN_BAD_ALLOC);

  size_t center = self->Head;
  size_t nRight = self->Capacity - center;
  size_t nLeft = center;

  memcpy(newBuf, self->Buffer + center, nRight * sizeof(Stage));
  memcpy(newBuf + nRight, self->Buffer, nLeft * sizeof(Stage));
  free(self->Buffer);

  self->Capacity = newCapacity;
  self->Buffer = newBuf;
  self->Tail = 0;
  self->Head = self->Size;

  return TN_OK;
}

TnStatus StageQueuePush(StageQueue* self, const Stage* val) {
  if (!self || !val) return TNSTATUS(TN_BAD_ARG_PTR);

  if (self->Size == self->Capacity) {
    TnStatus status = StageQueueResize(self);
    if (!TnStatusOk(status)) return status;
  }

  self->Buffer[self->Head] = *val;
  self->Head = (self->Head + 1) % self->Capacity;
  self->Size++;

  return TN_OK;
}

TnStatus StageQueuePop(StageQueue* self, Stage* val) {
  if (!self || !val) return TNSTATUS(TN_BAD_ARG_PTR);

  if (self->Size == 0) return TNSTATUS(TN_UNDERFLOW);

  *val = self->Buffer[self->Tail];
  self->Tail = (self->Tail + 1) % self->Capacity;
  self->Size--;

  return TN_OK;
}