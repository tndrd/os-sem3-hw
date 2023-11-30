#pragma once
#include "Stage.h"
#include "TnStatus.h"
#include "Logger.h"

typedef struct {
  Logger* Logger;
  const char* Path;
} BackupProducer;

TnStatus BackupProducerInit(BackupProducer* self, Logger* logger);
TnStatus BackupProducerDestroy(BackupProducer* self);
TnStatus BackupProducerBackup(BackupProducer* self, const Stage* stage);