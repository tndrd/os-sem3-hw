#pragma once

#include <pthread.h>
#include <signal.h>

#include <iostream>

#include "HwBackupException.hpp"
#include "StateValueWrapper.hpp"
#include "StderrWarning.hpp"

namespace HwBackup {
namespace PThread {

class Mutex : public StateValueWrapper<pthread_mutex_t> {
 public:
  Mutex();
  virtual ~Mutex();
  void Lock();
  void Unlock();
};

class Cond : public StateValueWrapper<pthread_cond_t> {
 public:
  Cond();
  virtual ~Cond();
  void Wait(Mutex& mutex);
  void Signal();
};

class Thread {
 public:
  using Routine = void* (*)(void*);

 private:
  StateValueWrapper<bool> IsJoined;
  StateValueWrapper<pthread_t> Impl;

 public:
  Thread(Routine target, void* args);
  Thread();
  virtual ~Thread();

  Thread(Thread&&) = default;
  Thread& operator=(Thread&&) = default;

 public:
  void Join();
  void Kill(int sig);
  void SetSigmask(int how, const sigset_t& set);
};

}  // namespace PThread
}  // namespace HwBackup