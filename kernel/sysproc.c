
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"
#include "dcache.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if (t == SBRK_EAGER || n < 0) {
    if (growproc(n) < 0) {
      return -1;
    }
  } else {
 
    if (addr + n < addr)
      return -1;
    if (addr + n > TRAPFRAME)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if (n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n) {
    if (killed(myproc())) {
      release(&tickslock);
      return -1;
    }
    sleep_prepare(&ticks);
    release(&tickslock);
    sleep();
    acquire(&tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
}


uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_hello(void)
{
  printk("Syscall ejecutada por el proceso %d\n", myproc()->pid);
  return 0;
}

uint64
sys_sumados(void)
{
  int a, b;

  argint(0, &a);
  argint(1, &b);

  int suma = a + b;
  printk("Kernel: %d + %d = %d\n", a, b, suma);

  return suma;
}

uint64
sys_setpriority(void)
{
  int pid, pr;
  argint(0, &pid);
  argint(1, &pr);
  return setpriority(pid, pr);
}

uint64
sys_sem_alloc(void)
{
  char name[16];
  int initial;
  if (argstr(0, name, sizeof(name)) < 0)
    return -1;
  argint(1, &initial);
  return sem_alloc(name, initial);
}

uint64
sys_sem_free(void)
{
  int id;
  argint(0, &id);
  return sem_free(id);
}

uint64
sys_sem_wait(void)
{
  int id;
  argint(0, &id);
  return sem_wait(id);
}

uint64
sys_sem_signal(void)
{
  int id;
  argint(0, &id);
  return sem_signal(id);
}

uint64
sys_pc_reset(void)
{
  return pc_reset();
}

uint64
sys_pc_put(void)
{
  int item;
  argint(0, &item);
  return pc_put(item);
}

uint64
sys_pc_get(void)
{
  return pc_get();
}

// Obtener las estadisticas de Dentry Cache.
uint64
sys_dcachestat(void)
{
  uint64 user_addr;
  struct dcache_stat stats;
  struct proc *p = myproc();

  // Obtener la direccion de memoria del usuario.
  argaddr(0, &user_addr);

  // Obtener una copia de los contadores del kernel.
  dcache_get_stats(&stats);

  // Copiar las estadisticas al espacio de usuario.
  if (copyout(p->pagetable, p->sz, user_addr,
              (char *)&stats, sizeof(stats)) < 0)
    return -1;

  return 0;
}
