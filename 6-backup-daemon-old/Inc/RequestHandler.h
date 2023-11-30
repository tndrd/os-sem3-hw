#pragma once
#include "DBackuperManager.h"

typedef void (*GenericHandler)(DBackuperManager*, const void*, void*); 

typedef struct Command {
  const void* Request;
  void* Responce;
  GenericHandler Handler;
};

typedef struct Responce {
  TnStatus Result;
  void* Data;
};

typedef struct {
  Logger* logger;
} RequestHandler;

TnStatus RequestHandlerInit(RequestHandler* self, Logger* logger);
TnStatus RequestHandlerDestroy(RequestHandler* self);
TnStatus RequestHandlerHandle(RequestHandler* self, DBackuperManager* target, Command command);