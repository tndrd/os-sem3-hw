#pragma once
#include "RequestHandler.h"

typedef struct {
  Logger* logger;
} SignalInterface;

TnStatus SignalInterfaceInit(SignalInterface* self, Logger* logger);
TnStatus SignalInterfaceDestroy(SignalInterface* self);
TnStatus SignalInterfaceHasRequest(SignalInterface* self, int* ret);
TnStatus SignalInterfaceGetRequest(SignalInterface* self, Command* ret);