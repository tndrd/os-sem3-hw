#include "MCAlgorithm.h"

double PartialMonteCarlo(double (*foo)(double), CalculationArea limits,
                         size_t numPoints, int *seedPtr) {
  assert(foo);
  assert(seedPtr);

  size_t count = 0;

  for (int i = 0; i < numPoints; ++i) {
    double dx = limits.W * (double)(rand_r(seedPtr)) / RAND_MAX;
    double dy = limits.H * (double)(rand_r(seedPtr)) / RAND_MAX;

    double x = limits.X + dx;
    double y = limits.Y + dy;

    if (y < foo(x))
      count++;
  }

  double areaFilled = (double)(count) / (double)(numPoints);
  double area = limits.W * limits.H * areaFilled;
  return area;
}
