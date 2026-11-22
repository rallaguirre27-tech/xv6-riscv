#ifndef DCACHE_H
#define DCACHE_H

#ifndef __ASSEMBLER__
#include "types.h"
#endif

#define DENTRY_CACHE_SIZE 64
#define DENTRY_HASH_SIZE  16

#define DENTRY_POSITIVE 1
#define DENTRY_NEGATIVE 2

#define DCACHE_MISS 0
#define DCACHE_HIT  1

struct dentry {
  int valid;

  uint parent_dev;
  uint parent_inum;

  char name[DIRSIZ];

  int result_type;

  uint target_inum;
  uint offset;

  struct dentry *hash_next;

  struct dentry *lru_prev;
  struct dentry *lru_next;
};

struct dcache {
  struct spinlock lock;

  struct dentry entries[DENTRY_CACHE_SIZE];

  struct dentry *hash[DENTRY_HASH_SIZE];

  struct dentry *lru_head;
  struct dentry *lru_tail;

  uint hits;
  uint misses;
};





void dcacheinit(void);

int dcache_lookup(
    uint dev,
    uint parent_inum,
    char *name,
    int *result_type,
    uint *target_inum,
    uint *offset
);

void dcache_insert(
    uint dev,
    uint parent_inum,
    char *name,
    int result_type,
    uint target_inum,
    uint offset
);

void dcache_remove(
    uint dev,
    uint parent_inum,
    char *name
);








#endif
