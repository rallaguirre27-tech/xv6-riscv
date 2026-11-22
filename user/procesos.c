#include "kernel/types.h"
#include "user/user.h"

int
main(void)
{
  int i, pid;
  int padre = getpid();
  printf("=== INICIO ===\n");
  printf("Padre, PID = %d\n", padre);

  for (i = 0; i < 3; i++) {
    pid = fork();
    if (pid < 0) {
      printf("Error al crear hijo %d\n", i + 1);
      break;
    }
    if (pid == 0) {
      printf("Hijo %d inicia: PID= %d\n", i + 1, getpid());

      // Bucle de retardo en lugar de sleep(50)
      for (volatile int j = 0; j < 10000000; j++) {
      }

      printf("Hijo %d termina: PID= %d\n", i + 1, getpid());
      exit(0);
    }
  }

  while (wait(0) >= 0) {
  }
  printf("Padre %d: todos los hijos recogidos\n", padre);
  exit(0);
}
