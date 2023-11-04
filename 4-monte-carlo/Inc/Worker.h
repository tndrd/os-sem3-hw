#pragma once
#include <pthread.h>

#include "Status.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>

struct WorkerImpl;
typedef size_t WorkerID;

typedef TnStatus (*WorkerFooT)(const void* args, void* result);
typedef void (*WorkerCallbackFooT)(struct WorkerImpl* worker, void* args);

typedef struct {
  void* Args;
  WorkerCallbackFooT Function;
} WorkerCallbackT;

typedef struct {
  WorkerFooT Function;
  const void* Args;
  void* Result;
  TnStatus* Status;
} WorkerTask;

typedef enum {
  WORKER_FREE,
  WORKER_BUSY,
  WORKER_DONE,
} WorkerState;

typedef struct WorkerImpl {
  WorkerID ID;
  pthread_t Thread;

  /* Sync */
  pthread_mutex_t Mutex;
  pthread_cond_t Cond;

  /* Const during thread execution */
  WorkerCallbackT Callback;

  /* Main-W, Thread-R */
  WorkerTask Task;
  int HasTask;
  int ResultBeenRead;
  int Active;

  /* Main-R, Thread-W */
  WorkerState State;

} Worker;

#ifdef __cplusplus
extern "C" {
#endif

TnStatus WorkerInit(Worker* worker, WorkerID id);
TnStatus WorkerRun(Worker* worker, WorkerCallbackT callback);
TnStatus WorkerAssignTask(Worker* worker, WorkerTask task);
TnStatus WorkerDestroy(Worker* worker);
TnStatus WorkerStop(Worker* worker);
TnStatus WorkerFinish(Worker* worker);
TnStatus WorkerGetState(const Worker* worker, WorkerState* state);

#ifdef __cplusplus
}
#endif

static void WorkerSleep(Worker* worker);
static void WorkerWakeUp(Worker* worker);

static void WorkerSleepUntil(Worker* worker, int* condition);
static void WorkerExecute(void* workerArg);
static void WorkerNotify(Worker* worker);
static void* WorkerLoop(void* workerPtr);
