#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  int pid1, pid2;
  
  printf("Iniciando prueba de planificador por prioridades...\n");

  pid1 = fork();
  if (pid1 < 0) {
    printf("Error en fork\n");
    exit(1);
  } else if (pid1 == 0) {
    // Proceso hijo 1: Baja prioridad (ej: prioridad 8)
    setpriority(getpid(), 8);
    printf("Hijo 1 (PID: %d) configurado con prioridad baja (8)\n", getpid());
    for (int i = 0; i < 50000000; i++) {
      // Carga de trabajo simulada
    }
    printf("Hijo 1 (PID: %d) termino su ejecucion\n", getpid());
    exit(0);
  } else {
    pid2 = fork();
    if (pid2 < 0) {
      printf("Error en fork\n");
      exit(1);
    } else if (pid2 == 0) {
      // Proceso hijo 2: Alta prioridad (ej: prioridad 2)
      setpriority(getpid(), 2);
      printf("Hijo 2 (PID: %d) configurado con prioridad alta (2)\n", getpid());
      for (int i = 0; i < 50000000; i++) {
        // Carga de trabajo simulada
      }
      printf("Hijo 2 (PID: %d) termino su ejecucion\n", getpid());
      exit(0);
    }
  }

  // Esperar a que terminen los hijos
  wait(0);
  wait(0);
  printf("Prueba de prioridades finalizada exitosamente.\n");
  exit(0);
}
