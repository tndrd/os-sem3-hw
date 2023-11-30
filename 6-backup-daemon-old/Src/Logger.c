#include "Logger.h"

TnStatus LoggerInit(Logger* self, FILE* file, LogType level) {
  if (!self || !file) return TNSTATUS(TN_BAD_ARG_PTR);

  self->File = file;
  self->Level = level;

  pthread_mutex_init(&self->Mutex, NULL);

  return TN_OK;
}

TnStatus LoggerDestroy(Logger* self) {
  if (!self) return TNSTATUS(TN_BAD_ARG_PTR);

  pthread_mutex_destroy(&self->Mutex);
}

static void LoggerLock(Logger* self) {
  assert(self);
  pthread_mutex_lock(&self->Mutex);
}

static void LoggerUnlock(Logger* self) {
  assert(self);
  pthread_mutex_unlock(&self->Mutex);
}

static TnStatus LoggerPrint_v(Logger* self, LogType type, const char* prefix,
                              const char* fmt, va_list args) {
  if (!self || !prefix || !fmt) return TNSTATUS(TN_BAD_ARG_PTR);
  if (!LoggerHasToPrint(self, type)) return TN_OK;

  LoggerLock(self);

  LoggerPrintTime(self);
  fprintf(self->File, "[%s][%s]: ", prefix, LogTypeToStr(type));
  vfprintf(self->File, fmt, args);
  fprintf(self->File, "\n");
  LoggerUnlock(self);

  return TN_OK;
}

static void LoggerPrintTime(Logger* self) {
  assert(self);
#define BUF_SIZE 32

  char buf[BUF_SIZE];
  time_t now = time(0);

  strftime(buf, BUF_SIZE, "%Y-%m-%d %H:%M:%S", localtime(&now));
  fprintf(self->File, "[%s]", buf);

#undef BUF_SIZE
}

static const char* LogTypeToStr(LogType level) {
  switch (level) {
    case LOGGING_ERROR:
      return "ERROR";
    case LOGGING_WARN:
      return "WARNING";
    case LOGGING_INFO:
      return "INFO";
    default:
      assert(0 && "Unexpected type");
  }
}

static int LoggerHasToPrint(Logger* self, LogType type) {
  assert(self);
  LogType level = self->Level;

  if (level == LOGGING_QUIET) return 0;

  switch (type) {
    case LOGGING_ERROR:
      return 1;
    case LOGGING_WARN:
      return level != LOGGING_ERROR;
    case LOGGING_INFO:
      return level == LOGGING_INFO;
    default:
      assert(0 && "Unexpected type");
  }
}

TnStatus LoggerInfo(Logger* self, const char* prefix, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  TnStatus result = LoggerPrint_v(self, LOGGING_INFO, prefix, fmt, args);
  va_end(args);

  return result;
}

TnStatus LoggerWarn(Logger* self, const char* prefix, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  TnStatus result = LoggerPrint_v(self, LOGGING_WARN, prefix, fmt, args);
  va_end(args);

  return result;
}

TnStatus LoggerError(Logger* self, const char* prefix, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  TnStatus result = LoggerPrint_v(self, LOGGING_ERROR, prefix, fmt, args);
  va_end(args);

  return result;
}