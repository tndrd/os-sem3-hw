#pragma once
#include <assert.h>

#include "TnStatus.h"

typedef struct WDListNodeImpl {
  int Wd;
  const char* Path;

  struct WDListNodeImpl* Next;
  struct WDListNodeImpl* Prev;
} WDListNode;

typedef struct {
  WDListNode DummyTail;
  WDListNode* Head;
  size_t Size;
} WatchDescriptorList;

TnStatus WatchDescriptorListInit(WatchDescriptorList* self);
TnStatus WatchDescriptorListDestroy(WatchDescriptorList* self);
TnStatus WatchDescriptorListAdd(WatchDescriptorList* self, int newWd,
                                const char* path);
TnStatus WatchDescriptorListRemove(WatchDescriptorList* self, WDListNode* node);
TnStatus WatchDescriptorListFind(WatchDescriptorList* self, int targetWd,
                                 WDListNode** ret);