#pragma once
#include <linux/limits.h>
#include <poll.h>
#include <sys/inotify.h>

#include <vector>

#include "DescriptorWrapper.hpp"
#include "WDCache.hpp"

namespace HwBackup {

class INotifyWrapper {
 public:
  struct Event {
    WDCache::WDescriptorT Wd;
    uint32_t Mask;
    uint32_t Cookie;
    std::string Name;
  };

 private:
  static constexpr size_t InotifyEventMaxSize =
      sizeof(inotify_event) + NAME_MAX + 1;

  using BufferT = std::vector<uint8_t>;

 private:
  DescriptorWrapper Fd;
  WDCache Cache;
  BufferT Buffer;

 public:
  INotifyWrapper(size_t eventCapacity);

  INotifyWrapper(const INotifyWrapper&) = delete;
  INotifyWrapper& operator=(const INotifyWrapper&) = delete;

  INotifyWrapper(INotifyWrapper&&) = default;
  INotifyWrapper& operator=(INotifyWrapper&&) = default;

  ~INotifyWrapper();

 public:
  void Register(const std::string& path, uint32_t mask);
  void Unregister(WDCache::WDescriptorT wd);
  void Unregister(const std::string& path);
  void UnregisterAll();
  const WDCache& GetCache() const;

  std::vector<Event> GetEvents();
  void RegisterPoll(pollfd& pollFd);
  bool PollPending(const pollfd& pollFd);

 private:
  static DescriptorWrapper FdInit();
  static BufferT BufferInit(size_t eventCapacity);

 private:
  int RmWatch(int wd) const;
  void Unregister(WDCache::ListIt it);
};

}  // namespace HwBackup