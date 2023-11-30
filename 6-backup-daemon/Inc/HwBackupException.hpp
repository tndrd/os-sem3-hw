#pragma once
#include <string.h>

#include <stdexcept>
#include <string>

namespace HwBackup {

using std::string_literals::operator""s;

class Exception : public std::exception {
 private:
  std::string Message;
  const char* File;
  const char* Function;
  const char* Line;

  mutable std::string Buf;

 public:
  Exception(const std::string& message, const char* file, const char* function,
            const char* line);

  virtual const char* what() const noexcept override;
  virtual ~Exception() = default;
};

#define XSTRINGIFY(a) STRINGIFY(a)
#define STRINGIFY(a) #a
#define THROW(msg) \
  throw HwBackup::Exception(msg, __FILE__, __FUNCTION__, XSTRINGIFY(__LINE__))

#define THROW_ERRNO(msg, err) THROW(msg + ": "s + strerror(errno))

}  // namespace HwBackup