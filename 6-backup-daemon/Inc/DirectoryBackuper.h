#pragma once

#include <pthread.h>

#include "DirectoryWatcher.h"
#include "BackupProducer.h"

typedef struct {
  Logger* logger;

  DirectoryWatcher Watcher;
  BackupProducer BProducer;

  pthread_t Thread;
  pthread_mutex_t Mutex;
  pthread_cond_t Cond;

  size_t PeriodS;
  int DoStop;
  int DoRun;
} DirectoryBackuper;

TnStatus DirectoryBackuperInit(DirectoryBackuper* self, const char* srcPath, const char* dstPath, size_t periodS, Logger* logger);
TnStatus DirectoryBackuperStop(DirectoryBackuper* self);
TnStatus DirectoryBackuperPause(DirectoryBackuper* self);
TnStatus DirectoryBackuperSetPeriod(DirectoryBackuper* self);

static void* DirectoryBackuperMainLoop(void* selfPtr);