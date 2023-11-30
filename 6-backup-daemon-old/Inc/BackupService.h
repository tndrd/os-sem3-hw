#pragma once
#include "SignalInterface.h"
#include "RPCInterface.h"
#include "Logger.h"

typedef struct {
  SignalInterface ISig;
  RPCInterface IRPC;
  DBackuperManager Manager;
  RequestHandler Handler;
  Logger Logger;
} BackupService;

TnStatus BackupServiceInit(BackupService* self, LoggingLevel logLevel);
TnStatus BackupServiceDestroy(BackupService* self);
TnStatus BackupServiceRun(BackupService* self);