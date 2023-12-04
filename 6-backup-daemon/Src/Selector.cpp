#include "Selector.hpp"

using namespace HwBackup;

size_t Selector::Register(int fd, short events) {
  pollfd newPollFd;

  newPollFd.fd = fd;
  newPollFd.events = events;

  PollFds.push_back(newPollFd);
  return PollFds.size() - 1;
}

void Selector::Wait() {
  int ret = poll(PollFds.data(), PollFds.size(), -1);
  if (ret < 0) THROW_ERRNO("poll()", errno);
}

short Selector::GetEvents(size_t i) const {
  return PollFds.at(i).revents;
}