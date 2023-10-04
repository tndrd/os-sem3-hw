#pragma once

#include <assert.h>
#include <math.h>
#include <stdlib.h>

typedef struct {
  double X, Y;
  double W, H;
} CalculationArea;

double PartialMonteCarlo(double (*foo)(double), CalculationArea limits,
                         size_t nPoints, int *seedPtr);
