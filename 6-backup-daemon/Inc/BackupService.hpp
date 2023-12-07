#pragma once
#include <type_traits>

#include "BackupProducer.hpp"
#include "FSMonitor.hpp"
#include "FifoChannel.hpp"
#include "HwBackupException.hpp"
#include "Incremental.hpp"
#include "Logger.hpp"

#define HISTORY_NAME "History/"
#define CHANNEL_NAME "Channel"
#define CACHE_NAME "Cache/"

namespace HwBackup {
class BackupService {
 private:
  std::string SrcPath;
  std::string DstPath;

  Logger* LoggerPtr;
  PThread::Thread Thread;
  mutable PThread::Mutex Mutex;
  PThread::Cond Cond;

  FSMonitor Monitor;
  BackupProducer Producer;
  Fifo::Listener Listener;

  Selector Selector;
  SelectorAlarm Alarm;

  size_t Period;

  bool DoStop = false;

 public:
  BackupService(const std::string& srcPath, const std::string& dstPath,
                size_t period, Logger* loggerPtr)
      : Monitor{1, loggerPtr},
        Producer{std::move(ProducerInit(loggerPtr))},
        Listener{dstPath + CHANNEL_NAME},
        LoggerPtr{loggerPtr},
        SrcPath{srcPath},
        DstPath{dstPath},
        Period(period) {
    Alarm.RegisterAt(Selector);
  }

  void Run() {}

  void Thread1() {
    PathTree stages;

    Monitor.RegisterAt(Selector);
    Listener.RegisterAt(Selector);

    Monitor.Start(SrcPath);
    Producer.Open(SrcPath, DstPath + CACHE_NAME);

    while (1) {
      Mutex.Lock();
      if (DoStop) {
        Mutex.Unlock();
        break;
      }
      Mutex.Unlock();

      Selector.Wait();

      if (Monitor.DataReady(Selector)) {
        Mutex.Lock();
        Monitor.GetStages(stages);
        Mutex.Unlock();
      }

      if (Listener.DataReady(Selector)) {
        size_t newPeriod;
        uint8_t* buf = reinterpret_cast<uint8_t*>(&newPeriod);
        Listener.ReadTo(buf, sizeof(size_t));

        SetPeriod(newPeriod);
      }

      if (Alarm.HadAlarmed(Selector)) LOG_INFO(GetLogger(), "Got alarm");
    }
  }

  void Thread2() {
    while (1) {
      Mutex.Lock();
      if (DoStop) {
        Mutex.Unlock();
        break;
      }

      timespec ts = GetPeriodTimespec();
      Cond.TimedWait
    }
  }

  void SetPeriod(size_t newPeriod) {
    Mutex.Lock();
    Period = newPeriod;
    Mutex.Unlock();
  }

  size_t GetPeriod() const {
    Mutex.Lock();
    size_t period = Period;
    Mutex.Unlock();

    return period;
  }

 private:
  static FSMonitor MonitorInit(Logger* loggerPtr) {
    return FSMonitor{1, loggerPtr};
  }

  static BackupProducer ProducerInit(Logger* loggerPtr) {
    return BackupProducer{loggerPtr,
                          IncrBackupProducer::Create(HISTORY_NAME, loggerPtr)};
  }

  Logger& GetLogger() {
    assert(LoggerPtr);
    return *LoggerPtr;
  }

  timespec GetPeriodTimespec() const {
    timespec ts;
    size_t ms = GetPeriod();
    clock_gettime(CLOCK_REALTIME, &ts);

    size_t ns_per_s = 1000 * 1000 * 1000;

    // Make tv_nsec < 1s
    while (ts.tv_nsec > ns_per_s) {
      ts.tv_nsec -= ns_per_s;
      ts.tv_sec++;
    }
    
    // ns < 1ms
    size_t ns = ms % 1000;
    size_t s = ms / 1000;

    ts.tv_sec += s;
    ts.tv_nsec += ns; // < 1s + 1ms, so wont overflow;

    return ts;
  }
};

}  // namespace HwBackup
