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

/**
 * Notice: I am used awesome macro from phusion passanger. I have renamed
 *  psg_align_ptr macro to falloc_align_ptr.
 * Here is Phusion Passanger's license:
 *  https://github.com/phusion/passenger/blob/stable-5.0/LICENSE
 */

#ifndef FALLOC_H
#define FALLOC_H

typedef struct f_pool_block_s f_pool_block_t;
typedef struct f_pool_s f_pool_t;

struct f_pool_block_s {
  f_pool_t *owner; // owner pool
  char *alloc;
  char *last;
  char *endp;
  size_t max;
  size_t refcount;
  f_pool_block_t *next;
};

struct f_pool_s {
  size_t blocksize;
  size_t max_size;
  f_pool_block_t *first;
  f_pool_block_t *curr;
};

// Api for f_pool_block_t not open. Internal use only.

/*  Allocate new falloc pool. With block size @param blocksize
    and max size @param max_size. */
f_pool_t *f_pool_new (size_t blocksize, size_t max_size);

/*  Deallocate all resources owned by this pool.
    If @param assertions set to non-zero, then falloc asserts all allocations
    freed by 'f_pool_free'. */
void f_pool_destroy (f_pool_t *self, int assertions);

/*  Allocate memory from pool. */
void *f_pool_alloc (f_pool_t *self, size_t size);

/*  Mark memory pointer as unused. So falloc can deallocate it's block
    if it has no references. */
void f_pool_free (f_pool_t *self, void *ptr);


#endif // FALLOC_H
