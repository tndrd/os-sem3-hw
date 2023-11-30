#include "Stage.hpp"

using namespace HwBackup;

Stage Stage::CreateStage(FileT fileT, ChangeT changeT,
                         const std::string& Path) {
  return {fileT, changeT, Path, time(0)};
}
