#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#define MAX_SEMAPHORES 64

struct semaphore {
  int used;
  int value;
  int waiters;
  char name[16];
  struct spinlock lock;
};

#endif
