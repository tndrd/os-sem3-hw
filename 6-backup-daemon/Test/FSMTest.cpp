#include "FSMonitor.hpp"
#include "BackupProducer.hpp"

using namespace HwBackup;

int main(int argc, char* argv[]) {
  if (argc != 2) THROW("Not enough arguments, expected path");

  const char* path = argv[1];

  std::ostream* LoggingStream = &std::cout;
  Logger logger {LoggingStream};

  FSMonitor monitor {1, &logger};
  BackupProducer backup {&logger};

  backup.Open("dst/", path);
  monitor.Start(path);

  PathTree stages;

  while(1) {
    monitor.GetStages(stages);
    stages.Dump(std::cout);
    backup.Backup(stages);

    stages.Clear();
  }
}