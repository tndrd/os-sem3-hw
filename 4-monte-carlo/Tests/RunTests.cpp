#include "Driver.h"
#include "gtest/gtest.h"

using FooT = double (*)(double);
const double StepSize = 0.001;
const size_t NTiles = 10;
const size_t NWorkers = 4;
const size_t NPoints = 1000;
const double Threshold = 0.001;

CalculationArea GetFunctionLimits(FooT foo, double xStart, double xEnd,
                                  double stepSize, double threshold) {
  assert(xEnd > xStart);
  assert(threshold > 0);
  assert(stepSize > 0);

  double yMax = 0;
  double x = xStart;
  size_t nSteps = ((xEnd - xStart) / stepSize) + 1;

  for (int i = 0; i < nSteps + 1; ++i) {
    assert(foo(x) > -threshold);

    if (foo(x) - yMax > threshold) yMax = foo(x);

    x += stepSize;
  }

  CalculationArea limits;

  limits.X = xStart;
  limits.W = xEnd - xStart;
  limits.Y = 0;
  limits.H = yMax;

  return limits;
}

void TestFunction(FooT function, FooT primitive, double xStart, double xEnd,
                  double stepSize, size_t nTiles, size_t nWorkers,
                  size_t nPoints, double threshold) {
  CalculationArea limits =
      GetFunctionLimits(function, xStart, xEnd, stepSize, threshold);

  Vec2D tileSize;

  tileSize.X = limits.W / nTiles;
  tileSize.Y = limits.H / nTiles;

  CalculationResult result =
      Driver(function, limits, tileSize, nPoints, nWorkers);
  double expected = primitive(xEnd) - primitive(xStart);

  ASSERT_LE(std::abs(result.Value - expected), threshold)
      << "result: " << result.Value << ", expected: " << expected << std::endl;
}

void DoTest(FooT function, FooT primitive, double xStart, double xEnd) {
  return TestFunction(function, primitive, xStart, xEnd, StepSize, NTiles,
                      NWorkers, NPoints, Threshold);
}

#define FOO(name) double name(double x)

FOO(fc) { return 1; }

FOO(pc) { return x; }

FOO(fx) { return x; }

FOO(px) { return (x * x) / 2; }

FOO(fsin) { return sin(x); }

FOO(psin) { return -cos(x); }

FOO(fexp) { return exp(x); }

FOO(pexp) { return exp(x); }

FOO(f1divx) { return 1 / x; }

FOO(plog) { return log(x); }

TEST(Functions, Fx) { DoTest(fx, px, 0, 1); }

TEST(Functions, Fc) { DoTest(fc, pc, -100, 100); }

TEST(Functions, Fsin) { DoTest(fsin, psin, 0, 3.14); }

TEST(Functions, Fexp) { DoTest(fexp, pexp, -10, 1); }

TEST(Functions, Fxdiv1) { DoTest(f1divx, plog, 1, 10); }

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}