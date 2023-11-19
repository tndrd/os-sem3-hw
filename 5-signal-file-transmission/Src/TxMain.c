#include "TxDriver.h"

int main(int argc, char* argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Wrong arguments, expected <RX_PID>\n");
    exit(1);
  }

  pid_t rxPid = atoi(argv[1]);

  TxDriver(rxPid);
}