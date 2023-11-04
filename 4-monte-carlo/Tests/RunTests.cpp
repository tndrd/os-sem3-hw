#include <vector>

#include "Worker.h"
#include "gtest/gtest.h"

#define CALL(foo) ASSERT_EQ(foo, STATUS_SUCCESS);

const int DefaultArg = 666;

void Callback(Worker* worker, void* arg) {
  ASSERT_NE(worker, nullptr);
  ASSERT_NE(arg, nullptr);
  ASSERT_EQ(*((int*)(arg)), DefaultArg);
}

void DummyCallbackFoo(Worker* worker, void* arg) {}

TEST(Worker, Init) {
  Worker worker;
  CALL(WorkerInit(&worker, 0));

  int arg = DefaultArg;
  WorkerCallbackT callback;
  callback.Args = &arg;
  callback.Function = Callback;

  CALL(WorkerRun(&worker, callback));
  CALL(WorkerStop(&worker));
  CALL(WorkerDestroy(&worker));
}

TEST(Worker, Callback) {
  Worker worker;
  CALL(WorkerInit(&worker, 0));

  int arg = DefaultArg;
  WorkerCallbackT callback;
  callback.Args = &arg;
  callback.Function = Callback;

  CALL(WorkerRun(&worker, callback));
  sleep(1);
  CALL(WorkerStop(&worker));
  CALL(WorkerDestroy(&worker));
}

TnStatus Pow(const void* args, void* res) {
  int* val = (int*)args;
  int* result = (int*)res;

  *result = *val * *val;

  return STATUS_SUCCESS;
}

void TaskCompleteCallback(Worker* worker, void* args) { *(int*)args = 1; }

TEST(Worker, SyncTaskChain) {
  Worker worker;
  CALL(WorkerInit(&worker, 0));

  int arg, res, done;
  WorkerCallbackT callback;
  callback.Args = &done;
  callback.Function = TaskCompleteCallback;

  WorkerTask task;
  TnStatus status;
  task.Args = &arg;
  task.Result = &res;
  task.Status = &status;
  task.Function = Pow;

  CALL(WorkerRun(&worker, callback));

  int nRepeats = 5;
  for (int i = 0; i < nRepeats; ++i) {
    while (worker.State != WORKER_FREE)
      ;

    arg = i;
    res = 0;
    done = 0;

    CALL(WorkerAssignTask(&worker, task));
    while (worker.State != WORKER_DONE)
      ;

    ASSERT_EQ(done, 1);
    ASSERT_EQ(res, i * i);

    CALL(WorkerFinish(&worker));
  }

  CALL(WorkerStop(&worker));
  CALL(WorkerDestroy(&worker));
}

#define N_REPEATS 5

const int NRepeats = N_REPEATS;
int TestBuf[N_REPEATS];
int DoStop = 0;

void WorkerChainTaskCallback(Worker* worker, void* args) {
  static int i = 0;
  WorkerTask* task = (WorkerTask*)(args);

  if (worker->State == WORKER_FREE) {
    int* arg = (int*)(task->Args);
    *arg = i;
    WorkerAssignTask(worker, *task);
  }

  if (worker->State == WORKER_DONE) {
    int* result = (int*)(task->Result);
    TestBuf[i] = *result;
    WorkerFinish(worker);

    ASSERT_EQ(*result, i * i);
    i++;
    if (i == NRepeats) DoStop = 1;
  }
}

TEST(Worker, AsyncTaskChain) {
  Worker worker;
  CALL(WorkerInit(&worker, 0));

  WorkerTask task;
  int arg;
  int result;
  TnStatus status;

  task.Args = &arg;
  task.Result = &result;
  task.Status = &status;
  task.Function = Pow;

  WorkerCallbackT callback;
  callback.Args = &task;
  callback.Function = WorkerChainTaskCallback;

  CALL(WorkerRun(&worker, callback));
  while (!DoStop)
    ;
  CALL(WorkerStop(&worker));
  CALL(WorkerDestroy(&worker));

  for (int i = 0; i < NRepeats; ++i) {
    ASSERT_EQ(TestBuf[i], i * i);
  }
}