#pragma once
#include "DirectoryBackuper.h"
#include "Logger.h"

typedef size_t DBackuperId; 

typedef struct {
  DirectoryBackuper Backuper;
  int Present;
  DBackuperId Id;

  ListEntry* Prev;
  ListEntry* Next;

} ListEntry;

typedef struct {
  Logger* logger;
  ListEntry* Tail;

  // Etc
} DBackuperAllocator;

typedef struct {
  DirectoryBackuper* Current;
  DBackuperId Id;
} DBackuperAllocatorIterator;

TnStatus DBackuperAllocatorInit(DBackuperAllocator* self, Logger* logger);
TnStatus DBackuperAllocatorAllocate(DBackuperAllocator* self, DBackuperId* newId);
TnStatus DBackuperAllocatorRemove(DBackuperAllocator* self, DBackuperId id);
TnStatus DBackuperAllocatorGet(DBackuperAllocator* self, DBackuperId id, const DirectoryBackuper** res);
TnStatus DBackuperAllocatorDestroy(DBackuperAllocator* self);
TnStatus DBackuperAllocatorGetIter(DBackuperAllocator* self, DBackuperAllocatorIterator* newIter);