#pragma once

#include <pthread.h>
#include <sys/time.h>
#include <time.h>

#include "TnStatus.h"

typedef enum { LOGGING_INFO, LOGGING_WARN, LOGGING_ERROR, LOGGING_QUIET } LogType;

typedef struct {
  FILE* File;
  LogType Level;
  pthread_mutex_t Mutex;
} Logger;

TnStatus LoggerInit(Logger* self, FILE* file, LogType level);
TnStatus LoggerDestroy(Logger* self);
TnStatus LoggerInfo(Logger* self, const char* prefix, const char* fmt, ...);
TnStatus LoggerWarn(Logger* self, const char* prefix, const char* fmt, ...);
TnStatus LoggerError(Logger* self, const char* prefix, const char* fmt, ...);

#define STRINGIFY(exp) #exp
#define XSTRINGIFY(exp) STRINGIFY(exp)
#define LOC_STAMP "In " __FILE__ ", line " XSTRINGIFY(__LINE__) ": "

#define LOG_ERROR(fmt, ...) LoggerError(&self->Logger, ENTITY_NAME, fmt, __VA_ARGS__)
#define LOG_INFO(fmt, ...) LoggerInfo(&self->Logger, ENTITY_NAME, fmt, __VA_ARGS__)
#define LOG_WARN(fmt, ...) LoggerWarn(&self->Logger, ENTITY_NAME, fmt, __VA_ARGS__)


static void LoggerLock(Logger* self);
static void LoggerUnlock(Logger* self);

static TnStatus LoggerPrint_v(Logger* self, LogType type, const char* prefix,
                              const char* fmt, va_list args);

static void LoggerPrintTime(Logger* self);
static const char* LogTypeToStr(LogType level);

static int LoggerHasToPrint(Logger* self, LogType type);