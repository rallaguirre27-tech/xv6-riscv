#include "kernel/types.h"
#include "user/user.h"
#define NUM_HIJOS 5
int main(void) {
  int i, pid;
  printf("Padre PID=%d\n", getpid());
  for (i = 0; i < NUM_HIJOS; i++) {
    pid = fork();
    if (pid < 0) {
      printf("Error al crear hijo\n");
      break;
    }
    if (pid == 0) {
      volatile unsigned long contador = 0;
      int j;
      printf("Hijo %d INICIA, PID=%d\n", i + 1, getpid());
      for (j = 0; j < 5000000; j++) {
        contador += (unsigned long) (j % 7);
      }
      printf("Hijo %d TERMINA, PID=%d\n", i + 1, getpid());
      exit(0);
    }
  }
  while (wait(0) >= 0) { }
  printf("Padre: finalizo la espera\n");
  exit(0);
}
