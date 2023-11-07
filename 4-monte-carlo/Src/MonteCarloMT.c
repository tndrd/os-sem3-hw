#include "MonteCarloMT.h"

void ExitWithError(const char* msg) {
  assert(msg);
  fprintf(stderr, "Error: %s\n", msg);
  exit(1);
}

Tiler CreateTiler(Rectangle2D area, size_t xCount, size_t yCount) {
  if (xCount == 0 || yCount == 0)
    ExitWithError("Wrong tiler parameters");

  Tiler tiler;
  tiler.Offset = area.Position;

  tiler.TileSize.X = area.Size.X / xCount;
  tiler.TileSize.Y = area.Size.Y / yCount;
  tiler.XCount = xCount;
  tiler.YCount = yCount;

  tiler.NTiles = xCount * yCount;
  tiler.Current = 0;

  return tiler;
}

int TilerGetNext(Tiler* tiler, Rectangle2D* tile) {
  assert(tiler);

  if (tiler->Current == tiler->NTiles)
    return 0;
  
  size_t tileX = tiler->Current % tiler->XCount;
  size_t tileY = tiler->Current / tiler->XCount;

  tile->Position.X = tiler->Offset.X + tileX * tiler->TileSize.X; 
  tile->Position.Y = tiler->Offset.Y + tileY * tiler->TileSize.Y;

  tile->Size.X = tiler->TileSize.X;
  tile->Size.Y = tiler->TileSize.Y;

  tiler->Current++;

  return 1; 
}

static void MonteCarloAdapter(void* argsPtr, void* resultPtr) {
  MonteCarloArgs* args = (MonteCarloArgs*)argsPtr;
  double* result = (double*)resultPtr;

  *result = MonteCarlo(args);
}

double MonteCarloMT(MonteCarloArgs* args, size_t nWorkers, size_t tilesX,
                    size_t tilesY) {
  assert(args);
  assert(args->Function);

  ThreadPool tp;

  if (ThreadPoolInit(&tp, nWorkers) != STATUS_SUCCESS)
    ExitWithError("Failed to create thread pool\n");

  if (ThreadPoolRun(&tp) != STATUS_SUCCESS)
    ExitWithError("Failed to run thread pool\n");

  Tiler tiler = CreateTiler(args->Limits, tilesX, tilesY);

  WorkerTask* tasks;
  MCTask* tasksImpl;
  Rectangle2D tile;

  tasks = (WorkerTask*)malloc(tiler.NTiles * sizeof(WorkerTask));
  tasksImpl = (MCTask*)malloc(tiler.NTiles * sizeof(MCTask));

  if (!tasks || !tasksImpl) ExitWithError("Failed to allocate memory\n");

  for (int i = 0; i < tiler.NTiles; ++i) {
    if(!TilerGetNext(&tiler, &tile))
      ExitWithError("Tiler is weird\n");

    tasksImpl[i].Args = *args;
    tasksImpl[i].Args.Limits = tile;
    tasksImpl[i].Args.NPoints /= tiler.NTiles;
    tasksImpl[i].Result = NAN;

    tasks[i].Args = &tasksImpl[i].Args;
    tasks[i].Result = &tasksImpl[i].Result;
    tasks[i].Function = MonteCarloAdapter;

    if(ThreadPoolAddTask(&tp, tasks[i]) != STATUS_SUCCESS)
      ExitWithError("Failed to assign task\n");
  }

  if (TilerGetNext(&tiler, &tile))
      ExitWithError("Tiler is weird again\n");

  if(ThreadPoolWaitAll(&tp) != STATUS_SUCCESS)
    ExitWithError("Failed to wait\n");

  double result = 0;
  for (int i = 0; i < tiler.NTiles; ++i) {
    assert(!isnan(tasksImpl[i].Result));
    result += tasksImpl[i].Result;
  }

  return result;
}