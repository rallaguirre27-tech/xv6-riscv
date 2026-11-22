#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "semaphore.h"

static struct semaphore sems[MAX_SEMAPHORES];

void
sem_init_all(void)
{
  for (int i = 0; i < MAX_SEMAPHORES; i++) {
    sems[i].used = 0;
    sems[i].value = 0;
    sems[i].waiters = 0;
    initlock(&sems[i].lock, "sem");
  }
}

int
sem_alloc(const char *name, int initial)
{
  if (initial < 0)
    return -1;

  for (int i = 0; i < MAX_SEMAPHORES; i++) {
    struct semaphore *s = &sems[i];
    acquire(&s->lock);
    if (!s->used) {
      s->used = 1;
      s->value = initial;
      s->waiters = 0;
      safestrcpy(s->name, name, sizeof(s->name));
      release(&s->lock);
      return i;
    }
    release(&s->lock);
  }
  return -1;
}

int
sem_free(int id)
{
  if (id < 0 || id >= MAX_SEMAPHORES)
    return -1;

  struct semaphore *s = &sems[id];
  acquire(&s->lock);
  if (!s->used || s->waiters > 0) {
    release(&s->lock);
    return -1;
  }
  s->used = 0;
  s->value = 0;
  s->name[0] = 0;
  release(&s->lock);
  return 0;
}

int
sem_wait(int id)
{
  if (id < 0 || id >= MAX_SEMAPHORES)
    return -1;

  struct semaphore *s = &sems[id];
  acquire(&s->lock);
  if (!s->used) {
    release(&s->lock);
    return -1;
  }
  while (s->value == 0) {
    if (killed(myproc())) {
      release(&s->lock);
      return -1;
    }
    s->waiters++;
    sleep_prepare(s);
    release(&s->lock);
    sleep();
    acquire(&s->lock);
    s->waiters--;
  }
  if (killed(myproc())) {
    release(&s->lock);
    return -1;
  }
  s->value--;
  release(&s->lock);
  return 0;
}

int
sem_signal(int id)
{
  if (id < 0 || id >= MAX_SEMAPHORES)
    return -1;

  struct semaphore *s = &sems[id];
  acquire(&s->lock);
  if (!s->used || s->value >= 2147483647) {
    release(&s->lock);
    return -1;
  }
  s->value++;
  wakeup(s);
  release(&s->lock);
  return 0;
}

#define BUFSIZE 4
static int slots[BUFSIZE], in, out;
static struct spinlock pc_lock;

void
pc_init(void)
{
  initlock(&pc_lock, "pc");
}

int
pc_reset(void)
{
  acquire(&pc_lock);
  in = out = 0;
  for (int i = 0; i < BUFSIZE; i++)
    slots[i] = -1;
  release(&pc_lock);
  return 0;
}

int
pc_put(int item)
{
  acquire(&pc_lock);
  int pos = in;
  release(&pc_lock);

  yield();

  acquire(&pc_lock);
  slots[pos] = item;
  in = (pos + 1) % BUFSIZE;
  release(&pc_lock);
  return 0;
}

int
pc_get(void)
{
  acquire(&pc_lock);
  int pos = out;
  release(&pc_lock);

  yield();

  acquire(&pc_lock);
  int item = slots[pos];
  slots[pos] = -1;
  out = (pos + 1) % BUFSIZE;
  release(&pc_lock);
  return item;
}
