#pragma once

#include <dirent.h>

#include <queue>

#include "INotifyWrapper.hpp"
#include "Logger.hpp"
#include "PThreadWrapper.hpp"
#include "Stage.hpp"

namespace HwBackup {

class DirectoryWatcher {
 private:
  using QueueT = std::queue<Stage>;

 private:
  static constexpr int InterruptSigno = SIGUSR1;

  Logger* LoggerPtr;

  INotifyWrapper INotify;
  size_t INotifyQueueSize;
  QueueT Queue;

  PThread::Cond Empty;
  mutable PThread::Mutex Mutex;
  PThread::Thread Thread;

  std::string RootPath;
  bool Active = false;

 public:
  DirectoryWatcher(Logger* loggerPtr, size_t queueSize)
      : LoggerPtr{loggerPtr}, INotify{queueSize}, INotifyQueueSize{queueSize} {
    if (!loggerPtr) THROW("Logger pointer is null");
  }

  void Start(const std::string& rootPath) {
    if (Active) THROW("Already started");
    RootPath = rootPath;

    RegisterTree(".");

    TweakSignals_Main();
    Active = true;
    Thread = {MainLoopAdapter, this};
  }

  void Stop() {
    if (!Active) THROW("Not started");
    Active = false;
    Thread.Kill(InterruptSigno);
    Thread.Join();
    INotify.UnregisterAll();
  }

 private:
  Logger& GetLogger() {
    assert(LoggerPtr);
    return *LoggerPtr;
  }

  void MainLoop() {
    TweakSignals_Thread();

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, InterruptSigno);

    sigset_t currentMask;
    pthread_sigmask(SIG_BLOCK, NULL, &currentMask);

    pollfd pollFd;
    INotify.RegisterPoll(pollFd);

    while (1) {
      pthread_sigmask(SIG_BLOCK, &set, NULL);
      if (!Active) {
        pthread_sigmask(SIG_UNBLOCK, &set, NULL);
        break;
      }
      if (ppoll(&pollFd, 1, NULL, &currentMask) < 0)
        THROW_ERRNO("poll()", errno);
      pthread_sigmask(SIG_UNBLOCK, &set, NULL);

      if (pollFd.revents & POLLIN)
        for (const auto& event: INotify.GetEvents())
          HandleEvent(event);
    }
  }

  void HandleEvent(const INotifyWrapper::Event& event) {
    auto cached = INotify.GetCache().Find(event.Wd);
    if (cached == INotify.GetCache().End())
      THROW("Watch desciptor is not cached");

    std::string eventPath = cached->Path + "/" + event.Name;

    using FileT = Stage::FileT;
    using ChangeT = Stage::ChangeT;

    if (event.Mask & (IN_ISDIR | IN_CREATE)) { // New directory created
      RegisterTree(eventPath);
      PushStage(FileT::Directory, ChangeT::Created, eventPath);
    }

    if (event.Mask & (IN_ISDIR | IN_DELETE)) { // Directory deleted
      INotify.Unregister(eventPath);
      PushStage(FileT::Directory, ChangeT::Deleted, eventPath);
    }

    if (event.Mask & IN_ISDIR) return;

    if (event.Mask & IN_CREATE) // New file created
      PushStage(FileT::Regular, ChangeT::Created, eventPath);
    
    if (event.Mask & IN_MODIFY) // File modified
      PushStage(FileT::Regular, ChangeT::Modified, eventPath);

    if (event.Mask & IN_DELETE)
      PushStage(FileT::Regular, ChangeT::Deleted, eventPath);
  }

  void PushStage(Stage::FileT fileT, Stage::ChangeT changeT, const std::string& path) {
    Mutex.Lock();
    Queue.emplace(Stage::Create(fileT, changeT, path));
    Empty.Signal();
    Mutex.Unlock();
  }

  bool HasStages() const {
    Mutex.Lock();
    bool result = !Queue.empty();
    Mutex.Unlock();
    return result;
  }

  Stage GetStage() {
    Mutex.Lock();
    while(Queue.empty())
      Empty.Wait(Mutex);
    Stage result = Queue.front();
    Mutex.Unlock();

    return result;
  }

  static void DummySigHandler(int signo) {}

  static void BlockAllSignals() {
    sigset_t set;
    sigfillset(&set);

    int ret = sigprocmask(SIG_BLOCK, &set, NULL);
    if (ret < 0) THROW_ERRNO("sigprocmask()", errno);
  }

  static void UnblockAllSignals() {
    sigset_t set;
    sigfillset(&set);

    int ret = sigprocmask(SIG_UNBLOCK, &set, NULL);
    if (ret < 0) THROW_ERRNO("sigprocmask()", errno);
  }

  void TweakSignals_Main() {
    BlockAllSignals();

    struct sigaction sa;
    int ret;
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, InterruptSigno);

    sa.sa_handler = DummySigHandler;
    sa.sa_mask = set;

    ret = sigaction(InterruptSigno, &sa, NULL);
    if (ret < 0) THROW_ERRNO("sigaction()", errno);

    ret = pthread_sigmask(SIG_BLOCK, &set, NULL);
    if (ret < 0) THROW_ERRNO("pthread_sigmask()", errno);
  }

  void TweakSignals_Thread() {
    int ret;
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, InterruptSigno);

    ret = pthread_sigmask(SIG_UNBLOCK, &set, NULL);
    if (ret < 0) THROW_ERRNO("pthread_sigmask()", errno);

    UnblockAllSignals();
  }

  static void* MainLoopAdapter(void* args) {
    assert(args);
    DirectoryWatcher* thisPtr = reinterpret_cast<DirectoryWatcher*>(args);
    thisPtr->MainLoop();
    return NULL;
  }

  std::vector<std::string> ScanTree(const std::string& relPathRoot) {
    std::vector<std::string> BFSCache;
    BFSCache.push_back(relPathRoot);
    auto currentIt = BFSCache.begin();

    auto deleter = [](DIR* dir) {
      int ret = closedir(dir);
      if (ret < 0) THROW_ERRNO("closedir()", errno);
    };

    do {
      std::string relPath = *currentIt;
      std::string absPath = RootPath + relPath;
      std::unique_ptr<DIR, decltype(deleter)> dir{opendir(absPath.c_str()),
                                                  deleter};

      if (!dir.get()) THROW_ERRNO("Failed to open directory", errno);

      dirent* curDir = NULL;
      while ((curDir = readdir(dir.get())) != NULL) {
        if (curDir->d_type == DT_REG) continue;

        std::string entryPath = relPath + "/" + curDir->d_name;

        if (curDir->d_type != DT_DIR) {
          LOG_WARN(GetLogger(),
                   "File type of \"" << entryPath << "\" is not supported");
          continue;
        }

        BFSCache.push_back(entryPath);
      }
      ++currentIt;
    } while (currentIt != BFSCache.end());

    return BFSCache;
  }

  void RegisterTree(const std::string& relPathRoot) {
    auto dirs = ScanTree(relPathRoot);
    RegisterDirectories(dirs);
  }

  void RegisterDirectories(const std::vector<std::string>& dirs) {
    auto iter = dirs.cbegin();

    uint32_t mask = IN_CREATE | IN_MODIFY | IN_DELETE;

    for (; iter != dirs.cend(); ++iter) {
      try {
        INotify.Register(*iter, mask);
      } catch (...) {
        for (; iter != dirs.cbegin(); --iter) INotify.Unregister(*iter);
        throw;
      }
    }
  }
};

}  // namespace HwBackup