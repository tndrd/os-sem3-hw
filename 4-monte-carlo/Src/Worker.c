#include "Worker.h"

static TnStatus WorkerValidatePrecondition(Worker *worker)
{
  TnStatus status;

  if (!worker)
    return TN_BAD_ARG_PTR;
  if (!worker->Target)
    return TN_WORKER_BAD_FOO;
  if ((status = WorkerArgsValidate(&worker->Args)) != TN_SUCCESS)
    return status;
}

static TnStatus WorkerValidatePostcondition(Worker *worker)
{
  TnStatus status;

  if (!worker)
    return TN_BAD_ARG_PTR;
  if ((status = WorkerResultValidate(&worker->Result)) != TN_SUCCESS)
    return status;
}

static void* WorkerLoop(void * ptr)
{
  Worker* worker = (Worker*)ptr;
  TnStatus status;
  assert(worker);

  while (worker->State != WORKER_STOP)
  {
    if (worker->State != WORKER_READY)
      continue;

    if ((status = WorkerValidatePrecondition(worker)) != TN_SUCCESS)
    {
      worker->State = WORKER_ERROR;
      worker->Error = status;
      continue;
    }

    worker->State = WORKER_BUSY;
    worker->Target(&worker->Args, &worker->Result);

    if ((status = WorkerValidatePostcondition(worker)) != TN_SUCCESS)
    {
      worker->State = WORKER_ERROR;
      worker->Error = status;
    }

    worker->State = WORKER_DONE;
  }
}

TnStatus WorkerInit(Worker *worker)
{
  if (!worker)
    return TN_BAD_ARG_PTR;

  worker->State = WORKER_INIT;
  worker->Target = NULL;

  WorkerArgsInit(&worker->Args);
  WorkerResultInit(&worker->Result);

  int err = pthread_create(&worker->Thread, NULL, &WorkerLoop, worker);
  if (err != 0)
  {
    worker->Errno = err;
    return TN_ERRNO_ERROR;
  }

  worker->Errno = 0;
  return TN_SUCCESS;
}

TnStatus WorkerSetTarget(Worker *worker, WorkerFoo foo)
{
  if (!worker || !foo)
    return TN_BAD_ARG_PTR;

  worker->Target = foo;

  return TN_SUCCESS;
}

TnStatus WorkerSetArgs(Worker *worker, WorkerArgs *args)
{
  if (!worker || !args)
    return TN_BAD_ARG_PTR;

  worker->Args = *args;

  return TN_SUCCESS;
}

TnStatus WorkerRun(Worker *worker)
{
  if (!worker)
    return TN_BAD_ARG_PTR;

  if (worker->State != WORKER_INIT)
    return TN_WORKER_NOT_READY;

  worker->State = WORKER_READY;

  return TN_SUCCESS;
}

TnStatus WorkerReadResult(Worker *worker, WorkerResult *result)
{
  if (!worker || !result)
    return TN_BAD_ARG_PTR;

  if (worker->State != WORKER_DONE)
    return TN_WORKER_NOT_READY;

  *result = worker->Result;
  worker->State = WORKER_INIT;

  return TN_SUCCESS;
}

TnStatus WorkerStop(Worker *worker)
{
  if (!worker)
    return TN_BAD_ARG_PTR;

  if (worker->State != WORKER_INIT)
    return TN_WORKER_NOT_READY;

  worker->State = WORKER_STOP;

  int err = pthread_join(worker->Thread, NULL);
  if (err != 0)
  {
    worker->Errno = err;
    return TN_ERRNO_ERROR;
  }

  return TN_SUCCESS;
}

const char *WorkerGetErrorDescription(TnStatus status, const Worker *worker)
{
  assert(worker);

  if (status == TN_ERRNO_ERROR)
    return strerror(worker->Errno);

  return TnStatusGetDescription(status);
}

TnStatus WorkerGetState(const Worker *worker, WorkerState *state)
{
  if (!worker || !state)
    return TN_BAD_ARG_PTR;

  *state = worker->State;

  return TN_SUCCESS;
}
