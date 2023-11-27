#pragma once
#include "Logger.h"
#include "WatchDescriptorList.h"

typedef struct {
  Logger* Logger;
  WatchDescriptorList* Buckets;
  size_t NBuckets;

  size_t Size;
} WatchDescriptorMap;

typedef struct {
  WatchDescriptorMap* Map;
  size_t Bucket;
  size_t ListN;
  WDListNode* Node;
} WDMapIterator;

TnStatus
WatchDescriptorMapInit(WatchDescriptorMap* self, size_t nBuckets,
                       Logger* logger);
TnStatus WatchDescriptorMapDestroy(WatchDescriptorMap* self);
TnStatus WatchDescriptorMapFind(WatchDescriptorMap* self, int targetWd,
                                WDListNode** ret);
TnStatus WatchDescriptorMapAdd(WatchDescriptorMap* self, int newWd,
                               const char* path);
TnStatus WatchDescriptorMapRemove(WatchDescriptorMap* self, int targetWd);
TnStatus WatchDescriptorMapGetIterator(WatchDescriptorMap* self, WDMapIterator* iter);
TnStatus WDMapIteratorGetNext(WDMapIterator* iter, WDListNode** node);

static int WDMapHash(int wd);
static WatchDescriptorList* WDMapGetBucket(WatchDescriptorMap* self, int wd);
static int WDMapHas(WatchDescriptorMap* self, int wd);