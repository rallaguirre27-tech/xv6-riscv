// dcachetest.c - Pruebas funcionales de Dentry Cache (xv6-riscv)
// Uso: dcachetest
// Compilar con -DHAVE_DCACHE_STATS si el integrante 4 ya expuso dcachestat().
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#ifdef HAVE_DCACHE_STATS
#include "kernel/dcache.h"   // ASUMIDO: struct dcache_stat { hits, misses, neg_hits, invalidations }
#endif

static int passed, failed;

static void check(char *name, int cond)
{
  if(cond){ passed++; printf("[PASS] %s\n", name); }
  else    { failed++; printf("[FAIL] %s\n", name); }
}

static int create(char *p)
{
  int fd = open(p, O_CREATE | O_RDWR);
  if(fd < 0) return -1;
  write(fd, "x", 1);
  close(fd);
  return 0;
}

static int exists(char *p)
{
  int fd = open(p, O_RDONLY);
  if(fd < 0) return 0;
  close(fd);
  return 1;
}

static void mkname(char *buf, char *prefix, int n)
{
  char tmp[12];
  int k = 0, j = 0;
  while(*prefix) buf[j++] = *prefix++;
  if(n == 0) tmp[k++] = '0';
  while(n > 0){ tmp[k++] = '0' + n % 10; n /= 10; }
  while(k > 0) buf[j++] = tmp[--k];
  buf[j] = 0;
}

// T1: busquedas positivas repetidas
static void test_positive(void)
{
  int i, ok = 1;
  create("/dt/a");
  for(i = 0; i < 200; i++)
    if(!exists("/dt/a")){ ok = 0; break; }
  check("T1 busqueda positiva repetida", ok);
}

// T2: busquedas negativas repetidas
static void test_negative(void)
{
  int i, ok = 1;
  for(i = 0; i < 200; i++)
    if(exists("/dt/no_existe")){ ok = 0; break; }
  check("T2 busqueda negativa repetida", ok);
}

// T3: cache negativa debe invalidarse al crear el archivo
static void test_neg_then_create(void)
{
  int before = exists("/dt/nuevo");   // genera entrada negativa
  create("/dt/nuevo");
  check("T3 negativo -> create -> existe", before == 0 && exists("/dt/nuevo"));
}

// T4: unlink debe invalidar la entrada positiva
static void test_unlink(void)
{
  create("/dt/borrar");
  int a = exists("/dt/borrar");       // cachea positivo
  unlink("/dt/borrar");
  check("T4 positivo -> unlink -> no existe", a == 1 && !exists("/dt/borrar"));
}

// T5: enlaces (link)
static void test_link(void)
{
  create("/dt/orig");
  int r = link("/dt/orig", "/dt/enlace");
  int a = exists("/dt/enlace");
  unlink("/dt/orig");
  int b = exists("/dt/enlace");       // el enlace debe seguir vivo
  int c = exists("/dt/orig");         // el original no
  unlink("/dt/enlace");
  int d = exists("/dt/enlace");
  check("T5 link/unlink consistente", r == 0 && a && b && !c && !d);
}

// T6: directorios anidados y rutas relativas
static void test_nested(void)
{
  int ok;
  mkdir("/dt/d1");
  mkdir("/dt/d1/d2");
  create("/dt/d1/d2/f");
  ok = exists("/dt/d1/d2/f");
  chdir("/dt/d1");
  ok = ok && exists("d2/f") && exists("../d1/d2/f");
  chdir("/");
  unlink("/dt/d1/d2/f");
  ok = ok && !exists("/dt/d1/d2/f");
  unlink("/dt/d1/d2");
  unlink("/dt/d1");
  check("T6 rutas anidadas y relativas", ok);
}

// T7: recrear un archivo con el mismo nombre
static void test_recreate(void)
{
  int i, ok = 1;
  for(i = 0; i < 20; i++){
    create("/dt/ciclo");
    if(!exists("/dt/ciclo")) ok = 0;
    unlink("/dt/ciclo");
    if(exists("/dt/ciclo")) ok = 0;
  }
  check("T7 crear/borrar/crear (20 ciclos)", ok);
}

// T8: varios procesos concurrentes
#define NPROC 4
static void test_concurrent(void)
{
  int p, bad = 0, st;
  for(p = 0; p < NPROC; p++){
    if(fork() == 0){
      char name[32];
      int i, err = 0;
      for(i = 0; i < 50; i++){
        mkname(name, "/dt/c", p * 100 + i);
        if(create(name) < 0) err = 1;
        if(!exists(name)) err = 1;
        if(!exists("/dt/a")) err = 1;          // lectura compartida
        if(exists("/dt/fantasma")) err = 1;    // negativa compartida
        unlink(name);
        if(exists(name)) err = 1;
      }
      exit(err);
    }
  }
  for(p = 0; p < NPROC; p++){
    wait(&st);
    if(st != 0) bad++;
  }
  check("T8 concurrencia (4 procesos)", bad == 0);
}

#ifdef HAVE_DCACHE_STATS
// T9: las estadisticas deben reflejar el uso de la cache
static void test_stats(void)
{
  struct dcache_stat s0, s1;
  int i;
  create("/dt/st");
  exists("/dt/st");
  dcachestat(&s0);
  for(i = 0; i < 100; i++) exists("/dt/st");
  for(i = 0; i < 100; i++) exists("/dt/st_no");
  dcachestat(&s1);
  check("T9a hits aumentan", s1.hits > s0.hits);
  check("T9b hits negativos aumentan", s1.neg_hits > s0.neg_hits);
  unlink("/dt/st");
}
#endif

int main(void)
{
  mkdir("/dt");
  printf("== dcachetest ==\n");
  test_positive();
  test_negative();
  test_neg_then_create();
  test_unlink();
  test_link();
  test_nested();
  test_recreate();
  test_concurrent();
#ifdef HAVE_DCACHE_STATS
  test_stats();
#endif
  unlink("/dt/a");
  unlink("/dt");
  printf("== Resultado: %d PASS, %d FAIL ==\n", passed, failed);
  exit(failed ? 1 : 0);
}
