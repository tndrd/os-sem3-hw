#pragma once
#include <stdlib.h>

typedef enum {
  STAGE_CREATED,
  STAGE_MODIFIED,
  STAGE_DELETED
} StageType;

typedef struct {
  StageType Type;
  const char* Data;
  size_t Size;
} Stage;