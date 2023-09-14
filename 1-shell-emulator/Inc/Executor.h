#pragma once

#define _GNU_SOURCE // For execvpe()

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>

#include "ShellStatus.h"

#define PIPE_SEPARATOR '|'
#define EXIT_CMD "exit"

typedef struct {
  char** Env;

  int ReadFd;
  int WriteFd;

  int ExitCode;

  int Active;
} ExecutorContext;

ShellStatus ExecutorContextInit(ExecutorContext* ctx, char** env);

static ShellStatus ExecuteSingleCommand(ExecutorContext* ctx,
                                        char** tokens, int readFd,
                                        int writeFd);

static ShellStatus ClearPipe(ExecutorContext* ctx);

static int CheckInternalCmds(ExecutorContext* ctx, char** tokens);

ShellStatus Execute(ExecutorContext* ctx, char** tokens, int inputFd,
                    int outputFd);

ShellStatus ExecutorDestroy(ExecutorContext* ctx);