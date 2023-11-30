#include "DirectoryWatcher.h"

void ErrorHandlerImpl(DirectoryWatcher* self, void* arg) {
#define ENTITY_NAME "DirectoryWatcher"
  LOG_ERROR("ErrorHandler: TnStatus", "");
  TnStatusPrintDescription(self->Status);
#undef ENTITY_NAME
}

void ErrorHandler(DirectoryWatcher* self, void* arg) {
  return ErrorHandlerImpl(self, arg);
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Expected path as argument");
    exit(1);
  }

  const char* path = argv[1];

  Logger logger;
  LoggerInit(&logger, stderr, LOGGING_INFO);

  DirectoryWatcher dw;
  TnStatus status;

  DWErrorCallback callback;
  callback.Arg = NULL;
  callback.ErrorHandler = ErrorHandler;

  status = DirectoryWatcherInit(&dw, 0, &logger, callback);
  TnStatusAssert(status);

  status = DirectoryWatcherStart(&dw, path);
  TnStatusAssert(status);

  Stage stage;
  const char* fType;
  const char* sType;

  status = WDMapDump(&dw.WDMap);
  TnStatusAssert(status);

  while (1) {
    status = DirectoryWatcherGetStage(&dw, &stage);
    TnStatusAssert(status);

    switch (stage.FileType) {
      case STAGE_DIR:
        fType = "Directory";
        break;
      case STAGE_FILE:
        fType = "File";
        break;
      default:
        assert(0);
    }

    switch (stage.StageType) {
      case STAGE_CREATED:
        sType = "created";
        break;
      case STAGE_MODIFIED:
        sType = "modified";
        break;
      case STAGE_DELETED:
        sType = "deleted";
        break;
      default:
        assert(0);
    }

    fprintf(stderr, "\"%s\": %s %s\n", stage.Path, fType, sType);
    status = WDMapDump(&dw.WDMap);
    TnStatusAssert(status);
  }
}