#pragma once
#include <string>

namespace HwBackup {

class Stage {
 public:
  enum class FileTypeEnum { Regular, Directory };
  enum class ChangeTypeEnum { Created, Modified, Renamed, Deleted };

  using FileT = FileTypeEnum;
  using ChangeT = ChangeTypeEnum;

 public:
  FileT FileType;
  ChangeT ChangeType;
  std::string Path;
  time_t Timestamp;

 public:
  static Stage Create(FileT, ChangeT, const std::string& Path);
};

}  // namespace HwBackup
