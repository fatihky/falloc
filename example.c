#include <stdio.h>
#include <assert.h>
#include "falloc.h"

int main(int argc, char *argv[])
{
  f_pool_t *pool = f_pool_new (4096, 0);
  char *ptr = f_pool_alloc (pool, 20);
  char *ptr2 = f_pool_alloc (pool, 245);
  char *ptr3 = f_pool_alloc (pool, 3);

  assert (ptr != NULL);
  assert (ptr2 != NULL);
  assert (ptr3 != NULL);

  f_pool_free (pool, ptr);
  f_pool_free (pool, ptr2);
  f_pool_free (pool, ptr3);

  // f_pool_block_destroy (block);
  f_pool_destroy (pool, 0);

  return 0;
}
