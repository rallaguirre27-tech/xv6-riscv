#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define BUFSIZE        4
#define TOTAL_ITEMS    12
#define NUM_PRODUCERS  2
#define NUM_CONSUMERS  2
#define ITEMS_PER_PROD (TOTAL_ITEMS / NUM_PRODUCERS)
#define ITEMS_PER_CONS (TOTAL_ITEMS / NUM_CONSUMERS)

int
main(int argc, char *argv[])
{
  int sync_mode = 1;
  if (argc > 1) {
    if (strcmp(argv[1], "nosync") == 0)
      sync_mode = 0;
    else if (strcmp(argv[1], "sync") == 0)
      sync_mode = 1;
    else {
      printf("Uso: producer_consumer [sync|nosync]\n");
      exit(1);
    }
  }

  pc_reset();

  int empty = -1, full = -1, mutex = -1;
  if (sync_mode) {

    empty = sem_alloc("empty", BUFSIZE);
    full = sem_alloc("full", 0);
    mutex = sem_alloc("mutex", 1);
    if (empty < 0 || full < 0 || mutex < 0) {
      printf("Error al crear semaforos\n");
      exit(1);
    }
  }

  int pfd[2];
  if (pipe(pfd) < 0) {
    printf("Error en pipe\n");
    exit(1);
  }

  // Productores
  for (int p = 0; p < NUM_PRODUCERS; p++) {
    int pid = fork();
    if (pid < 0) {
      printf("fork productor fallo\n");
      exit(1);
    }
    if (pid == 0) {
      close(pfd[0]);
      int start = p * ITEMS_PER_PROD;
      for (int k = 0; k < ITEMS_PER_PROD; k++) {
        int val = start + k;
        if (sync_mode) {
          sem_wait(mutex);
          sem_wait(empty);
        }
        pc_put(val);
        if (sync_mode) {
          sem_signal(mutex);
          sem_signal(full);
        }
      }
      exit(0);
    }
  }

  // Consumidores
  for (int c = 0; c < NUM_CONSUMERS; c++) {
    int pid = fork();
    if (pid < 0) {
      printf("fork consumidor fallo\n");
      exit(1);
    }
    if (pid == 0) {
      close(pfd[0]);
      for (int k = 0; k < ITEMS_PER_CONS; k++) {
        if (sync_mode) {
          sem_wait(full);
          sem_wait(mutex);
        }
        int val = pc_get();
        if (sync_mode) {
          sem_signal(mutex);
          sem_signal(empty);
        }
        write(pfd[1], &val, sizeof(val));
      }
      close(pfd[1]);
      exit(0);
    }
  }

  // Padre: recolecta y valida
  close(pfd[1]);
  int seen[TOTAL_ITEMS];
  for (int i = 0; i < TOTAL_ITEMS; i++)
    seen[i] = 0;

  int received = 0;
  int errors = 0;
  int val;
  while (read(pfd[0], &val, sizeof(val)) == sizeof(val)) {
    received++;
    printf("Consumido: %d\n", val);
    if (val < 0 || val >= TOTAL_ITEMS) {
      errors++;
    } else {
      if (seen[val] > 0)
        errors++;
      seen[val]++;
    }
  }
  close(pfd[0]);

  int missing = 0;
  for (int i = 0; i < TOTAL_ITEMS; i++) {
    if (seen[i] == 0)
      missing++;
  }

  for (int i = 0; i < NUM_PRODUCERS + NUM_CONSUMERS; i++)
    wait(0);

  if (sync_mode) {
    sem_free(empty);
    sem_free(full);
    sem_free(mutex);
  }

  printf("Recibidos = %d Errores = %d Faltantes = %d\n", received, errors,
         missing);
  exit(0);
}
