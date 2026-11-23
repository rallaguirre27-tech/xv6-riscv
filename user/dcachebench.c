// dcachebench.c - Benchmark de Dentry Cache (xv6-riscv)
// Uso: dcachebench <nfiles> <iteraciones> [nprocs]
// Ejemplo: dcachebench 100 2000 4
// Salida en formato CSV para pegar en el informe.
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#ifdef HAVE_DCACHE_STATS
#include "kernel/dcache.h"
#endif

static void mkpath(char *buf, int i)
{
  char tmp[12];
  int k = 0, j = 0;
  strcpy(buf, "/bench/f");
  j = 8;
  if(i == 0) tmp[k++] = '0';
  while(i > 0){ tmp[k++] = '0' + i % 10; i /= 10; }
  while(k > 0) buf[j++] = tmp[--k];
  buf[j] = 0;
}

static void populate(int n)
{
  char p[32];
  int i, fd;
  mkdir("/bench");
  for(i = 0; i < n; i++){
    mkpath(p, i);
    fd = open(p, O_CREATE | O_RDWR);
    if(fd >= 0) close(fd);
  }
}

static void cleanup(int n)
{
  char p[32];
  int i;
  for(i = 0; i < n; i++){ mkpath(p, i); unlink(p); }
  unlink("/bench");
}

// busquedas de un archivo existente (el ultimo: peor caso sin cache)
static void lookup_pos(int n, int iters)
{
  char p[32]; int i, fd;
  mkpath(p, n - 1);
  for(i = 0; i < iters; i++){ fd = open(p, O_RDONLY); if(fd >= 0) close(fd); }
}

// busquedas de un archivo inexistente
static void lookup_neg(int iters)
{
  int i, fd;
  for(i = 0; i < iters; i++){ fd = open("/bench/zzz_no", O_RDONLY); if(fd >= 0) close(fd); }
}

// crear + borrar
static void create_unlink(int iters)
{
  char p[32]; int i, fd;
  for(i = 0; i < iters; i++){
    mkpath(p, 100000 + (i % 50));
    fd = open(p, O_CREATE | O_RDWR);
    if(fd >= 0) close(fd);
    unlink(p);
  }
}

// ejecuta fn en np procesos y devuelve ticks totales
static int run(int np, int which, int n, int iters)
{
  int t0 = uptime(), i;
  for(i = 0; i < np; i++){
    if(fork() == 0){
      if(which == 0) lookup_pos(n, iters);
      else if(which == 1) lookup_neg(iters);
      else create_unlink(iters / 10);
      exit(0);
    }
  }
  for(i = 0; i < np; i++) wait(0);
  return uptime() - t0;
}

int main(int argc, char *argv[])
{
  int n = 100, iters = 2000, np = 1;
  if(argc > 1) n = atoi(argv[1]);
  if(argc > 2) iters = atoi(argv[2]);
  if(argc > 3) np = atoi(argv[3]);
  if(n < 1 || n > 150){ printf("nfiles debe estar entre 1 y 150 (limite de inodos)\n"); exit(1); }

  populate(n);
  printf("prueba,archivos,iteraciones,procesos,ticks\n");
  printf("busqueda_positiva,%d,%d,%d,%d\n", n, iters, np, run(np, 0, n, iters));
  printf("busqueda_negativa,%d,%d,%d,%d\n", n, iters, np, run(np, 1, n, iters));
  printf("crear_borrar,%d,%d,%d,%d\n",      n, iters / 10, np, run(np, 2, n, iters));


#ifdef HAVE_DCACHE_STATS
  struct dcache_stat s;

  if (dcachestat(&s) == 0) {
    printf("HITS,MISSES,POS_HITS,NEG_HITS,ENTRIES_SCANNED,INVALIDATIONS,EVICTIONS\n");

    printf("%d,%d,%d,%d,%d,%d,%d\n",
           (int)s.hits,
           (int)s.misses,
           (int)s.pos_hits,
           (int)s.neg_hits,
           (int)s.entries_scanned,
           (int)s.invalidations,
           (int)s.evictions);
  } else {
    printf("Error al obtener estadisticas\n");
  }
#endif
