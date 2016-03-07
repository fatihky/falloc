falloc is a memory pool implementation and it is good for small allocations.

I have tried to make [Phusion Passanger's palloc](https://github.com/phusion/passenger/blob/stable-5.0/src/cxx_supportlib/MemoryKit/) to reference counted,
but I couldn't do it. So I have written this allocator.

falloc allocates memory from blocks and writes offset of pointer before
returns it to the back. When you free allocated memory, falloc decreases
block's reference count and deallocates it if it has no references.
