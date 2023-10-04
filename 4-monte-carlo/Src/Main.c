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

TnStatus TilerInit(Tiler *tiler, CalculationArea limits, Vec2D tileSize)
{
  if (!tiler)
    return TN_BAD_ARG_PTR;

  tiler->Limits = limits;
  tiler->TileSize = tileSize;
  tiler->Position.X = limits.X;
  tiler->Position.Y = limits.Y;

  return TN_SUCCESS;
}

TnStatus TilerNext(Tiler *tiler, CalculationArea *result)
{
  if (!tiler)
    return TN_BAD_ARG_PTR;

  result->X = tiler->Position.X;
  result->Y = tiler->Position.Y;
  result->W = tiler->TileSize.X;
  result->H = tiler->TileSize.Y;

  double newPosX = tiler->Position.X + tiler->TileSize.X;
  double newPosY = tiler->Position.Y;

  double width = tiler->Limits.X + tiler->Limits.W;
  double height = tiler->Limits.Y + tiler->Limits.H;

  if (newPosX > width)
  {
    newPosX = tiler->Limits.X;
    newPosY += tiler->TileSize.Y;

    if (newPosY > height)
      return TILER_FINISHED;
  }

  tiler->Position.X = newPosX;
  tiler->Position.Y = newPosY;

  return TN_SUCCESS;
}

void ExitWithError(const char *msg)
{
  fprintf(stderr, "Error: %s\n", msg);
  exit(1);
}

#define RUN(expr)           \
  status = expr;            \
  if (status != TN_SUCCESS) \
    ExitWithError(WorkerGetErrorDescription(status, worker));

Worker *CreateWorkerArray(size_t nWorkers)
{
  TnStatus status;
  Worker *workers = (Worker *)malloc(nWorkers * sizeof(Worker));

  if (!workers)
    ExitWithError("Failed to allocate memory");

  for (int i = 0; i < nWorkers; ++i)
  {
    Worker *worker = &workers[i];
    RUN(WorkerInit(worker));
    worker->TLS.Seed = time(NULL);
  }

  return workers;
}

void *MCWrapper(const WorkerArgs *args, WorkerResult *result)
{

  double (*foo)(double) = args->Foo;
  CalculationArea limits = args->Limits;
  size_t nPoints = args->NPoints;
  int *seedPtr = args->SeedPtr;

  *result = PartialMonteCarlo(foo, limits, nPoints, seedPtr);
}

double CalculateIntegralSum(const double (*foo)(double), CalculationArea limits, Vec2D tileSize, size_t nPoints, size_t nWorkers)
{
  if (!foo)
    ExitWithError("Bad function pointer");

  TnStatus status;
  Worker *workers = CreateWorkerArray(nWorkers);

  Tiler tiler;
  TilerInit(&tiler, limits, tileSize);

  WorkerArgs args;
  WorkerArgsInit(&args);
  args.Foo = foo;
  args.NPoints = nPoints;

  int done = 0;
  double sum = 0;
  double res;
  WorkerState state;

  while (!done)
  {
    for (int i = 0; i < nWorkers; ++i)
    {
      Worker *worker = &workers[i];

      RUN(WorkerGetState(worker, &state));

      if (state == WORKER_INIT)
      {
        if (TilerNext(&tiler, &args.Limits) == TILER_FINISHED)
          done = 1;

#ifdef DBGOUT
        fprintf(stderr, "Thread #%d: Start [%lf, %lf]\n", i, args.Limits.X, args.Limits.Y);
#endif
        RUN(WorkerSetTarget(worker, &MCWrapper));

        args.SeedPtr = &worker->TLS.Seed;

        RUN(WorkerSetArgs(worker, &args));
        RUN(WorkerRun(worker));
      }

      if (state == WORKER_DONE)
      {
        RUN(WorkerReadResult(worker, &res));
        sum += res;
#ifdef DBGOUT
        fprintf(stderr, "Thread #%d: Finish [%lf]\n", i, sum);
#endif
      }

      if (state == WORKER_ERROR)
      {
        const char *desc = WorkerGetErrorDescription(worker->Error, worker);
        fprintf(stderr, "Thread #%d: Error: %s\n", i, desc);
        exit(1);
      }
    }
  }

  for (int i = 0; i < nWorkers; ++i)
  {
    WorkerState state;
    Worker *worker = &workers[i];

    do
    {
      RUN(WorkerGetState(worker, &state));
    } while (state != WORKER_DONE && state != WORKER_INIT);

    if (state == WORKER_DONE)
    {
      RUN(WorkerReadResult(worker, &res));
      sum += res;
    }
  }

  for (int i = 0; i < nWorkers; ++i)
  {
    Worker *worker = &workers[i];
    RUN(WorkerStop(worker));
  }

  return sum;
}

#undef RUN

double foo(double x)
{
  return x;
}

double foo2(double x)
{
  return (x * x * x)+ (x * x) + 1;
}

double foo2a (double x)
{
  return (x * x * x * x) / 4 + (x * x * x) / 3 + x;
}

int main(int argc, char *argv[])
{
  CalculationArea limits = {0, 0, 2, 13};
  Vec2D tileSize = {0.05, 0.05};

  double result = CalculateIntegralSum(&foo2, limits, tileSize, 100000, 8);
  printf("Result: %lf\n", result);
  printf("Test: %lf\n", foo2a(2));
}