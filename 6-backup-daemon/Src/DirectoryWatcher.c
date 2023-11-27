#include "DirectoryWatcher.h"

#define ENTITY_NAME "DirectoryWatcher"

TnStatus DirectoryWatcherInit(DirectoryWatcher* self, DirWatcherId id,
                              Logger* logger, DWErrorCallback callback) {
  if (!self || !logger || !callback.ErrorHandler)
    return TNSTATUS(TN_BAD_ARG_PTR);

  self->Callback = callback;
  self->Logger = logger;
  int ret;
  TnStatus status;

  if ((ret = pthread_mutex_init(&self->Mutex, NULL)) != 0) {
    errno = ret;
    return TNSTATUS(TN_ERRNO);
  }

  if ((ret = pthread_cond_init(&self->Cond, NULL)) != 0) {
    errno = ret;
    ret = pthread_mutex_destroy(&self->Mutex);
    return TNSTATUS(TN_ERRNO);
  }

  int iNotifyFd = inotify_init();
  if (iNotifyFd < 0) {
    pthread_mutex_destroy(&self->Mutex);
    pthread_cond_destroy(&self->Cond);
    return TNSTATUS(TN_ERRNO);
  }

  status = StageQueueInit(&self->StageQueue, logger);
  if (!TnStatusOk(status)) {
    pthread_mutex_destroy(&self->Mutex);
    pthread_cond_destroy(&self->Cond);
    close(iNotifyFd);
    return status;
  }

  status = WatchDescriptorMapInit(&self->WDMap, DIR_WATCHER_N_BUCKETS, logger);
  if (!TnStatusOk(status)) {
    pthread_mutex_destroy(&self->Mutex);
    pthread_cond_destroy(&self->Cond);
    StageQueueDestroy(&self->StageQueue);
    close(iNotifyFd);
    return status;
  }

  self->Path = NULL;
  self->INotifyFd = iNotifyFd;
  self->Id = id;
  self->DoStop = 0;
  return TN_OK;
}

TnStatus DirectoryWatcherStart(DirectoryWatcher* self, const char* path) {
  if (!self || !path) return TNSTATUS(TN_BAD_ARG_PTR);
  TnStatus status;

  const char* newPath = strdup(path);
  if (!newPath) return TNSTATUS(TN_BAD_ALLOC);

  status = DirectoryWatcherRegisterTree(self, newPath, strlen(newPath));
  if (!TnStatusOk(status)) return status;

  self->Path = newPath;

  int ret = pthread_create(&self->Thread, NULL, DirectoryWatcherMainLoop, self);
  if (ret != 0) {
    errno = ret;
    return TNSTATUS(TN_ERRNO);
  }

  return TN_OK;
}

TnStatus DirectoryWatcherStop(DirectoryWatcher* self) {
  if (!self) return TNSTATUS(TN_ERRNO);

  int ret = pthread_join(self->Thread, NULL);
  if (ret != 0) {
    errno = ret;
    return TNSTATUS(TN_ERRNO);
  }

  TnStatus status = DirectoryWatcherUnregisterAllDirectories(self);

  errno = self->Errno;
  return self->Status;
}

TnStatus DirectoryWatcherDestroy(DirectoryWatcher* self) {
  if (!self) return TNSTATUS(TN_BAD_ARG_PTR);

  TnStatus status;
  int ret;

  close(self->INotifyFd);
  StageQueueDestroy(&self->StageQueue);
  WatchDescriptorMapDestroy(&self->WDMap);

  pthread_mutex_destroy(&self->Mutex);
  pthread_cond_destroy(&self->Cond);

  return TN_OK;
}

static void DirectoryWatcherLock(DirectoryWatcher* self) {
  assert(self);
  pthread_mutex_lock(&self->Mutex);
}

static void DirectoryWatcherUnlock(DirectoryWatcher* self) {
  assert(self);
  pthread_mutex_unlock(&self->Mutex);
}

static void DirectoryWatcherSleep(DirectoryWatcher* self) {
  assert(self);
  pthread_cond_wait(&self->Cond, &self->Mutex);
}

static void DirectoryWatcherSignal(DirectoryWatcher* self) {
  assert(self);
  pthread_cond_signal(&self->Cond);
}

