#pragma once

#include "TnStatus.h"
#include "WorkerContent.h"
#include <assert.h>
#include <math.h>
#include <pthread.h>

typedef enum
{
  WORKER_INIT,
  WORKER_READY,
  WORKER_BUSY,
  WORKER_DONE,
  WORKER_STOP,
  WORKER_ERROR
} WorkerState;

typedef struct
{
  WorkerState State;
  TnStatus Error;
  WorkerTLS TLS;

  WorkerFoo Target;
  WorkerArgs Args;
  WorkerResult Result;

  pthread_t Thread;
  int Errno;
} Worker;

static TnStatus WorkerValidatePrecondition(Worker *worker);
static TnStatus WorkerValidatePostcondition(Worker *worker);
static void* WorkerLoop(void * ptr);

TnStatus WorkerInit(Worker *worker);
const char *WorkerGetErrorDescription(TnStatus status, const Worker *worker);
TnStatus WorkerSetTarget(Worker *worker, WorkerFoo foo);
TnStatus WorkerSetArgs(Worker *worker, WorkerArgs *args);
TnStatus WorkerRun(Worker *worker);
TnStatus WorkerReadResult(Worker *worker, WorkerResult *result);
TnStatus WorkerStop(Worker *worker);
TnStatus WorkerGetState(const Worker *worker, WorkerState *state);
