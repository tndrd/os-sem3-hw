#pragma once

#include "DBackuperList.h"

typedef struct {
  Logger* logger;
  DBackuperAllocator List;

} DBackuperManager;

TnStatus DBackuperManagerInit(DBackuperManager* self, Logger* logger);
TnStatus DBackuperManagerDestroy(DBackuperManager* self);
TnStatus DBackuperManagerAdd(DBackuperManager* self, const char* path, size_t periodS, DBackuperId* newId);
TnStatus DBackuperManagerRemove(DBackuperManager* self, DBackuperId id);
TnStatus DBackuperManagerGet(DBackuperManager* self, DBackuperId id);
TnStatus DBackuperManagerGetIter(DBackuperManager* self, DBackuperAllocatorIterator* newIter);