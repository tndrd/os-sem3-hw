#pragma once
#include <string.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <utility>

#include "HwBackupException.hpp"
#include "StateValueWrapper.hpp"
#include "StderrWarning.hpp"

namespace HwBackup {

class DescriptorWrapper : public StateValueWrapper<int> {
 public:
  DescriptorWrapper(int);
  DescriptorWrapper() = default;
  virtual ~DescriptorWrapper();
};

}  // namespace HwBackup