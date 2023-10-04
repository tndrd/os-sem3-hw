#include "WorkerContent.h"

void WorkerArgsInit(WorkerArgs *args)
{
  assert(args);

  args->Foo = NULL;

  args->Limits.X = NAN;
  args->Limits.Y = NAN;
  args->Limits.W = NAN;
  args->Limits.H = NAN;

  args->NPoints = 0;
  args->SeedPtr = NULL;
}

TnStatus WorkerArgsValidate(WorkerArgs *args)
{
  if (!args)
    return TN_BAD_ARG_PTR;
  if (!args->Foo)
    return TN_WORKER_BAD_FOO;
  if (!args->SeedPtr)
    return TN_BAD_ARG_PTR;

  if (isnan(args->Limits.X))
    return TN_BAD_VALUE;
  if (isnan(args->Limits.Y))
    return TN_BAD_VALUE;
  if (isnan(args->Limits.W))
    return TN_BAD_VALUE;
  if (isnan(args->Limits.H))
    return TN_BAD_VALUE;

  if (args->NPoints == 0)
    return TN_BAD_VALUE;

  return TN_SUCCESS;
}

void WorkerResultInit(WorkerResult *result)
{
  assert(result);
  *result = NAN;
}

TnStatus WorkerResultValidate(WorkerResult *result)
{
  if (!result)
    return TN_BAD_ARG_PTR;

  return isnan(*result) ? TN_BAD_VALUE : TN_SUCCESS;
}