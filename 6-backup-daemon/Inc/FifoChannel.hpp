#pragma once

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <array>

#include "DescriptorWrapper.hpp"
#include "HwBackupException.hpp"
#include "PThreadWrapper.hpp"
#include "Selector.hpp"
#include "Helpers.hpp"

#define LISTENER_BUFSIZE 256

namespace HwBackup {

struct Fifo {
  class Listener {
   private:
    DescriptorWrapper Fd;
    std::string Path;
    Helpers::Pipe Pipe;
    PThread::Thread Thread;
    PThread::Mutex Mutex;
    std::array<uint8_t, LISTENER_BUFSIZE> Buffer;

    Selector::IdT SelectorId;

   public:
    Listener(const std::string& path) : Path{path} {
      int ret = mkfifo(Path.c_str(), 0666);
      if (ret < 0) THROW_ERRNO("mkfifo()", errno);
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

    void RegisterAt(Selector& selector) {
      SelectorId = selector.Register(Pipe.GetOut(), POLLIN);
    }

    bool DataReady(const Selector& selector) {
      return selector.GetEvents(SelectorId) & POLLIN;
    }

    Listener(Listener&&) = default;
    Listener& operator=(Listener&&) = default;

    ~Listener() {
      int ret = unlink(Path.c_str());
      if (ret < 0) {
        STDERR_WARN_ERRNO("unlink()", errno);
      }
    }

   private:
    void MainLoop() {
      while (1) {
        int ret = open(Path.c_str(), O_RDONLY);
        if (ret < 0) THROW_ERRNO("open()", errno);

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
  };
};

};  // namespace HwBackup