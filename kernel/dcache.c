#include "types.h"
#include "param.h"
#include "riscv.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "defs.h"
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
dcache_key_match(struct dentry *de, uint parent_dev, uint parent_inum, char *name)
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



