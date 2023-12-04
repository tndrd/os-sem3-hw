#include <poll.h>
#include <vector>
#include <HwBackupException.hpp>

namespace HwBackup {
struct Selector {
  private:
    std::vector<pollfd> PollFds;
  public:
    size_t Register(int fd, short events);
    void Wait();

    short GetEvents(size_t i) const;
};
}