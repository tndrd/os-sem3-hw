#pragma once
#include <assert.h>

#include <ostream>
#include <string>

namespace HwBackup {

class Stage final {
 public:
  enum class FileTypeEnum { Regular, Directory, Undefined };
  enum class ChangeTypeEnum { Created, Modified, Renamed, Deleted, Undefined };

  using FileT = FileTypeEnum;
  using ChangeT = ChangeTypeEnum;

 public:
  FileT FileType;
  ChangeT ChangeType;
  std::string Path;
  time_t Timestamp;

 public:
  std::ostream& Dump(std::ostream& os) const;
  static Stage Create(FileT, ChangeT, const std::string& Path);

  static const char* ToString(FileT);
  static const char* ToString(ChangeT);
};

std::ostream& operator<<(std::ostream& os, const Stage& stage);

}  // namespace HwBackup
