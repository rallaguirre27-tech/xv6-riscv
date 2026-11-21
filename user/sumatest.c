#include "kernel/types.h"
#include "user/user.h"

int
main(void)
{
  int resultado = sumados(10, 20);
  printf("Usuario: resultado = %d\n", resultado);
  exit(0);
}
