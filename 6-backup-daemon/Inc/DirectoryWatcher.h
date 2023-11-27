#pragma once

#define _GNU_SOURCE

#include <dirent.h>
#include <linux/limits.h>
#include <pthread.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <unistd.h>

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

typedef struct DirectoryWatcherImpl {
  DirWatcherId Id;
  Logger* Logger;

  StageQueue StageQueue;
  const char* Path;

  pthread_t Thread;
  pthread_cond_t Cond;
  pthread_mutex_t Mutex;

  int DoStop;

  DWErrorCallback Callback;

  int INotifyFd;
  WatchDescriptorMap WDMap;

  TnStatus Status;
  int Errno;
} DirectoryWatcher;

TnStatus DirectoryWatcherInit(DirectoryWatcher* self, DirWatcherId id,
                              Logger* logger, DWErrorCallback callback);
TnStatus DirectoryWatcherStart(DirectoryWatcher* self, const char* path);
TnStatus DirectoryWatcherStop(DirectoryWatcher* self);
TnStatus DirectoryWatcherDestroy(DirectoryWatcher* self);

TnStatus DirectoryWatcherGetStage(DirectoryWatcher* self, Stage* ret);

static void* DirectoryWatcherMainLoop(void* selfPtr);

static void DirectoryWatcherLock(DirectoryWatcher* self);
static void DirectoryWatcherUnlock(DirectoryWatcher* self);
static void DirectoryWatcherSleep(DirectoryWatcher* self);
static void DirectoryWatcherSignal(DirectoryWatcher* self);

static TnStatus ReadFromFd(int fd, char* buf, size_t size);

static void* DirectoryWatcherFinish(DirectoryWatcher* self, TnStatus status);

static void* DirectoryWatcherMainLoop(void* selfPtr);

static void DirectoryWatcherFatalErrorHandler(DirectoryWatcher* self);

static TnStatus DirectoryWatcherHandleFileEvent(
    DirectoryWatcher* self, const struct inotify_event* event, StageType sType);

static TnStatus DirectoryWatcherHandleDirectoryCreatedEvent(
    DirectoryWatcher* self, const struct inotify_event* event);

static TnStatus DirectoryWatcherHandleDirectoryDeletedEvent(
    DirectoryWatcher* self, const struct inotify_event* event);

static TnStatus DirectoryWatcherDispatchEvent(
    DirectoryWatcher* self, const struct inotify_event* event);

static TnStatus DirectoryWatcherAddStage(DirectoryWatcher* self, Stage* stage);

static TnStatus DirectoryWatcherEmplaceStage(DirectoryWatcher* self,
                                             FileType fileType,
                                             StageType stageType,
                                             const char* path, size_t pathSize);

static TnStatus DirectoryWatcherGetPathOfEvent(DirectoryWatcher* self,
                                               const struct inotify_event* event,
                                               const char** newPathPtr,
                                               size_t* newPathSizePtr);

static TnStatus DirectoryWatcherRegisterDirectory(DirectoryWatcher* self,
                                                  const char* path, int* newWd);

static TnStatus DirectoryWatcherUnregisterDirectory(DirectoryWatcher* self,
                                                    int wd);

static TnStatus DirectoryWatcherUnregisterDirectoryNode(DirectoryWatcher* self,
                                                        WDListNode* node);

static TnStatus DirectoryWatcherUnregisterAllDirectories(
    DirectoryWatcher* self);

// Sizes should include '\0' character
static TnStatus ConcatenatePath(const char* path, size_t pathSize,
                                const char* name, size_t nameSize,
                                const char** newPathPtr, size_t* newPathSizePtr);

// pathSize includes '\0' character
static TnStatus DirectoryWatcherRegisterTree_Recursive(DirectoryWatcher* self,
                                                       const char* path,
                                                       size_t pathSize);

static TnStatus DirectoryWatcherRegisterTree(DirectoryWatcher* self,
                                             const char* path, size_t pathSize);

TnStatus DirectoryWatcherGetStage(DirectoryWatcher* self, Stage* ret);