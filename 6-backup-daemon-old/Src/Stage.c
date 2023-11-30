#include "Stage.h"

TnStatus CreateStage(FileType fileType, StageType stageType, const char* path, size_t pathSize, Stage* newStage) {
  if (!path || !newStage) return TNSTATUS(TN_BAD_ARG_PTR);

  const char* newPath = strdup(path);
  if (!newPath) return TNSTATUS(TN_BAD_ALLOC);

  newStage->FileType = fileType;
  newStage->StageType = stageType;
  newStage->Path = newPath;
  newStage->PathSize = pathSize;

  return TN_OK;
}