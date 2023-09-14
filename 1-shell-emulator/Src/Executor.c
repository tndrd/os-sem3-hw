#include "Executor.h"

ShellStatus ExecutorContextInit(ExecutorContext* ctx, char** env) {
  if (!ctx || !env) return SH_BAD_ARG_PTR;

  int newPipeFd[2];

  if (pipe(newPipeFd) < 0) return SH_ERRNO_ERROR;

  ctx->Env = env;

  ctx->ReadFd = newPipeFd[0];
  ctx->WriteFd = newPipeFd[1];

  ctx->Active = 1;

  return SH_SUCCESS;
}

static int CheckInternalCmds(ExecutorContext* ctx,
                                     char** tokens) {
  assert(ctx);
  assert(tokens);

  if (strcmp(tokens[0], EXIT_CMD) == 0) {
    ctx->Active = 0;
    return 1;
  }

  return 0;
}

static ShellStatus ExecuteSingleCommand(ExecutorContext* ctx,
                                        char** tokens, int readFd,
                                        int writeFd) {
  assert(ctx);
  assert(tokens);

  if (CheckInternalCmds(ctx, tokens)) {
    return SH_SUCCESS;
  }

  pid_t pid = fork();

  if (pid < 0) return SH_ERRNO_ERROR;

  if (pid > 0) {
    int status;
    wait(&status);

    assert(WIFEXITED(status));
    ctx->ExitCode = WEXITSTATUS(status);

    if (ctx->ExitCode == 0) return SH_PROGRAM_FAILURE;
    return SH_SUCCESS;
  }

  // Child code

  dup2(readFd, STDIN_FILENO);
  dup2(writeFd, STDOUT_FILENO);

  execvpe(tokens[0], tokens, ctx->Env);  // Using p-version to search PATH

  perror("Failed to run program");  // Didn't find a better solution to
                                    // propagate errno
  exit(1);
}

static ShellStatus ClearPipe(ExecutorContext* ctx) {
  assert(ctx);
  close(ctx->ReadFd);
  close(ctx->WriteFd);

  int newPipeFd[2];

  if (pipe(newPipeFd) < 0) return SH_ERRNO_ERROR;

  ctx->ReadFd = newPipeFd[0];
  ctx->WriteFd = newPipeFd[1];

  return SH_SUCCESS;
}

ShellStatus Execute(ExecutorContext* ctx, char** tokens, int inputFd,
                    int outputFd) {
  if (!ctx || !tokens) return SH_BAD_ARG_PTR;

  size_t startToken = 0;
  int readFd = inputFd;
  int writeFd = ctx->WriteFd;

  while (tokens[startToken] != NULL) {
    size_t endToken = startToken;

    while (tokens[endToken] != NULL && tokens[endToken][0] != PIPE_SEPARATOR)
      endToken++;

    tokens[endToken] = NULL;

    if (endToken == startToken) {  // Empty command
      startToken = endToken + 1;
      continue;
    }

    if (tokens[endToken + 1] == NULL)  // Last command
      writeFd = outputFd;

    ShellStatus status =
        ExecuteSingleCommand(ctx, tokens + startToken, readFd, writeFd);

    if (status != SH_SUCCESS) {
      if (ClearPipe(ctx) != SH_SUCCESS) return SH_FAILED_TO_CLEAR_PIPE_FATAL;
      return status;
    }

    readFd = ctx->ReadFd;
    startToken = endToken + 1;
  }

  return SH_SUCCESS;
}

ShellStatus ExecutorDestroy(ExecutorContext* ctx) {
  if (!ctx) return SH_BAD_ARG_PTR;
  close(ctx->ReadFd);
  close(ctx->WriteFd);
}