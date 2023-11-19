#include "TxDriver.h"
#include "RxDriver.h"

int main() {

  sigset_t set;
  sigfillset(&set);
  sigprocmask(SIG_BLOCK, &set, NULL);

  pid_t pid = fork();

  if (pid < 0) {
    perror("fork");
    exit(1);
  }

  if (pid == 0) { // Child
    RxDriver();
  } else { // Parent
    TxDriver(pid);
  }

  return 0;
}