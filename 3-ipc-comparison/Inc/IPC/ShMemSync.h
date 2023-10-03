#pragma once

#include <stdlib.h>

#define SHM_SYNC_READING 0
#define SHM_SYNC_WRITING 1
#define SHM_SYNC_FINISH 2

#ifdef __cplusplus
extern "C" {
#endif

char GetShmState(const char* ptr);
void SetShmState(char* ptr, char state);
size_t GetShmSize(const char* ptr);
void SetShmSize(char* ptr, size_t size);
char* GetShmBuf(char* ptr);
size_t GetShmCapacity(size_t size);

#ifdef __cplusplus
}
#endif