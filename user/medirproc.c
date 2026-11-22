#include "kernel/types.h"
#include "user/user.h"
int
main(int argc, char *argv[])
{
  int n = 1;
  if (argc > 1)
    n = atoi(argv[1]);
  if (n < 1 || n > 10) {
    printf("Uso: medirproc [1..10]\n");
    exit(1);
  }
  int inicio = uptime();
  int creados = 0;
  for (int i = 0; i < n; i++) {
    int pid = fork();
    if (pid < 0) {
      printf("No se pudo crear otro hijo\n");
      break;
    }
    if (pid == 0) {
      volatile unsigned long total = 0;
      for (int j = 0; j < 5000000; j++)
        total += j % 7;
      exit(0);
    }
    creados++;
  }
  for (int i = 0; i < creados; i++)
    wait(0);
  int fin = uptime();
  printf("Hijos=%d | Duracion=%d ticks\n", creados, fin - inicio);
  exit(0);
}
