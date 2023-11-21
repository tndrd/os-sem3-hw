#pragma once
#include <dirent.h>
#include <linux/limits.h>
#include <pthread.h>
#include <sys/inotify.h>
#include <sys/types.h>

#include "Logger.h"
#include "StageQueue.h"
#include "WatchDescriptorMap.h"

#define DIR_WATCHER_N_BUCKETS 256

typedef size_t DirWatcherId;
struct DirectoryWatcherImpl;

typedef struct {
  void (*ErrorHandler)(struct DirectoryWatcherImpl*, void*);
  void* Arg;
} DWErrorCallback;

typedef struct {
  DirWatcherId Id;
  Logger* Logger;

  StageQueue StageQueue;
  const char* Path;

  pthread_t Thread;
  pthread_cond_t Cond;
  pthread_mutex_t Mutex;

  DWErrorCallback Callback;

  int INotifyFd;
  WatchDescriptorMap WDMap;

} DirectoryWatcher;

TnStatus DirectoryWatcherInit(DirectoryWatcher* self, DirWatcherId id,
                              const char* path, Logger* logger,
                              DWErrorCallback callback);
TnStatus DirectoryWatcherHasStage(DirectoryWatcher* self, int* ret);
TnStatus DirectoryWatcherGetStage(DirectoryWatcher* self, Stage* ret);

static void* DirectoryWatcherMainLoop(void* selfPtr);

static TnStatus DirectoryWatcherLock(DirectoryWatcher* self);
static TnStatus DirectoryWatcherUnlock(DirectoryWatcher* self);
static TnStatus DirectoryWatcherSleep(DirectoryWatcher* self);
static TnStatus DirectoryWatcherSignal(DirectoryWatcher* self);
static TnStatus DirectoryWatcherAddDirectory(DirectoryWatcher* self,
                                        const char* path);