#pragma once
#include "TnStatus.h"

typedef struct {
  int Wd;
  const char* Path;

  WDListNode* Next;
  WDListNode* Prev;
} WDListNode;

typedef struct {
  WDListNode DummyTail;
  WDListNode* Head;
  size_t Size;
} WatchDescriptorList;

TnStatus WatchDescriptorListInit(WatchDescriptorList* self);
TnStatus WatchDescriptorListDestroy(WatchDescriptorList* self);
TnStatus WatchDescriptorListAdd(WatchDescriptorList* self, int newWd, const char* path);
TnStatus WatchDescriptorListRemove(WatchDescriptorList* self, WDListNode* node);
TnStatus WatchDescriptorListFind(WatchDescriptorList* self, int targetWd, WDListNode** ret);