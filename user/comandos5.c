#include "kernel/types.h"
#include "user/user.h"

int
main(void)
{
  int pid;

  // Hijo 1: Ejecuta 'ls'
  if ((pid = fork()) == 0) {
    char *args[] = {"ls", 0};
    printf("--- Hijo 1 (PID=%d) ejecuta: ls ---\n", getpid());
    exec("ls", args);
    printf("Error: exec fallo para ls\n");
    exit(1);
  }

  // Hijo 2: Ejecuta 'echo'
  if ((pid = fork()) == 0) {
    char *args[] = {"echo", "Hola_desde_xv6_hijo2", 0};
    printf("--- Hijo 2 (PID=%d) ejecuta: echo ---\n", getpid());
    exec("echo", args);
    printf("Error: exec fallo para echo\n");
    exit(1);
  }

  // Hijo 3: Ejecuta 'cat' sobre el archivo README
  if ((pid = fork()) == 0) {
    char *args[] = {"cat", "README", 0};
    printf("--- Hijo 3 (PID=%d) ejecuta: cat README ---\n", getpid());
    exec("cat", args);
    printf("Error: exec fallo para cat\n");
    exit(1);
  }

  // Hijo 4: Ejecuta 'wc' sobre el archivo README
  if ((pid = fork()) == 0) {
    char *args[] = {"wc", "README", 0};
    printf("--- Hijo 4 (PID=%d) ejecuta: wc README ---\n", getpid());
    exec("wc", args);
    printf("Error: exec fallo para wc\n");
    exit(1);
  }

  // Hijo 5: Ejecuta 'uptime'
  if ((pid = fork()) == 0) {
    char *args[] = {"uptime", 0};
    printf("--- Hijo 5 (PID=%d) ejecuta: uptime ---\n", getpid());
    exec("uptime", args);
    printf("Error: exec fallo para uptime\n");
    exit(1);
  }

  // El proceso padre espera a que terminen los 5 hijos recolectando sus recursos
  for (int i = 0; i < 5; i++) {
    wait(0);
  }

  printf("Padre (PID=%d): Todos los hijos con comandos han finalizado.\n",
         getpid());
  exit(0);
}
