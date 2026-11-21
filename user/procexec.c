#include "kernel/types.h"
#include "user/user.h"
int main(void) {
  int pid = fork();
  if (pid < 0) {
    printf("Error: fork()\n");
    exit(1);
  }
  if (pid == 0) {
    char *args[] = {"ls", 0};
    printf("Hijo PID=%d: ejecutara ls\n", getpid());
    exec("ls", args);
    printf("Error: exec() fallo\n");
    exit(1);
  }
  wait(0);
  printf("Padre PID=%d: hijo recogido\n", getpid());
  exit(0);
}