static TnStatus ReadFromFd(int fd, char* buf, size_t size) {
  size_t total = 0;

  while (total != size) {
    int ret = read(fd, buf + total, size - total);
    if (ret <= 0) return TNSTATUS(TN_ERRNO);

    total += ret;
  }

  return TN_OK;
}

static void* DirectoryWatcherFinish(DirectoryWatcher* self, TnStatus status) {
  assert(self);
  self->Errno = errno;
  self->Status = status;

  assert(self->Callback.ErrorHandler);
  self->Callback.ErrorHandler(self, self->Callback.Arg);

  return NULL;
}

static void* DirectoryWatcherMainLoop(void* selfPtr) {
  assert(selfPtr);
  DirectoryWatcher* self = (DirectoryWatcher*)selfPtr;
  TnStatus status;

  char buffer[sizeof(struct inotify_event) + NAME_MAX + 1];
  const struct inotify_event* event = (const struct inotify_event*)buffer;

  while (1) {
    DirectoryWatcherLock(self);
    if (self->DoStop) {
      DirectoryWatcherUnlock(self);
      status = TN_OK;
      return DirectoryWatcherFinish(self, status);
    }
    DirectoryWatcherUnlock(self);

    status = ReadFromFd(self->INotifyFd, buffer, sizeof(buffer));
    if (!TnStatusOk(status)) return DirectoryWatcherFinish(self, status);

    status = DirectoryWatcherDispatchEvent(self, event);
    if (!TnStatusOk(status)) return DirectoryWatcherFinish(self, status);
  }
}

static void DirectoryWatcherFatalErrorHandler(DirectoryWatcher* self) {
  assert(self);
  TnStatus status;

  DirectoryWatcherUnregisterAllDirectories(self);
}

static TnStatus DirectoryWatcherHandleFileEvent(
    DirectoryWatcher* self, const struct inotify_event* event,
    StageType sType) {
  assert(self);
  assert(event);
  assert(!(event->mask & IN_ISDIR));
  TnStatus status;

  const char* path;
  size_t pathSize;

  status = DirectoryWatcherGetPathOfEvent(self, event, &path, &pathSize);
  if (TnStatusOk(status)) return status;

  status =
      DirectoryWatcherEmplaceStage(self, STAGE_FILE, sType, path, pathSize);
  if (!TnStatusOk(status)) return status;

  switch (sType) {
    case STAGE_CREATED:
      LOG_INFO("File %s created", event->name);
      break;
    case STAGE_MODIFIED:
      LOG_INFO("File %s modified", event->name);
      break;
    case STAGE_DELETED:
      LOG_INFO("File %s deleted", event->name);
      break;
    default:
      assert(0);
  }

  return TN_OK;
}

static TnStatus DirectoryWatcherHandleDirectoryCreatedEvent(
    DirectoryWatcher* self, const struct inotify_event* event) {
  assert(self);
  assert(event);
  assert(event->mask & IN_ISDIR & IN_CREATE);
  TnStatus status;

  const char* path;
  size_t pathSize;
  Stage stage;

  status = DirectoryWatcherGetPathOfEvent(self, event, &path, &pathSize);
  if (TnStatusOk(status)) return status;

  status = DirectoryWatcherRegisterTree(self, path, pathSize);
  if (!TnStatusOk(status)) return status;

  status = DirectoryWatcherEmplaceStage(self, STAGE_DIR, STAGE_CREATED, path,
                                        pathSize);
  if (!TnStatusOk(status)) return status;

  LOG_INFO("Registered new directory: %s", path);

  return TN_OK;
}

static TnStatus DirectoryWatcherHandleDirectoryDeletedEvent(
    DirectoryWatcher* self, const struct inotify_event* event) {
  assert(self);
  assert(event);
  assert(event->mask & IN_ISDIR & IN_DELETE);
  TnStatus status;

  const char* path;
  size_t pathSize;
  Stage stage;

  status = DirectoryWatcherGetPathOfEvent(self, event, &path, &pathSize);
  if (TnStatusOk(status)) {
    LOG_ERROR("Failed to get path of event on %s", event->name);
    return status;
  }

  status = DirectoryWatcherEmplaceStage(self, STAGE_DIR, STAGE_DELETED, path,
                                        pathSize);
  if (!TnStatusOk(status)) return status;

  status = DirectoryWatcherUnregisterDirectory(self, event->wd);
  if (!TnStatusOk(status)) return status;

  LOG_INFO("Unregistered directory %s", path);

  return TN_OK;
}

