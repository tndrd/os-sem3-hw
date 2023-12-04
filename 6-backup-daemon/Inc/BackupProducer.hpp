#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include <fstream>
#include <functional>
#include <stack>
#include <string>
#include <unordered_set>

#include "DescriptorWrapper.hpp"
#include "HwBackupException.hpp"
#include "Logger.hpp"
#include "PathTree.hpp"

#define HEADER_NAME "HEADER"
#define STAGES_NAME "STAGES"

namespace HwBackup {

class BackupProducer {
 private:
  struct FileDeleter {
    void operator()(FILE* file) { fclose(file); }
  };

  using FileType = std::unique_ptr<FILE, FileDeleter>;

  struct HeaderDataT {
    size_t NStages;
  };

  struct StatResult {
    bool Exists;
    struct stat Stat;
  };

 private:
  Logger* LoggerPtr;
  std::string DstRoot;
  std::string SrcRoot;

  FileType StagesFile;
  FileType HeaderFile;

  HeaderDataT HeaderData = {};

 public:
  BackupProducer(Logger* loggerPtr) : LoggerPtr{loggerPtr} {
    if (!loggerPtr) THROW("Logger pointer is null");
  }

  void Open(const std::string& dstPath, const std::string& srcPath) {
    /*
    auto newHeaderFile = OpenFile(dstPath + HEADER_NAME);
    auto newStagesFile = OpenFile(dstPath + STAGES_NAME);
    auto newDstRoot = dstPath;
    auto newSrcRoot = srcPath;
    auto newHeaderData = ReadHeaderFile(newHeaderFile);

    std::swap(HeaderFile, newHeaderFile);
    std::swap(StagesFile, newStagesFile);
    std::swap(DstRoot, newDstRoot);
    std::swap(SrcRoot, newSrcRoot);
    std::swap(HeaderData, newHeaderData);

    LOG_INFO(GetLogger(), "Opened backup directory at \"" << DstRoot << "\"");
    LOG_INFO(GetLogger(), "Number of stages: " << HeaderData.NStages);

    SyncHeader();
    */

    DstRoot = dstPath;
    SrcRoot = srcPath;
  }

  static FileType OpenFile(const std::string& filePath) {
    int newFd = open(filePath.c_str(), O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);
    if (newFd < 0) THROW_ERRNO("Failed to open fd", errno);

    FILE* newFile = fdopen(newFd, "a+");
    if (!newFile) THROW_ERRNO("Failed open file", errno);

    return {newFile, FileDeleter{}};
  }

  static HeaderDataT ReadHeaderFile(FileType& file) {
    size_t stageId;
    int ret = FileRead(file.get(), "%zu", &stageId);

    if (ret == EOF) {
      stageId = 0;
    } else if (ret == 0)
      THROW("Failed to read header");

    return {stageId};
  }

  void SyncHeader() const {
    int ret = ftruncate(fileno(HeaderFile.get()), 0);
    if (ret < 0) THROW_ERRNO("Failed to truncate file", errno);
    FileWrite(HeaderFile.get(), "%zu", HeaderData.NStages);
  }

  template <typename... Args>
  static int FileRead(FILE* file, const char* fmt, Args... args) {
    int ret = fscanf(file, fmt, args...);
    if (ret == EOF && ferror(file))
      THROW_ERRNO("Failed to read from file", errno);

    return ret;
  }

  template <typename... Args>
  static int FileWrite(FILE* file, const char* fmt, Args... args) {
    int ret = fprintf(file, fmt, args...);
    if (ret < 0) THROW_ERRNO("Failed to write to file", errno);

    return ret;
  }

  void Backup(PathTree& tree) {
    LOG_INFO(GetLogger(), "Backup forward...");

    tree.VisitPreOrder(
        [this](const std::string& path) { return this->BackupForward(path); });

    LOG_INFO(GetLogger(), "Backup backward...");
    tree.VisitPostOrder(
        [this](const std::string& path) { return this->BackupBackward(path); });
  }

