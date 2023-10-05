#pragma once

#include "Worker.h"
#include "stdio.h"

typedef struct
{
  double X;
  double Y;
} Vec2D;

typedef struct
{
  CalculationArea Limits;
  Vec2D TileSize;
  Vec2D Position;
} Tiler;

typedef struct {
  double Value;
  double TimeS;
} CalculationResult;

static TnStatus TilerInit(Tiler *tiler, CalculationArea limits, Vec2D tileSize);
static TnStatus TilerNext(Tiler *tiler, CalculationArea *result);
static void ExitWithError(const char *msg);
static Worker *CreateWorkerArray(size_t nWorkers);
static void *MCWrapper(const WorkerArgs *args, WorkerResult *result);
static double CalculateIntegralSum(const double (*foo)(double), CalculationArea limits, Vec2D tileSize, size_t nPoints, size_t nWorkers);

#ifdef __cplusplus
extern "C" {
#endif

CalculationResult Driver(double (*foo)(double), CalculationArea limits, Vec2D tileSize, size_t nPoints, size_t nWorkers);

#ifdef __cplusplus
}
#endif