#pragma once

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <array>

#include "DescriptorWrapper.hpp"
#include "Helpers.hpp"
#include "HwBackupException.hpp"
#include "PThreadWrapper.hpp"
#include "Selector.hpp"

#define LISTENER_BUFSIZE 256
#define CTL_SIGNAL SIGUSR1

namespace HwBackup {

struct Fifo {
  using DataT = size_t;

  class Listener {
   private:
    DescriptorWrapper Fd;
    std::string Path;
    Helpers::Pipe Pipe;
    PThread::Thread Thread;
    std::array<uint8_t, LISTENER_BUFSIZE> Buffer;

    Selector::IdT SelectorId;
    bool DoStop = false;

   public:
    Listener(const std::string& path) : Path{path} {
      int ret = mkfifo(Path.c_str(), 0666);
      if (ret < 0 && errno != EEXIST) THROW_ERRNO("mkfifo()", errno);
    }

    void RegisterAt(Selector& selector) {
      SelectorId = selector.Register(Pipe.GetOut(), POLLIN);
    }

    bool DataReady(const Selector& selector) {
      return selector.GetEvents(SelectorId) & POLLIN;
    }

    DataT GetData() {
      DataT result;
      uint8_t* buf = reinterpret_cast<uint8_t*>(&result);
      ReadTo(buf, sizeof(result));

      return result;
    }

    void Start() {
      DoStop = false;
      Thread = PThread::Thread{MainLoopAdapter, this};
    }

    void Stop() {
      DoStop = true;
      Thread.Kill(CTL_SIGNAL);
      Thread.Join();
    }

    static void DummyHandler(int sigNo) {}

    Listener(Listener&&) = default;
    Listener& operator=(Listener&&) = default;

    ~Listener() = default;

   private:
    static void* MainLoopAdapter(void* thisPtr) {
      assert(thisPtr);
      Listener* self = reinterpret_cast<Listener*>(thisPtr);
      self->MainLoop();
      return NULL;
    }

    void ReadTo(uint8_t* buf, size_t size) {
      size_t total = 0;

      while (total < size) {
        int ret = read(Pipe.GetOut(), buf + total, size - total);
        if (ret < 0) THROW_ERRNO("read()", errno);
        if (ret == 0) THROW("Pipe closed unexpectedly");

        total += ret;
      }
    }

    void MainLoop() {
      TweakSignals();
      while (1) {
        int ret = open(Path.c_str(), O_RDONLY);
        if (ret < 0 && errno != EINTR) THROW_ERRNO("open()", errno);
        if (ret < 0 && errno == EINTR) {
          if (DoStop) break;

          THROW("Unexpected signal received");
        }

        Fd = DescriptorWrapper{ret};

        while (1) {
          int read = ReadToBuf();
          if (!read) break;
          WriteToPipe(read);
        }
      }
    }

    size_t ReadToBuf() {
      int ret = read(Fd.Get(), Buffer.data(), Buffer.size());

      if (ret < 0) THROW_ERRNO("read()", errno);

      return ret;
    }

    void WriteToPipe(size_t sz) {
      size_t total = 0;

      while (total < sz) {
        int ret = write(Pipe.GetIn(), Buffer.data() + total, sz - total);
        if (ret <= 0) THROW_ERRNO("write()", errno);

        total += ret;
      }
    }

    void Sig1Mask(int how, int sigNo) {
      sigset_t set;
      sigemptyset(&set);
      sigaddset(&set, sigNo);
      Thread.SetSigmask(how, set);
    }

    void TweakSignals() {
      Sig1Mask(SIG_BLOCK, CTL_SIGNAL);
      struct sigaction sa;
      sigset_t set;
      sa.sa_flags = 0;

      sigemptyset(&sa.sa_mask);
      sigaddset(&sa.sa_mask, CTL_SIGNAL);
      sa.sa_handler = DummyHandler;

      int ret = sigaction(CTL_SIGNAL, &sa, NULL);
      if (ret < 0) THROW_ERRNO("sigaction()", errno);
      Sig1Mask(SIG_UNBLOCK, CTL_SIGNAL);
    }
  };

  class Client {
   private:
    DescriptorWrapper Fd;
    bool Connected = false;

   public:
    Client() = default;

    void ConnectTo(const std::string& path) {
      if (Connected) THROW("Already connected");

      ValidatePath(path);

      int ret = open(path.c_str(), O_WRONLY);
      if (ret < 0) THROW_ERRNO("open()", errno);

      Fd = {ret};
      Connected = true;
    }

    void Disconnect() {
      if (!Connected) THROW("Not connected");
      Fd = {};
      Connected = false;
    }

    void Send(DataT data) {
      if (!Connected) THROW("Not connected");

      uint8_t* buf = reinterpret_cast<uint8_t*>(&data);
      Write(buf, sizeof(data));
    }

   private:
    void Write(uint8_t* buf, size_t size) {
      size_t total = 0;
      while (total < size) {
        int ret = write(Fd.Get(), buf + total, size - total);
        if (ret < 0) THROW_ERRNO("write()", errno);
        if (ret == 0) THROW("Server closed connection");

        total += ret;
      }
    }

    void ValidatePath(const std::string& path) {
      struct stat st;
      int ret = stat(path.c_str(), &st);
      if (ret < 0) THROW_ERRNO("stat()", errno);
      if (!S_ISFIFO(st.st_mode)) THROW("File " + path + " is not a FIFO");
    }
  };
};

};  // namespace HwBackup