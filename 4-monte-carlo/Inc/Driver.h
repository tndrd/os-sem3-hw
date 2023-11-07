#pragma once
#include "MonteCarloMT.h"
#include <time.h>
#include <unistd.h>

typedef struct {
  double Result;
  double TimeS;
} DriverResult;

DriverResult Driver(FunctionT function, size_t nWorkers, size_t nPoints);