#include "IPC/ShMemSync.h"

char GetShmState(const char* ptr) { return *ptr; }

void SetShmState(char* ptr, char state) { *ptr = state; }

size_t GetShmSize(const char* ptr) { return *((size_t*)(ptr + 1)); }

void SetShmSize(char* ptr, size_t size) { *((size_t*)(ptr + 1)) = size; }

char* GetShmBuf(char* ptr) { return ptr + 1 + sizeof(size_t); }

size_t GetShmCapacity(size_t size) { return size - 1 - sizeof(size_t); }