#include "DirectoryWatcher.h"

#define ENTITY_NAME "DirectoryWatcher"

TnStatus DirectoryWatcherInit(DirectoryWatcher* self, DirWatcherId id,
                              const char* path, Logger* logger,
                              DWErrorCallback callback) {
  if (!self || !path || !logger || !callback.Arg || !callback.ErrorHandler)
    return TNSTATUS(TN_BAD_ARG_PTR);

  self->Callback = callback;
  self->Logger = logger;
  int ret;
  TnStatus status;

  if ((ret = pthread_mutex_init(&self->Mutex, NULL)) != 0) {
    LOG_ERROR("pthread_mutex_init(): %s", strerror(ret));
    errno = ret;
    return TNSTATUS(TN_ERRNO);
  }

  if ((ret = pthread_cond_init(&self->Cond, NULL)) != 0) {
    LOG_ERROR("pthread_cond_init(): %s", strerror(ret));
    errno = ret;

    ret = pthread_mutex_destroy(&self->Mutex);
    if (ret != 0) LOG_WARN("pthread_mutex_destroy(): %s", strerror(ret));

    return TNSTATUS(TN_ERRNO);
  }

  int iNotifyFd = inotify_init();
  if (iNotifyFd < 0) {
    LOG_ERROR("inotify_init(): %s", strerror(errno));

    ret = pthread_mutex_destroy(&self->Mutex);
    if (ret != 0) LOG_WARN("pthread_mutex_destroy(): %s", strerror(ret));

    ret = pthread_cond_destroy(&self->Cond);
    if (ret != 0) LOG_WARN("pthread_cond_destroy(): %s", strerror(ret));

    return TNSTATUS(TN_ERRNO);
  }

  status = StageQueueInit(&self->StageQueue, logger);
  if (!TnStatusOk(status)) {
    LOG_ERROR("StageQueueInit(): %s", TnStatusCodeGetDescription(status.Code));

    ret = pthread_mutex_destroy(&self->Mutex);
    if (ret != 0) LOG_WARN("pthread_mutex_destroy(): %s", strerror(ret));

    ret = pthread_cond_destroy(&self->Cond);
    if (ret != 0) LOG_WARN("pthread_cond_destroy(): %s", strerror(ret));

    close(iNotifyFd);
    return status;
  }

  status = WatchDescriptorMapInit(&self->WDMap, DIR_WATCHER_N_BUCKETS, logger);
  if (!TnStatusOk(status)) {
    LOG_ERROR("WatchDescriptorMapInit(): %s",
              TnStatusCodeGetDescription(status.Code));

    ret = pthread_mutex_destroy(&self->Mutex);
    if (ret != 0) LOG_WARN("pthread_mutex_destroy(): %s", strerror(ret));

    ret = pthread_cond_destroy(&self->Cond);
    if (ret != 0) LOG_WARN("pthread_cond_destroy(): %s", strerror(ret));

    StageQueueDestroy(&self->StageQueue);
    close(iNotifyFd);
    return status;
  }

  self->Path = path;
  self->INotifyFd = iNotifyFd;
  self->Id = id;

  LOG_INFO("Initialized", "");
}

TnStatus DirectoryWatcherDestroy(DirectoryWatcher* self) {
  if (!self) return TNSTATUS(TN_BAD_ARG_PTR);

  TnStatus status;
  int ret;

  if (close(self->INotifyFd) != 0) LOG_WARN("close(): %s", strerror(errno));

  ret = pthread_join(self->Thread, NULL);
  if (ret != 0) LOG_WARN("pthread_join(): %s", strerror(ret));

  StageQueueDestroy(&self->StageQueue);
  WatchDescriptorMapDestroy(&self->WDMap);

  ret = pthread_mutex_destroy(&self->Mutex);
  if (ret != 0) LOG_WARN("pthread_mutex_destroy(): %s", strerror(ret));

  ret = pthread_cond_destroy(&self->Cond);
  if (ret != 0) LOG_WARN("pthread_cond_destroy(): %s", strerror(ret));

  LOG_INFO("Destroyed", "");
  return TN_OK;
}

static TnStatus DirectoryWatcherLock(DirectoryWatcher* self) {
  assert(self);
  int ret = pthread_mutex_lock(&self->Mutex);
  if (ret == 0) return TN_OK;

  LOG_ERROR("pthread_mutex_lock(): %s", strerror(ret));
  return TNSTATUS(TN_ERRNO);
}

static TnStatus DirectoryWatcherUnlock(DirectoryWatcher* self) {
  assert(self);
  int ret = pthread_mutex_unlock(&self->Mutex);
  if (ret == 0) return TN_OK;

  LOG_ERROR("pthread_mutex_unlock(): %s", strerror(ret));
  return TNSTATUS(TN_ERRNO);
}

static TnStatus DirectoryWatcherSleep(DirectoryWatcher* self) {
  assert(self);
  int ret = pthread_cond_wait(&self->Cond, &self->Mutex);
  if (ret == 0) return TN_OK;

  LOG_ERROR("pthread_cond_wait(): %s", strerror(ret));
  return TNSTATUS(TN_ERRNO);
}

static TnStatus DirectoryWatcherSignal(DirectoryWatcher* self) {
  assert(self);
  int ret = pthread_cond_signal(&self->Cond);
  if (ret == 0) return TN_OK;

  LOG_ERROR("pthread_cond_signal(): %s", strerror(ret));
  return TNSTATUS(TN_ERRNO);
}

static void* DirectoryWatcherMainLoop(void* selfPtr) {
  assert(selfPtr);
  DirectoryWatcher* self = (DirectoryWatcher*)selfPtr;

#define BUFSIZE

  char Buffer[sizeof(struct inotify_event) + NAME_MAX + 1];

  while (1) {
  }

#undef BUFSIZE
}

static TnStatus DirectoryWatcherAddDirectory(DirectoryWatcher* self,
                                             const char* path) {
  if (!self || !path) return TNSTATUS(TN_BAD_ARG_PTR);

  DIR* directory = opendir(path);

  if (!directory) {
    if (errno = ENOTDIR) {
      return TN_OK;
    } else
      return TNSTATUS(TN_ERRNO);
  }

  int wd = inotify_add_watch(self->INotifyFd, path, IN_CREATE | IN_MODIFY | IN_DELETE);
  if (wd < 0) {
    LOG_ERROR("inotify_add_watch(): %s", strerror(errno));
  }

  TnStatus status = WatchDescriptorMapAdd(&self->WDMap, wd, path);
  if (!TnIsOk(status)) {
    LOG_ERROR("WatchDescriptorMapAdd(): %s", TnStatusCodeGetDescription(status.Code));
    if (inotify_rm_watch(self->INotifyFd, wd) != 0)
      LOG_WARN("inotify_rm_watch(): %s", strerror(errno));
  }
}

#undef ENTITY_NAME