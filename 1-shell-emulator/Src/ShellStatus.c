#include "ShellStatus.h"

const char* GetErrorDescription(ShellStatus status) {
  switch (status) {
    case SH_SUCCESS:
      return "Success";

    case SH_BAD_ARG_PTR:
      return "Bad pointer argument";

    case SH_BAD_ALLOC:
      return "Failed to allocate memory";

    case SH_OUT_OF_RANGE:
      return "Index out of range";

    case SH_STRING_LEN_MAX_SIZE:
      return "Input string reached maximum size";

    case SH_ERRNO_ERROR:
      return strerror(errno);

    case SH_PROGRAM_FAILURE:
      return "Program return non-zero exit code";

    case SH_FAILED_TO_CLEAR_PIPE_FATAL:
      return "Fatal: failed to clear pipe";

    default:
      // assert(0 && "Unknown Error Code");
      return "Unknown Error Code";
  }
}