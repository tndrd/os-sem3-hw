#pragma once
#include "WatchDescriptorList.h"
#include "Logger.h"

typedef struct {
  Logger* Logger;
  WatchDescriptorList* Buckets;
  size_t NBuckets;

  size_t Size;

} WatchDescriptorMap;

TnStatus WatchDescriptorMapInit(WatchDescriptorMap* self, size_t nBuckets, Logger* logger);
TnStatus WatchDescriptorMapDestroy(WatchDescriptorMap* self);
TnStatus WatchDescriptorMapFind(WatchDescriptorMap* self, int targetWd, WDListNode** ret);
TnStatus WatchDescriptorMapAdd(WatchDescriptorMap* self, int newWd, const char* path);
TnStatus WatchDescriptorMapRemove(WatchDescriptorMap* self, int targetWd);

static int WDMapHash(int wd);
static WatchDescriptorList* WDMapGetBucket(WatchDescriptorMap* self, int wd);
static int WDMapHas(WatchDescriptorMap* self, int wd);