#include "FSMonitor.hpp"

using namespace HwBackup;

int main(int argc, char* argv[]) {
  if (argc != 2) THROW("Not enough arguments, expected path");

  const char* path = argv[1];

  std::ostream* LoggingStream = &std::cout;
  Logger logger {LoggingStream};

  FSMonitor monitor {1, &logger};

  monitor.Start(path);

  while(1) {
    auto stages = monitor.GetStages();
    
    for (const auto& stage: stages)
      std::cout << stage << std::endl;
  }
}