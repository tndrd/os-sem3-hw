#pragma once
#include <stdlib.h>

#include "TnStatus.h"

typedef struct {
  int* Buffer;
  size_t Size;
  size_t Capacity;
  size_t Head;
  size_t Tail;
} Queue;

TnStatus QueueInit(Queue* self, size_t capacity) {
  if (!self) return TNSTATUS(TN_BAD_ARG_PTR);
  if (!capacity) return TNSTATUS(TN_BAD_ARG_VAL);

  int* newBuffer = (int*)malloc(capacity * sizeof(int));
  if (!newBuffer) return TNSTATUS(TN_BAD_ALLOC);

  self->Buffer = newBuffer;
  self->Capacity = capacity;
  self->Size = 0;
  self->Head = 0;
  self->Tail = 0;

  return TN_OK;
}

TnStatus QueueDestroy(Queue* self) {
  if (!self) return TNSTATUS(TN_BAD_ARG_PTR);

  free(self->Buffer);

  return TN_OK;
}

TnStatus QueuePush(Queue* self, const int* val) {
  if (!self || !val) return TNSTATUS(TN_BAD_ARG_PTR);

  if (self->Size == self->Capacity) return TNSTATUS(TN_OVERFLOW);

  self->Buffer[self->Head] = *val;
  self->Head = (self->Head + 1) % self->Capacity;
  self->Size++;

  return TN_OK;
}

TnStatus QueuePop(Queue* self, int* val) {
  if (!self || !val) return TNSTATUS(TN_BAD_ARG_PTR);

  if (self->Size == 0) return TNSTATUS(TN_UNDERFLOW);

  *val = self->Buffer[self->Tail];
  self->Tail = (self->Tail + 1) % self->Capacity;
  self->Size--;

  return TN_OK;
}

TnStatus QueueEmpty(const Queue* self, int* empty) {
  if (!self || !empty) return TNSTATUS(TN_BAD_ARG_PTR);

  *empty = (self->Size == 0);

  return TN_OK;
}