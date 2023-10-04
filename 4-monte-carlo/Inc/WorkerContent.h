#pragma once

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include "TnStatus.h"
#include "MCAlgorithm.h"

typedef struct {
  double (*Foo)(double);
  CalculationArea Limits;
  size_t NPoints;
  int* SeedPtr;
} WorkerArgs;

typedef struct {
  int Seed;
} WorkerTLS;

typedef double WorkerResult;
typedef void* (*WorkerFoo)(const WorkerArgs*, WorkerResult*);

void WorkerArgsInit(WorkerArgs *args);
TnStatus WorkerArgsValidate(WorkerArgs *args);
void WorkerResultInit(WorkerResult *result);
TnStatus WorkerResultValidate(WorkerResult *result);