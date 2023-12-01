#include "Stage.hpp"

using namespace HwBackup;

Stage Stage::Create(FileT fileT, ChangeT changeT, const std::string& Path) {
  return {fileT, changeT, Path, time(0)};
}

std::ostream& Stage::Dump(std::ostream& os) const {
  os << "Stage [" << Path << "][";
  os << ToString(FileType) << " ";
  os << ToString(ChangeType) << "]";
#ifdef STAGE_DUMP_TIME
  os << "[" << Timestamp << "]";
#endif
  return os;
}

std::ostream& HwBackup::operator<<(std::ostream& os, const Stage& stage) {
  return stage.Dump(os);
}

const char* Stage::ToString(FileT fileT) {
  switch (fileT) {
    case FileT::Directory:
      return "DIRECTORY";
    case FileT::Regular:
      return "REGULAR";
    default:
      assert(0 && "Unexpected file type");
      return "UNEXPECTED";
  }
}

const char* Stage::ToString(ChangeT changeT) {
  switch (changeT) {
    case ChangeT::Created:
      return "CREATED";
    case ChangeT::Modified:
      return "MODIFIED";
    case ChangeT::Deleted:
      return "DELETED";
    case ChangeT::Renamed:
      return "RENAMED";
    default:
      assert(0 && "Unexpected change type");
      return "UNEXPECTED";
  }
}