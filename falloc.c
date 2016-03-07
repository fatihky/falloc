/*
  Copyright (c) 2016 Fatih Kaya  All rights reserved.

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom
  the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include "falloc.h"

#define F_HEADER_SIZE sizeof(unsigned long)
#define F_ALIGNMENT sizeof(unsigned long) /* platform word */

#define falloc_align_ptr(p, a) \
	(char *) (((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))

static f_pool_block_t *f_pool_block_new (f_pool_t *owner, size_t size)
{
  f_pool_block_t *self = malloc(sizeof (f_pool_block_t));

  if (self == NULL)
    return NULL;

  void *ptr = NULL;
  int rc = posix_memalign(&ptr, F_ALIGNMENT,
    size // alloc bytes
    + F_HEADER_SIZE // allow at least  one allocation with max size.
                    // so we need to allocate one header plus
    + sizeof (void *) // place block's pointer at beginning of alloc
                      // so we can easily access block of allocated bytes
    );

  if (rc != 0 || ptr == NULL) {
    free (self);
    return NULL;
  }

  memcpy (ptr, &self, sizeof (void *));

  self->owner = owner;
  self->alloc = (char *) ptr + sizeof (void *);
  self->last = self->alloc;
  self->endp = self->alloc + size + F_HEADER_SIZE;
  self->max = size;
  self->refcount = 0;
  self->next = NULL;

  return self;
}

static void f_pool_block_destroy (f_pool_block_t *self, int assertions)
{
  if (assertions)
    assert (self->refcount == 0);

  free (self->alloc - sizeof (void *));
  free (self);
}

static void *f_pool_block_alloc (f_pool_block_t *self, size_t size)
{
  size_t can_alloc = self->endp - self->last - F_HEADER_SIZE;

  if (size > can_alloc)
    return NULL;

  char *tmp = self->last;
  self->last = falloc_align_ptr(self->last, F_ALIGNMENT);
  size_t diff = (size_t) (self->last - tmp);

  if (diff > 0 && (can_alloc - diff) < size) {
    // reset last ptr
    self->last = tmp;
    return NULL;
  }

  // write offset at the beginning of the return block
  unsigned long offset = (unsigned long) (self->last - self->alloc);

  memcpy(self->last, &offset, sizeof (unsigned long));

  // move last pointer by F_HEADER_SIZE
  self->last += F_HEADER_SIZE; // sizeof(unsigned long)

  // store return value inside tmp variable
  tmp = self->last;

  // move last pointer
  self->last += size;

  // increase refcount
  self->refcount++;

  return tmp;
}

static size_t f_pool_block_can_alloc (f_pool_block_t *self)
{
  char *tmp = self->last;

  tmp = falloc_align_ptr(tmp, F_ALIGNMENT);

  return (size_t) (self->endp - tmp - F_HEADER_SIZE);
}

static f_pool_block_t *f_pool_block_free (void *ptr)
{
  f_pool_block_t *self = NULL;
  char *alloc = NULL;
  unsigned long offset = *((unsigned long *)ptr - 1);
  // printf ("offset: %lu\n", offset);
  // assert (self->alloc == ((char *)ptr - F_HEADER_SIZE - offset));
  // assert (self == *(f_pool_block_t **)((char *)self->alloc - sizeof (void *)));
  alloc = ((char *)ptr - F_HEADER_SIZE - offset);
  self = *(f_pool_block_t **)(alloc - sizeof (void *));
  // decrease refcount
  self->refcount--;
  return self;
}

f_pool_t *f_pool_new (size_t blocksize, size_t max_size)
{
  f_pool_t *self = malloc(sizeof (f_pool_t));

  if (self == NULL)
    return NULL;

  self->blocksize = blocksize;
  self->max_size = max_size;
  self->first = NULL;
  self->curr = NULL;

  return self;
}

void f_pool_destroy (f_pool_t *self, int assertions)
{
  f_pool_block_t *block = self->first;

  while (block != NULL) {
    f_pool_block_t *tmp = block->next;
    f_pool_block_destroy (block, assertions);
    block = tmp;
  }

  free (self);
}

static void *f_pool_add_block_and_alloc (f_pool_t *self, size_t size)
{
  f_pool_block_t *curr = f_pool_block_new (self, self->blocksize);

  if (curr == NULL)
    return NULL;

  if (self->curr)
    self->curr->next = curr;

  // if this block has no allocated blocks, add this
  // block to start of the pool
  if (!self->first)
    self->first = curr;

  self->curr = curr;

  return f_pool_block_alloc (curr, size);
}

void *f_pool_alloc (f_pool_t *self, size_t size)
{
  f_pool_block_t *curr = self->curr;

  if (curr == NULL || size > f_pool_block_can_alloc (curr))
    return f_pool_add_block_and_alloc (self, size);

  return f_pool_block_alloc (curr, size);
}

void f_pool_free (f_pool_t *self, void *ptr)
{
  f_pool_block_t *block = f_pool_block_free (ptr);

  if (block->refcount > 0)
    return;

  f_pool_block_t *tmp;
  f_pool_block_t *prev = NULL;

  for (tmp = self->first; tmp; prev = tmp, tmp = tmp->next) {
    if (tmp != block)
      continue;

    if (self->first == block)
      self->first = NULL;

    if (self->curr == block)
      self->curr = NULL;

    if (prev)
      prev->next = tmp->next;

    f_pool_block_destroy(block, 0);
    break;
  }
}
