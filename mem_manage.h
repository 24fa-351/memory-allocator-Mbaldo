#ifndef MEM_MANAGE_H
#define MEM_MANAGE_H

#include <stddef.h> // for size_t

// Initialize the memory manager with a fixed block of memory
void mm_init(size_t memory_size);

// Allocate a block of memory
void* mm_malloc(size_t size);

// Free a previously allocated block of memory
void mm_free(void* ptr);

// Reallocate a previously allocated block of memory
void* mm_realloc(void* ptr, size_t size);

// Clean up the memory manager (optional for testing purposes)
void mm_cleanup();

// Get the size of metadata overhead
size_t mm_metadata_size();

#endif // MEM_MANAGE_H
