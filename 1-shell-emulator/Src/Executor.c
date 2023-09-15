#include "Executor.h"

ShellStatus ExecutorContextInit(ExecutorContext* ctx, char** env, int inputFd,
                                int outputFd) {
  if (!ctx || !env) return SH_BAD_ARG_PTR;

  ctx->Env = env;
  ctx->Active = 1;

  ctx->InputFd = inputFd;
  ctx->OutputFd = outputFd;

  return SH_SUCCESS;
}

static int CheckInternalCmds(ExecutorContext* ctx, char** tokens) {
  assert(ctx);
  assert(tokens);

  if (strcmp(tokens[0], EXIT_CMD) == 0) {
    ctx->Active = 0;
    return 1;
  }

  return 0;
}

static ShellStatus ExecuteSingleCommand(ExecutorContext* ctx, char** argv) {
  assert(ctx);
  assert(argv);

  if (CheckInternalCmds(ctx, argv)) {
    return SH_SUCCESS;
  }

  pid_t pid = fork();

  if (pid < 0) return SH_ERRNO_ERROR;

  if (pid > 0) {
    int status;
    wait(&status);

    assert(WIFEXITED(status));
    ctx->ExitCode = WEXITSTATUS(status);

    if (ctx->ExitCode != 0) return SH_PROGRAM_FAILURE;
    return SH_SUCCESS;
  }

  // Child code
  dup2(ctx->ReadFd, STDIN_FILENO);
  dup2(ctx->WriteFd, STDOUT_FILENO);

  execvpe(argv[0], argv, ctx->Env);  // Using p-version to search PATH

  perror("Failed to run program");  // Didn't find a better solution to
                                    // propagate errno
  exit(1);
}

ShellStatus ExecutorRedirectOutput(ExecutorContext* ctx, RedirectMode mode) {
  assert(ctx);

  switch (mode) {
    case MODE_STANDARD:
      ctx->WriteFd = ctx->OutputFd;
      break;

    case MODE_PIPELINE: {
      int pipeFd[2];
      if (pipe(pipeFd) < 0) return SH_ERRNO_ERROR;
      ctx->WriteFd = pipeFd[1];
      ctx->NextReadFd = pipeFd[0];
      break;
    }

    case MODE_FILE_INPUT:
      ctx->ReadFd = ctx->FileFd;
      break;

    case MODE_FILE_OUTPUT:
      ctx->WriteFd = ctx->FileFd;
      break;
    
    default:
      assert(0 && "Unknown enum value");
      break;
  }

  return SH_SUCCESS;
}

RedirectMode ParseRedirectMode(ExecutorContext* ctx, char* token) {
  assert(ctx);

  if (token == NULL) return MODE_STANDARD;

  switch (*token) {
    case PIPE_SEPARATOR:
      return MODE_PIPELINE;
    case IFILE_SEPARATOR:
      return MODE_FILE_INPUT;
    case OFILE_SEPARATOR:
      return MODE_FILE_OUTPUT;
    default:
      return MODE_UNKNOWN;
  }
}

ShellStatus ParseRedirectCommand(ExecutorContext* ctx, RedirectMode mode,
                                 char** tokens, size_t* endTokenPtr) {
  assert(ctx);
  assert(tokens);
  assert(endTokenPtr);
  
  assert(mode != MODE_UNKNOWN);

  switch (mode) {
    case MODE_STANDARD:
    case MODE_PIPELINE:
      break;

    case MODE_FILE_OUTPUT: {
      int newFd = open(tokens[*endTokenPtr + 1], O_CREAT | O_WRONLY);
      if (newFd < 0) return SH_ERRNO_ERROR;

      ctx->FileFd = newFd;

      (*endTokenPtr)++;
      break;
    }

    case MODE_FILE_INPUT: {
      int newFd = open(tokens[*endTokenPtr + 1], O_RDONLY);
      if (newFd < 0) return SH_ERRNO_ERROR;

      ctx->FileFd = newFd;

      (*endTokenPtr)++;
      break;
    }

    default:
      assert(0 && "Unknown enum value");
  }

  return SH_SUCCESS;
}

void ExecutorContinue(ExecutorContext* ctx) {
  assert(ctx);

  if (ctx->ReadFd != ctx->InputFd) close(ctx->ReadFd);
  if (ctx->WriteFd != ctx->OutputFd) close(ctx->WriteFd);
  
  ctx->ReadFd = ctx->NextReadFd;
}

ShellStatus Execute(ExecutorContext* ctx, char** tokens) {
  if (!ctx || !tokens) return SH_BAD_ARG_PTR;

  ctx->ReadFd = ctx->InputFd;
  size_t endToken;
  ShellStatus status;

  for (size_t startToken = 0; tokens[startToken] != NULL;
       startToken = endToken + 1) {
    endToken = startToken;
    RedirectMode mode;

    while ((mode = ParseRedirectMode(ctx, tokens[endToken])) == MODE_UNKNOWN)
      endToken++;
    
    tokens[endToken] = NULL;

    if (endToken == startToken)  // Empty command
      continue;

    status = ParseRedirectCommand(ctx, mode, tokens, &endToken);
    if (status != SH_SUCCESS) return status;

    status = ExecutorRedirectOutput(ctx, mode);
    if (status != SH_SUCCESS) return status;

    status = ExecuteSingleCommand(ctx, tokens + startToken);
    ExecutorContinue(ctx);

    if (status != SH_SUCCESS) return status;
  }

  return SH_SUCCESS;
}

ShellStatus ExecutorDestroy(ExecutorContext* ctx) {
  if (!ctx) return SH_BAD_ARG_PTR;
  return SH_SUCCESS;
}