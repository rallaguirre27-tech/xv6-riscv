#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

struct service {
  char *name;
  int priority;
};

int
main(int argc, char *argv[])
{
  // Definición de los 5 servicios del caso de aplicación
  struct service srvs[5] = {{"monitor", 2},
                            {"webserver", 3},
                            {"dbupdate", 5},
                            {"analytics", 7},
                            {"backup", 8}};

  printf("=== INICIANDO SIMULACION DE SERVICIOS (Fase 7) ===\n");

  // Crear un proceso hijo para cada servicio
  for (int i = 0; i < 5; i++) {
    int pid = fork();
    if (pid < 0) {
      printf("Error al crear fork para el servicio %s\n", srvs[i].name);
      exit(1);
    }
    if (pid == 0) {
      // Código del Proceso Hijo
      setpriority(getpid(), srvs[i].priority);
      printf("[HIJO] Servicio: %s | PID: %d | Prioridad: %d iniciandose...\n",
             srvs[i].name, getpid(), srvs[i].priority);

      // Simulación de trabajo de CPU intensivo
      long long trabajo = 0;
      for (long long j = 0; j < 30000000; j++) {
        trabajo += j;
      }

      printf("[FIN] Servicio: %s (PID: %d) termino su trabajo de CPU.\n",
             srvs[i].name, getpid());
      exit(0);
    }
  }

  // El proceso Padre espera a que terminen los 5 hijos
  for (int i = 0; i < 5; i++) {
    wait(0);
  }

  printf("=== TODOS LOS SERVICIOS HAN FINALIZADO EXITOSAMENTE ===\n");
  exit(0);
}
