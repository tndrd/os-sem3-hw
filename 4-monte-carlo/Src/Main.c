#include "Driver.h"

double foo (double x) {
  return x;
}

int main() {
  CalculationArea limits = {0, 0, 1, 1};
  Vec2D tileSize = {0.05, 0.05};
  size_t nPoints = 1000;
  size_t nWorkers = 10;

  CalculationResult result = Driver(&foo, limits, tileSize, nPoints, nWorkers);

  printf("Result: %lf\n", result.Value);
  printf("Time: %lf s\n", result.TimeS);
}