  bool BackupForward(const std::string& path) {
    StatResult src = Stat(SrcRoot + path);
    StatResult dst = Stat(DstRoot + path);

    if (src.Exists == false && dst.Exists) {  // No longer exists
      if (S_ISREG(dst.Stat.st_mode)) {
        LOG_INFO(GetLogger(), "Removing file \"" << path << "\"");
        DeleteFile(path);
        return true;
      }
      if (S_ISDIR(dst.Stat.st_mode)) {
        LOG_INFO(GetLogger(), "Skipping dir \"" << path << "\"");
        return false;
      }
    }

    if (src.Exists && dst.Exists) {  // File modified
      if (S_ISDIR(src.Stat.st_mode) && S_ISDIR(dst.Stat.st_mode)) return true;

      if (S_ISREG(src.Stat.st_mode) && S_ISREG(dst.Stat.st_mode)) {
        LOG_INFO(GetLogger(), "Modifying file \"" << path << "\"");
        SyncFile(path);
        return true;
      }

      if (S_ISREG(src.Stat.st_mode) && S_ISDIR(dst.Stat.st_mode)) {
        LOG_INFO(GetLogger(), "Skipping dir->file  \"" << path << "\"");
        return false;
      }

      if (S_ISDIR(src.Stat.st_mode) && S_ISREG(src.Stat.st_mode)) {
        LOG_INFO(GetLogger(), "Changing file->dir \"" << path << "\"");
        DeleteFile(path);
        MakeDir(path);
        return true;
      }
    }

    if (src.Exists && dst.Exists == false) {  // File created
      if (S_ISDIR(src.Stat.st_mode)) {
        LOG_INFO(GetLogger(), "Creating dir \"" << path << "\"");
        MakeDir(path);
        return true;
      }
      if (S_ISREG(src.Stat.st_mode)) {
        LOG_INFO(GetLogger(), "Creating file \"" << path << "\"");
        SyncFile(path);
        return true;
      }
    }

    return true;
  }

  bool BackupBackward(const std::string& path) {
    StatResult src = Stat(SrcRoot + path);
    StatResult dst = Stat(DstRoot + path);

    if (src.Exists == false && dst.Exists) {  // No longer exists
      if (S_ISDIR(dst.Stat.st_mode)) {
        LOG_INFO(GetLogger(), "Deleting dir \"" << path << "\"");
        DeleteDir(path);
        return true;
      }
    }

    if (src.Exists && dst.Exists) {
      if (S_ISREG(src.Stat.st_mode) && S_ISDIR(dst.Stat.st_mode)) {
        LOG_INFO(GetLogger(), "Changing dir->file \"" << path << "\"");
        DeleteDir(path);
        SyncFile(path);
        return true;
      }
    }

    return true;
  }

  void MakeDir(const std::string& path) {
    std::string dst = DstRoot + path;
    int ret = mkdir(dst.c_str(), 0777);
    if (ret < 0) THROW_ERRNO("mkdir(" + dst + "): ", errno);
  }

  void SyncFile(const std::string& path) {
    std::string dst = DstRoot + path;
    std::string src = SrcRoot + path;

    pid_t pid = fork();

    if (pid < 0) THROW_ERRNO("fork()", errno);

    if (pid == 0) {  // Child
      execlp("cp", "cp", src.c_str(), dst.c_str(), NULL);
    } else {  // Parent
      wait(NULL);
    }
  }

  void DeleteFile(const std::string& path) {
    int ret = unlink((DstRoot + path).c_str());
    if (ret < 0) THROW_ERRNO("unlink()", errno);
  }

  void DeleteDir(const std::string& path) {
    int ret = rmdir((DstRoot + path).c_str());
    if (ret < 0) THROW_ERRNO("rmdir()", errno);
  }

  Logger& GetLogger() {
    assert(LoggerPtr);
    return *LoggerPtr;
  }

  static StatResult Stat(const std::string& path) {
    struct stat st;
    int ret = stat(path.c_str(), &st);
    if (ret < 0 && errno != ENOENT) THROW_ERRNO("stat(" + path + ")", errno);
    if (ret < 0 && errno == ENOENT) return {false};

    return {true, st};
  }
};  // namespace HwBackup

class BackupRestorer {};

}  // namespace HwBackup