#pragma once

#include <dirent.h>

#include <functional>
#include <memory>
#include <stack>
#include <string>

#include "HwBackupException.hpp"

namespace HwBackup {
namespace FileTree {
struct DirWrapper {
  struct DirDeleter {
    void operator()(DIR* dir);
  };

  using DirT = std::unique_ptr<DIR, DirDeleter>;

  static DirT CreateDir(const std::string& path);
};

using FuncT = std::function<void(const std::string&, const std::string&)>;

void DFS(const std::string& rootPath, FuncT func);

}  // namespace FileTree
}  // namespace HwBackup