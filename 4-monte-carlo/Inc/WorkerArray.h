#pragma once
#include "Worker.h"
#include <malloc.h>

typedef struct {
  Worker* Workers;
  size_t Size;
} WorkerArray;

TnStatus WorkerArrayInit(WorkerArray* workers, size_t size);
TnStatus WorkerArrayRun(WorkerArray* workers, WorkerCallbackT callback);
TnStatus WorkerArrayStop(WorkerArray* workers);
TnStatus WorkerArrayDestroy(WorkerArray* workers);
TnStatus WorkerArrayGet(WorkerArray* workers, WorkerID id, Worker** worker);