static TnStatus DirectoryWatcherDispatchEvent(
    DirectoryWatcher* self, const struct inotify_event* event) {
  assert(self);
  assert(event);
  TnStatus status;

  if (event->mask & IN_ISDIR) {
    if (event->mask & IN_CREATE)
      return DirectoryWatcherHandleDirectoryCreatedEvent(self, event);
    if (event->mask & IN_DELETE)
      return DirectoryWatcherHandleDirectoryDeletedEvent(self, event);
  } else {
    if (event->mask & IN_CREATE)
      return DirectoryWatcherHandleFileEvent(self, event, STAGE_CREATED);
    if (event->mask & IN_MODIFY)
      return DirectoryWatcherHandleFileEvent(self, event, STAGE_MODIFIED);
    if (event->mask & IN_DELETE)
      return DirectoryWatcherHandleFileEvent(self, event, STAGE_DELETED);
  }
  return TN_OK;
}

static TnStatus DirectoryWatcherAddStage(DirectoryWatcher* self, Stage* stage) {
  if (!self || !stage) return TNSTATUS(TN_BAD_ARG_PTR);
  TnStatus status = TN_OK;

  DirectoryWatcherLock(self);
  status = StageQueuePush(&self->StageQueue, stage);

  if (TnStatusOk(status)) DirectoryWatcherSignal(self);

  DirectoryWatcherUnlock(self);

  return status;
}

static TnStatus DirectoryWatcherEmplaceStage(DirectoryWatcher* self,
                                             FileType fileType,
                                             StageType stageType,
                                             const char* path,
                                             size_t pathSize) {
  if (!self || !path) return TNSTATUS(TN_BAD_ARG_PTR);

  Stage stage;
  TnStatus status;

  status = CreateStage(fileType, stageType, path, pathSize, &stage);
  if (!TnStatusOk(status)) return status;

  status = DirectoryWatcherAddStage(self, &stage);
  if (!TnStatusOk(status)) return status;

  return TN_OK;
}

static TnStatus DirectoryWatcherGetPathOfEvent(DirectoryWatcher* self,
                                               const struct inotify_event* event,
                                               const char** newPathPtr,
                                               size_t* newPathSizePtr) {
  if (!self || !event) return TNSTATUS(TN_BAD_ARG_PTR);
  TnStatus status;
  WDListNode* node;

  status = WatchDescriptorMapFind(&self->WDMap, event->wd, &node);

  if (!TnStatusOk(status)) return status;
  if (!node) return TNSTATUS(TN_BAD_ARG_VAL);

  const char* path = node->Path;
  size_t pathSize = strlen(path) + 1;
  const char* name = event->name;
  size_t nameSize = strlen(name) + 1;
  const char* newPath;
  size_t newPathSize;
  status =
      ConcatenatePath(path, pathSize, name, nameSize, &newPath, &newPathSize);

  if (!TnStatusOk(status)) return status;

  *newPathPtr = newPath;
  *newPathSizePtr = newPathSize;

  return TN_OK;
}

static TnStatus DirectoryWatcherRegisterDirectory(DirectoryWatcher* self,
                                                  const char* path,
                                                  int* newWd) {
  if (!self || !path || !newWd) return TNSTATUS(TN_BAD_ARG_PTR);

  uint32_t mask = IN_CREATE | IN_MODIFY | IN_DELETE;
  int wd = inotify_add_watch(self->INotifyFd, path, mask);
  if (wd < 0) return TNSTATUS(TN_ERRNO);

  TnStatus status = WatchDescriptorMapAdd(&self->WDMap, wd, path);
  if (!TnStatusOk(status)) {
    inotify_rm_watch(self->INotifyFd, wd);
    return status;
  }

  *newWd = wd;
  return TN_OK;
}

static TnStatus DirectoryWatcherUnregisterDirectory(DirectoryWatcher* self,
                                                    int wd) {
  if (!self) return TNSTATUS(TN_BAD_ARG_PTR);
  TnStatus status;
  WDListNode* node;

  status = WatchDescriptorMapFind(&self->WDMap, wd, &node);
  if (!TnStatusOk(status)) return status;

  if (!node) return TNSTATUS(TN_BAD_ARG_VAL);

  return DirectoryWatcherUnregisterDirectoryNode(self, node);
}

