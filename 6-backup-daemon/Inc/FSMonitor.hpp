#pragma once
#include <dirent.h>
#include <linux/limits.h>
#include <poll.h>
#include <sys/inotify.h>

#include <vector>

#include "DescriptorWrapper.hpp"
#include "Logger.hpp"
#include "Selector.hpp"
#include "WDCache.hpp"
#include "PathTree.hpp"

namespace HwBackup {

class FSMonitor final {
 private:
  static constexpr size_t InotifyEventMaxSize =
      sizeof(inotify_event) + NAME_MAX + 1;

  using BufferT = std::vector<uint8_t>;

 private:
  Logger* LoggerPtr;

  DescriptorWrapper Fd;
  WDCache Cache;
  BufferT Buffer;

  std::string RootPath;
  bool Started = false;

  Selector::IdT SelectorId;
  bool IsSelected = false;

 public:
  FSMonitor(size_t eventCapacity, Logger* loggerPtr);

  FSMonitor(const FSMonitor&) = delete;
  FSMonitor& operator=(const FSMonitor&) = delete;

  FSMonitor(FSMonitor&&) = default;
  FSMonitor& operator=(FSMonitor&&) = default;

  ~FSMonitor();

 public:
  void Start(const std::string& rootPath);
  void Stop();
  void GetStages(PathTree& dst);

 public:
  void RegisterAt(Selector& selector);
  bool DataReady(const Selector& selector) const;

 private:
  void Register(const std::string& eventPath, uint32_t mask);
  void Unregister(WDCache::WDescriptorT wd);
  void Unregister(const std::string& path);
  void UnregisterAll();

 private:
  static DescriptorWrapper FdInit();
  static BufferT BufferInit(size_t eventCapacity);
  void HandleEvent(const inotify_event& event, const std::string& eventPath);

  std::vector<std::string> ScanTree(const std::string& relPathRoot);
  void RegisterTree(const std::string& relPathRoot);
  void RegisterDirectories(const std::vector<std::string>& dirs);

  Logger& GetLogger();

  int RmWatch(int wd) const;
  void Unregister(WDCache::ListIt it);
};

}  // namespace HwBackup