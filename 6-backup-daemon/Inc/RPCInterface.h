#pragma once
#include "RequestHandler.h"

typedef struct {
  Logger* logger;
} RPCInterface;

TnStatus RPCInterfaceInit(RPCInterface* self, Logger* logger);
TnStatus RPCInterfaceDestroy(RPCInterface* self);
TnStatus RPCInterfaceHasCommand(RPCInterface* self, int* ret);
TnStatus RPCInterfaceGetCommand(RPCInterface* self, Command* cmd);
TnStatus RPCInterfaceSendResponce(RPCInterface* self, Responce responce);