static TnStatus DirectoryWatcherUnregisterDirectoryNode(DirectoryWatcher* self,
                                                        WDListNode* node) {
  if (!self || !node) return TNSTATUS(TN_BAD_ARG_PTR);
  TnStatus status;
  int ret;

  const char* path = node->Path;
  int wd = node->Wd;

  status = WatchDescriptorMapRemove(&self->WDMap, wd);
  if (!TnStatusOk(status)) return status;

  free((char*)path);
  inotify_rm_watch(self->INotifyFd, wd);

  return status;
}

static TnStatus DirectoryWatcherUnregisterAllDirectories(
    DirectoryWatcher* self) {
  if (!self) return TNSTATUS(TN_BAD_ARG_PTR);
  TnStatus status;

  WDMapIterator iter;
  status = WatchDescriptorMapGetIterator(&self->WDMap, &iter);

  if (!TnStatusOk(status)) return status;

  WDListNode* node;

  while (1) {
    status = WDMapIteratorGetNext(&iter, &node);
    if (!TnStatusOk(status)) break;
    if (!node) break;
    DirectoryWatcherUnregisterDirectoryNode(self, node);
  }

  return status;
}

// Sizes should include '\0' character
static TnStatus ConcatenatePath(const char* path, size_t pathSize,
                                const char* name, size_t nameSize,
                                const char** newPathPtr, size_t* newPathSizePtr) {
  if (!path || !name || !newPathPtr || !newPathSizePtr)
    return TNSTATUS(TN_BAD_ARG_PTR);

  size_t newPathSize = nameSize + pathSize;
  char* newPath = (char*)malloc(newPathSize);

  if (!newPath) return TNSTATUS(TN_BAD_ALLOC);

  memcpy(newPath, path, pathSize);
  newPath[pathSize - 1] = '/';
  memcpy(newPath + pathSize, name, nameSize);
  assert(newPath[newPathSize - 1] == '\0');

  *newPathPtr = newPath;
  *newPathSizePtr = newPathSize;

  return TN_OK;
}

// pathSize includes '\0' character
static TnStatus DirectoryWatcherRegisterTree_Recursive(DirectoryWatcher* self,
                                                       const char* path,
                                                       size_t pathSize) {
  if (!self || !path) return TNSTATUS(TN_BAD_ARG_PTR);
  TnStatus status;

  DIR* directory = opendir(path);
  if (!directory) return TNSTATUS(TN_ERRNO);

  int newWd;
  status = DirectoryWatcherRegisterDirectory(self, path, &newWd);
  if (!TnStatusOk(status)) {
    closedir(directory);
    return status;
  }

  struct dirent* curDir = NULL;
  while ((curDir = readdir(directory)) != NULL) {
    if (curDir->d_type = DT_REG) continue;

    if (curDir->d_type != DT_DIR) {
      LOG_WARN("File type %d is not supported", curDir->d_type);
      continue;
    }

    const char* name = curDir->d_name;
    size_t nameSize = strlen(name) + 1;
    const char* newPath;
    size_t newPathSize;

    status =
        ConcatenatePath(path, pathSize, name, nameSize, &newPath, &newPathSize);
    if (!TnStatusOk(status)) {
      closedir(directory);
      break;
    }

    status = DirectoryWatcherRegisterTree_Recursive(self, newPath, newPathSize);
    if (!TnStatusOk(status)) {
      closedir(directory);
      break;
    }
  }

  return status;
}

static TnStatus DirectoryWatcherRegisterTree(DirectoryWatcher* self,
                                             const char* path,
                                             size_t pathSize) {
  if (!self || !path) return TNSTATUS(TN_BAD_ARG_PTR);
  TnStatus status;

  return DirectoryWatcherRegisterTree_Recursive(self, path, pathSize);
}

TnStatus DirectoryWatcherGetStage(DirectoryWatcher* self, Stage* ret) {
  if (!self || !ret) return TNSTATUS(TN_BAD_ARG_PTR);
  TnStatus status;

  DirectoryWatcherLock(self);
  while(self->StageQueue.Size == 0)
    DirectoryWatcherSleep(self);

  status = StageQueuePop(&self->StageQueue, ret);
  DirectoryWatcherUnlock(self);
  
  return status;
}

#undef ENTITY_NAME