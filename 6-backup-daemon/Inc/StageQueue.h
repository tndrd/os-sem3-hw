#pragma once
#include <assert.h>
#include <stdlib.h>

#include "TnStatus.h"
#include "Stage.h"

#define STAGE_QUEUE_INITIAL_CAPACITY 2

typedef struct {
  Logger* logger;

  Stage* Buffer;
  size_t Size;
  size_t Capacity;
  size_t Head;
  size_t Tail;
} StageQueue;

TnStatus StageQueueInit(StageQueue* self, Logger* logger);
TnStatus StageQueueDestroy(StageQueue* self);
TnStatus StageQueuePush(StageQueue* self, const int* val);
TnStatus StageQueuePop(StageQueue* self, int* val);

static TnStatus StageQueueResize(StageQueue* self);