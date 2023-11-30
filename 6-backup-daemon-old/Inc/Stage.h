#pragma once
#include <stdlib.h>
#include "TnStatus.h"

typedef enum {
  STAGE_CREATED,
  STAGE_MODIFIED,
  STAGE_DELETED
} StageType;

typedef enum {
  STAGE_FILE,
  STAGE_DIR
} FileType;

typedef struct {
  StageType StageType;
  FileType FileType;
  const char* Path;
  size_t PathSize;
} Stage;

TnStatus CreateStage(FileType fileType, StageType stageType, const char* path, size_t pathSize, Stage* newStage);