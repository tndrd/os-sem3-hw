#pragma once

#include "DescriptorWrapper.hpp"

using namespace HwBackup;

DescriptorWrapper::DescriptorWrapper(int fd)
    : StateValueWrapper{std::move(fd)} {}

DescriptorWrapper::~DescriptorWrapper() {
  int ret = close(Get());
  if (ret < 0) STDERR_WARN_ERRNO("Close()", errno);
}