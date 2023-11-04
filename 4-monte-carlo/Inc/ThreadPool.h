#pragma once
#include "TQMonitor.h"
#include "WQMonitor.h"
#include "WorkerArray.h"

typedef struct {
  TQMonitor Tasks;
  WQMonitor FreeWorkers;
  WorkerArray Workers;
} ThreadPool;

static void WorkerCallback(Worker* worker, void* args);
TnStatus ThreadPoolInit(ThreadPool* tp, size_t nWorkers);
TnStatus ThreadPoolRun(ThreadPool* tp);
TnStatus ThreadPoolStop(ThreadPool* tp);
TnStatus ThreadPoolDestroy(ThreadPool* tp);
TnStatus WorkerPoolAddTask(ThreadPool* tp, WorkerTask task);
TnStatus WorkerPoolWaitAll(ThreadPool* tp);