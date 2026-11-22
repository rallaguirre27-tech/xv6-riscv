#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  printf("[PADRE] Solicitud iniciada por el estudiante (PID actual: %d).\n",
         getpid());

  // 1. fork(): Clona el proceso principal
  int pid = fork();

  if (pid < 0) {
    printf("[ERROR] Falló la creación del proceso hijo con fork().\n");
    exit(1);
  } else if (pid == 0) {
    // --- PROCESO HIJO ---
    printf(
      "[HIJO] Proceso procesador creado (PID: %d). Ejecutando programa secundario...\n",
      getpid());

    // 2. exec(): Reemplaza la imagen del proceso hijo cargando 'atencion'
    char *args[] = {"atencion", "Informe_Laboratorio_SO.pdf", 0};
    exec("atencion", args);

    // Si exec() retorna, ocurrió un error
    printf("[ERROR] Falló la ejecución de exec().\n");
    exit(1);
  } else {
    // --- PROCESO PADRE ---
    printf(
      "[PADRE] Proceso hijo en marcha (PID: %d). Bloqueando padre con wait()...\n",
      pid);

    // 3. wait(): Bloquea al padre hasta que el hijo termine
    int status;
    wait(&status);

    printf(
      "[PADRE] Notificación recibida: El hijo ha terminado. Proceso completo.\n");
  }

  exit(0);
}
