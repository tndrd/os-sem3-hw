#include "FSMonitor.hpp"
#include "BackupProducer.hpp"
#include "Incremental.hpp"

using namespace HwBackup;

int main(int argc, char* argv[]) {
  if (argc != 2) THROW("Not enough arguments, expected path");

  const char* path = argv[1];

  std::ostream* LoggingStream = &std::cout;
  Logger logger {LoggingStream};

  FSMonitor monitor {1, &logger};
  IEventObserver::PtrT incr = std::make_unique<IncrBackupProducer> ("dst/History/", &logger);
  BackupProducer backup {&logger, std::move(incr)};

  backup.Open(path, "dst/Cache/");
  monitor.Start(path);

  PathTree stages;

  while(1) {
    monitor.GetStages(stages);
    stages.Dump(std::cout);
    backup.Sync(stages);

    stages.Clear();
  }
}