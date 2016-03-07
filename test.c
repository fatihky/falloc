#include <stdio.h>
#include <assert.h>
#include "falloc.c"

int main (int argc, char *argv[])
{
  f_pool_block_t *block = f_pool_block_new (NULL, 4096);

  assert (block != NULL && "Can allocate new block");

  printf ("Block's address must be written the start of allocated buffer: ");
  assert (block == *(f_pool_block_t **) (block->alloc - (sizeof (void *))));
  printf ("PASS\n");

  f_pool_block_destroy (block, 1);

  return 0;
}
