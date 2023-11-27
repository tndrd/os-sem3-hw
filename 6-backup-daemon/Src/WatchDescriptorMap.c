#include "WatchDescriptorMap.h"
#define ENTITY_NAME "WatchDescriptorMap"

TnStatus WatchDescriptorMapInit(WatchDescriptorMap* self, size_t nBuckets,
                                Logger* logger) {
  if (!self || !logger) return TNSTATUS(TN_BAD_ARG_PTR);
  if (nBuckets == 0) return TNSTATUS(TN_BAD_ARG_VAL);
  self->Logger = logger;

  TnStatus status;
  WatchDescriptorList* newBuckets =
      (WatchDescriptorList*)malloc(nBuckets * sizeof(WatchDescriptorList));

  if (!newBuckets) return TNSTATUS(TN_BAD_ALLOC);

  int i = 0;
  for (; i < nBuckets; ++i) {
    status = WatchDescriptorListInit(&newBuckets[i]);
    if (!TnStatusOk(status)) break;
  }

  if (TnStatusOk(status)) {
    self->Buckets = newBuckets;
    self->NBuckets = nBuckets;
    return TN_OK;
  }

  TnStatus oldStatus = status;

  for (i = i - 1; i >= 0; --i) {
    status = WatchDescriptorListDestroy(&newBuckets[i]);
    if (!TnStatusOk(status))
      LOG_WARN("Failed to destroy bucket #%d", i,
               TnStatusCodeGetDescription(status.Code));
  }

  return oldStatus;
}

TnStatus WatchDescriptorMapDestroy(WatchDescriptorMap* self) {
  if (!self) return TNSTATUS(TN_BAD_ARG_PTR);
  assert(self->Buckets);

  TnStatus status;
  TnStatus errStatus = TN_OK;

  for (int i = 0; i < self->NBuckets; ++i) {
    status = WatchDescriptorListDestroy(&self->Buckets[i]);
    if (!TnStatusOk(status)) {
      LOG_WARN("Failed to destroy bucket #%d", i,
               TnStatusCodeGetDescription(status.Code));
      errStatus = status;
    }
  }

  return errStatus;
}

TnStatus WatchDescriptorMapFind(WatchDescriptorMap* self, int targetWd,
                                WDListNode** ret) {
  if (!self || !ret) return TNSTATUS(TN_BAD_ARG_PTR);

  WatchDescriptorList* list = WDMapGetBucket(self, targetWd);
  return WatchDescriptorListFind(list, targetWd, ret);
}

TnStatus WatchDescriptorMapAdd(WatchDescriptorMap* self, int newWd,
                               const char* path) {
  if (!self || !path) return TNSTATUS(TN_BAD_ARG_PTR);
  TnStatus status;

  assert(!WDMapHas(self, newWd));  // TODO add new errorcode

  WatchDescriptorList* list = WDMapGetBucket(self, newWd);
  status = WatchDescriptorListAdd(list, newWd, path);
  if (TnStatusOk(status)) return status;
}

TnStatus WatchDescriptorMapRemove(WatchDescriptorMap* self, int targetWd) {
  if (!self) return TNSTATUS(TN_BAD_ARG_PTR);
  TnStatus status;
  assert(WDMapHas(self, targetWd));  // Todo add new errorcode

  WatchDescriptorList* list = WDMapGetBucket(self, targetWd);
  WDListNode* node;

  status = WatchDescriptorListFind(list, targetWd, &node);
  assert(TnStatusOk(status));
  assert(node);

  const char* path = node->Path;

  status = WatchDescriptorListRemove(list, node);
  assert(TnStatusOk(status));

  return TN_OK;
}

static int WDMapHash(int wd) { return wd; }

static WatchDescriptorList* WDMapGetBucket(WatchDescriptorMap* self, int wd) {
  assert(self);
  assert(self->Buckets);

  size_t nBucket = WDMapHash(wd) % self->NBuckets;
  WatchDescriptorList* list = &self->Buckets[nBucket];

  assert(list);
  return list;
}

static int WDMapHas(WatchDescriptorMap* self, int wd) {
  WDListNode* node;
  TnStatus status;
  status = WatchDescriptorMapFind(self, wd, &node);
  assert(TnStatusOk(status));
  return node != NULL;
}

TnStatus WatchDescriptorMapGetIterator(WatchDescriptorMap* self,
                                       WDMapIterator* iter) {
  if (!self || !iter) return TNSTATUS(TN_BAD_ARG_PTR);

  iter->Map = self;
  iter->Bucket = 0;
  iter->ListN = 0;
  iter->Node = NULL;
}

TnStatus WDMapIteratorGetNext(WDMapIterator* self, WDListNode** node) {
  if (!self || !node) return TNSTATUS(TN_BAD_ARG_PTR);

  while (self->Bucket < self->Map->NBuckets &&
         self->Map->Buckets[self->Bucket].Size == 0) {
    self->Bucket++;
  }

  if (self->Bucket == self->Map->NBuckets) *node = NULL;

  if (self->ListN == self->Map->Buckets[self->Bucket].Size) {
    self->Bucket++;
  }

  if (self->ListN == 0)
    self->Node = self->Map->Buckets[self->Bucket].DummyTail.Next;

  assert(self->Node);
  *node = self->Node;

  self->Node = self->Node->Next;
  self->ListN++;

  return TN_OK;
}

#undef ENTITY_NAME