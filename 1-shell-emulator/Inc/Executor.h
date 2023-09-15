#pragma once

#define _GNU_SOURCE  // For execvpe()

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ShellStatus.h"

#define PIPE_SEPARATOR '|'
#define IFILE_SEPARATOR '<'
#define OFILE_SEPARATOR '>'
#define EXIT_CMD "exit"

typedef enum {
  MODE_STANDARD,
  MODE_PIPELINE,
  MODE_FILE_INPUT,
  MODE_FILE_OUTPUT,
  MODE_UNKNOWN
} RedirectMode;

typedef struct {
  char** Env;
  int ExitCode;

  int InputFd;
  int OutputFd;

  int ReadFd;
  int WriteFd;
  int NextReadFd;

  int FileFd;

  int Active;
} ExecutorContext;

ShellStatus ExecutorContextInit(ExecutorContext* ctx, char** env, int InputFd,
                                int OutputFd);

static ShellStatus ExecuteSingleCommand(ExecutorContext* ctx, char** argc);

static int CheckInternalCmds(ExecutorContext* ctx, char** tokens);

ShellStatus Execute(ExecutorContext* ctx, char** tokens);

ShellStatus ExecutorDestroy(ExecutorContext* ctx);