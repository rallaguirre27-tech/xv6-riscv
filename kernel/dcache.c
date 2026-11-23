#include "types.h"
#include "param.h"
#include "riscv.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "defs.h"
#define DCACHE_KERNEL
#include "dcache.h"

static struct dcache dcache;

static uint
dcache_hash(uint dev, uint inum, char *name)
{
  uint h = 2166136261U;

  h = (h ^ dev) * 16777619U;
  h = (h ^ inum) * 16777619U;

  for (int i = 0; i < DIRSIZ && name[i]; i++) {
    h = (h ^ (uchar)name[i]) * 16777619U;
  }

  return h % DENTRY_HASH_SIZE;
}

static int
dcache_key_match(struct dentry *de, uint parent_dev, uint parent_inum,
                 char *name)
{
  if (!de || !de->valid)
    return 0;
  if (de->parent_dev != parent_dev || de->parent_inum != parent_inum)
    return 0;
  return strncmp(de->name, name, DIRSIZ) == 0;
}

static struct dentry *
dcache_hash_lookup(uint parent_dev, uint parent_inum, char *name)
{
  uint bucket = dcache_hash(parent_dev, parent_inum, name);
  struct dentry *de = dcache.hash[bucket];

  while (de != 0) {
    if (dcache_key_match(de, parent_dev, parent_inum, name)) {
      return de;
    }
    de = de->hash_next;
  }

  return 0;
}

static void
dcache_hash_insert(struct dentry *de)
{
  uint bucket;

  if (!de)
    return;

  bucket = dcache_hash(de->parent_dev, de->parent_inum, de->name);
  de->hash_next = dcache.hash[bucket];
  dcache.hash[bucket] = de;
}

static void
dcache_hash_remove(struct dentry *de)
{
  uint bucket;
  struct dentry **curr;

  if (!de)
    return;

  bucket = dcache_hash(de->parent_dev, de->parent_inum, de->name);
  curr = &dcache.hash[bucket];

  while (*curr != 0) {
    if (*curr == de) {
      *curr = de->hash_next;
      de->hash_next = 0;
      return;
    }
    curr = &(*curr)->hash_next;
  }
}

static void
dcache_lru_remove(struct dentry *entry)
{
  if (!entry)
    return;

  if (entry->lru_prev)
    entry->lru_prev->lru_next = entry->lru_next;
  else
    dcache.lru_head = entry->lru_next;

  if (entry->lru_next)
    entry->lru_next->lru_prev = entry->lru_prev;
  else
    dcache.lru_tail = entry->lru_prev;

  entry->lru_prev = 0;
  entry->lru_next = 0;
}

static void
dcache_lru_insert_tail(struct dentry *entry)
{
  if (!entry)
    return;

  entry->lru_prev = dcache.lru_tail;
  entry->lru_next = 0;

  if (dcache.lru_tail)
    dcache.lru_tail->lru_next = entry;
  else
    dcache.lru_head = entry;

  dcache.lru_tail = entry;
}

static void
dcache_lru_touch(struct dentry *entry)
{
  if (!entry)
    return;

  if (dcache.lru_tail == entry)
    return;

  dcache_lru_remove(entry);
  dcache_lru_insert_tail(entry);
}

static struct dentry *
dcache_lru_evict(void)
{
  struct dentry *victim = dcache.lru_head;

  if (!victim)
    return 0;

  dcache_hash_remove(victim);
  dcache_lru_remove(victim);
  victim->valid = 0;

  return victim;
}

static struct dentry *
dcache_alloc_slot(void)
{
  for (int i = 0; i < DENTRY_CACHE_SIZE; i++) {
    if (!dcache.entries[i].valid) {
      return &dcache.entries[i];
    }
  }
  return dcache_lru_evict();
}

static struct dentry *
dcache_put_entry(uint parent_dev, uint parent_inum, char *name, int result_type,
                 uint target_inum, uint offset)
{
  struct dentry *de = dcache_hash_lookup(parent_dev, parent_inum, name);

  if (de != 0) {
    de->result_type = result_type;
    de->target_inum = target_inum;
    de->offset = offset;
    de->valid = 1;
    dcache_lru_touch(de);
    return de;
  }

  de = dcache_alloc_slot();
  if (!de)
    return 0;

  de->parent_dev = parent_dev;
  de->parent_inum = parent_inum;
  safestrcpy(de->name, name, DIRSIZ);
  de->result_type = result_type;
  de->target_inum = target_inum;
  de->offset = offset;
  de->valid = 1;

  dcache_hash_insert(de);
  dcache_lru_insert_tail(de);

  return de;
}

void
dcacheinit(void)
{
  initlock(&dcache.lock, "dcache");

  dcache.lru_head = 0;
  dcache.lru_tail = 0;

  dcache.hits = 0;
  dcache.misses = 0;

  for (int i = 0; i < DENTRY_HASH_SIZE; i++) {
    dcache.hash[i] = 0;
  }

  for (int i = 0; i < DENTRY_CACHE_SIZE; i++) {
    dcache.entries[i].valid = 0;
    dcache.entries[i].hash_next = 0;
    dcache.entries[i].lru_prev = 0;
    dcache.entries[i].lru_next = 0;
  }
}

int
dcache_lookup(uint dev, uint parent_inum, char *name, int *result_type,
              uint *target_inum, uint *offset)
{
  struct dentry *de;

  if (!name)
    return DCACHE_MISS;

  acquire(&dcache.lock);
  de = dcache_hash_lookup(dev, parent_inum, name);
  if (de != 0) {
    if (result_type)
      *result_type = de->result_type;
    if (target_inum)
      *target_inum = de->target_inum;
    if (offset)
      *offset = de->offset;
    dcache_lru_touch(de);
    dcache.hits++;
    release(&dcache.lock);
    return DCACHE_HIT;
  }

  dcache.misses++;
  release(&dcache.lock);
  return DCACHE_MISS;
}

void
dcache_insert(uint dev, uint parent_inum, char *name, int result_type,
              uint target_inum, uint offset)
{
  if (!name)
    return;

  acquire(&dcache.lock);
  dcache_put_entry(dev, parent_inum, name, result_type, target_inum, offset);
  release(&dcache.lock);
}

void
dcache_remove(uint dev, uint parent_inum, char *name)
{
  struct dentry *de;

  if (!name)
    return;

  acquire(&dcache.lock);
  de = dcache_hash_lookup(dev, parent_inum, name);
  if (de != 0) {
    dcache_hash_remove(de);
    dcache_lru_remove(de);
    de->valid = 0;
  }
  release(&dcache.lock);
}
