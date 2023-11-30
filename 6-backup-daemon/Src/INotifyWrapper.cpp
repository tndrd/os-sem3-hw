#pragma once
#include "INotifyWrapper.hpp"

#include <vector>

using namespace HwBackup;

INotifyWrapper::INotifyWrapper(size_t eventCapacity)
    : Fd{FdInit()}, Buffer{BufferInit(eventCapacity)} {}

INotifyWrapper::~INotifyWrapper() {
  for (auto it = Cache.Begin(); it != Cache.End(); ++it) {
    int ret = RmWatch(it->Wd);
    if (ret < 0) STDERR_WARN_ERRNO("inotify_rm_watch()", errno);
  }
}

void INotifyWrapper::Register(const std::string& path, uint32_t mask) {
  int wd = inotify_add_watch(Fd.Get(), path.c_str(), mask);
  if (wd < 0) THROW_ERRNO("inotify_add_watch()", errno);

  Cache.Add({wd, path});
}

void INotifyWrapper::Unregister(WDCache::ListIt iter) {
  if (iter == Cache.End()) THROW("Entry not found");

  int ret = RmWatch(iter->Wd);
  if (ret < 0) THROW_ERRNO("inotify_rm_watch()", errno);

  Cache.Remove(iter);
}

void INotifyWrapper::Unregister(WDCache::WDescriptorT wd) {
  auto iter = Cache.Find(wd);
  Unregister(iter);
}

void INotifyWrapper::Unregister(const std::string& path) {
  auto iter = Cache.Find(path);
  Unregister(iter);
}

const WDCache& INotifyWrapper::GetCache() const { return Cache; }

auto INotifyWrapper::GetEvents() -> std::vector<Event> {
  int len = read(Fd.Get(), Buffer.data(), Buffer.size());
  if (len < 0) THROW_ERRNO("read()", errno);

  const inotify_event* event;
  const uint8_t* ptr = Buffer.data();

  std::vector<Event> events;

  for (; ptr < Buffer.data() + len; ptr += sizeof(inotify_event) + event->len) {
    event = reinterpret_cast<const inotify_event*>(ptr);

    Event newEvent;
    newEvent.Mask = event->mask;
    newEvent.Cookie = event->cookie;
    newEvent.Wd = event->wd;
    newEvent.Name = event->name;

    events.push_back(newEvent);
  }

  return events;
}

auto INotifyWrapper::FdInit() -> DescriptorWrapper {
  int ret = inotify_init();
  if (ret < 0) THROW_ERRNO("inotify_init()", errno);

  return ret;
}

auto INotifyWrapper::BufferInit(size_t eventCapacity) -> BufferT {
  return BufferT(eventCapacity * InotifyEventMaxSize, 0);
}

int INotifyWrapper::RmWatch(int wd) const {
  return inotify_rm_watch(Fd.Get(), wd);
}

void INotifyWrapper::RegisterPoll(pollfd& pollFd) {
  pollFd.fd = Fd.Get();
  pollFd.events = POLLIN;
}

bool INotifyWrapper::PollPending(const pollfd& pollFd) {
  return pollFd.fd == Fd.Get();
}

void INotifyWrapper::UnregisterAll() {
  for (auto it = Cache.Begin(); it != Cache.End(); ++it) {
    int ret = RmWatch(it->Wd);
    if (ret < 0) THROW_ERRNO("inotify_rm_watch()", errno);
    Cache.Remove(it);
  }
}