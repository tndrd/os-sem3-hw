#include "WatchDescriptorList.h"

TnStatus WatchDescriptorListInit(WatchDescriptorList* self) {
  if (!self) return TNSTATUS(TN_BAD_ARG_PTR);

  self->DummyTail.Next = NULL;
  self->DummyTail.Prev = NULL;
  self->DummyTail.Path = NULL;
  self->DummyTail.Wd = -1;
  self->Size = 0;
  self->Head = &self->DummyTail;
}

TnStatus WatchDescriptorListDestroy(WatchDescriptorList* self) {
  if (!self) return TNSTATUS(TN_BAD_ARG_PTR);

  WDListNode* current = self->DummyTail.Next;
  size_t counter = 0;
  
  while(current != NULL) {
    WDListNode* next = current->Next;
    free(current);
    current = next;
    counter++;
  }

  assert(counter == self->Size);

  return TN_OK;
}

TnStatus WatchDescriptorListAdd(WatchDescriptorList* self, int newWd, const char* newPath) {
  if (!self || !newPath) return TNSTATUS(TN_BAD_ARG_VAL);

  WDListNode* newNode = (WDListNode*)malloc(sizeof(WDListNode));

  if (!newNode) return TNSTATUS(TN_BAD_ALLOC);

  newNode->Wd = newWd;
  newNode->Path = newPath;

  newNode->Prev = self->Head;
  newNode->Next = NULL;

  self->Head->Next = newNode;
  self->Head = newNode;

  self->Size++;
  return TN_OK;
}

TnStatus WatchDescriptorListRemove(WatchDescriptorList* self,  WDListNode* node) {
  if (!self || !node) return TNSTATUS(TN_BAD_ARG_PTR);

  if (self->Size == 0) return TNSTATUS(TN_UNDERFLOW);

  node->Prev->Next = node->Next;
  
  if (node->Next) {
    node->Next->Prev = node->Prev;
  } else {
    self->Head = node->Prev;
  }

  free(node);
}

TnStatus WatchDescriptorListFind(WatchDescriptorList* self, int targetWd, WDListNode** ret) {
  if (!self || !ret) return TNSTATUS(TN_BAD_ARG_PTR);

  *ret = NULL;

  WDListNode* current = self->DummyTail.Next;
  while(current) {
    if (current->Wd == targetWd) {
      *ret = current;
      break;
    }

    current = current->Next;
  }

  return TN_OK;
}