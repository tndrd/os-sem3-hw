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

  if (!newBuckets) {
    LOG_ERROR("Failed to allocate memory for buckets", "");
    return TNSTATUS(TN_BAD_ALLOC);
  }

  int i = 0;
  for (; i < nBuckets; ++i) {
    status = WatchDescriptorListInit(&newBuckets[i]);
    if (!TnStatusOk(status)) {
      LOG_ERROR("Failed to initialize bucket #%d: ", i,
                TnStatusCodeGetDescription(status.Code));
      break;
    }
  }

  if (TnStatusOk(status)) {
    LOG_INFO("Initialized", "");
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
  TnStatus status = WatchDescriptorListAdd(list, newWd, path);

  if (TnStatusOk(status)) {
    LOG_INFO("Added directory %s", path);
  } else {
    LOG_ERROR("Failed to add directory %s", path);
  }

  return status;
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

  LOG_INFO("Removed directory %s", path);

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

#undef ENTITY_NAME