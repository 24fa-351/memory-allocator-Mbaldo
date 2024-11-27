#include "mem_manage.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define MAX_HEAP_SIZE 1024 // Maximum number of free blocks in the heap

typedef struct Block {
    size_t size;         // Size of the block
    bool is_free;        // Whether the block is free
    struct Block* next;  // Pointer to the next block (for splitting, etc.)
} Block;

static void* heap_start = NULL;   // Starting address of the managed heap
static size_t heap_size = 0;      // Total size of the managed heap
static Block* heap[MAX_HEAP_SIZE]; // Min-heap to track free blocks
static size_t heap_count = 0;     // Number of blocks in the heap

// Align size to 8 bytes
#define ALIGN(size) (((size) + 7) & ~7)

// Minimum block size to store metadata
#define MIN_BLOCK_SIZE (sizeof(Block))

/**
 * Helper function to swap two blocks in the heap.
 */
static void swap(size_t i, size_t j) {
    Block* temp = heap[i];
    heap[i] = heap[j];
    heap[j] = temp;
}

/**
 * Min-Heap: Percolate up to maintain heap property.
 */
static void heapify_up(size_t index) {
    while (index > 0) {
        size_t parent = (index - 1) / 2;
        if (heap[index]->size < heap[parent]->size) {
            swap(index, parent);
            index = parent;
        } else {
            break;
        }
    }
}

/**
 * Min-Heap: Percolate down to maintain heap property.
 */
static void heapify_down(size_t index) {
    size_t smallest = index;
    size_t left = 2 * index + 1;
    size_t right = 2 * index + 2;

    if (left < heap_count && heap[left]->size < heap[smallest]->size)
        smallest = left;

    if (right < heap_count && heap[right]->size < heap[smallest]->size)
        smallest = right;

    if (smallest != index) {
        swap(index, smallest);
        heapify_down(smallest);
    }
}

/**
 * Insert a block into the min-heap.
 */
static void heap_insert(Block* block) {
    if (heap_count >= MAX_HEAP_SIZE) {
        printf("Error: Free block heap overflow. Cannot insert block.\n");
        return;
    }

    heap[heap_count] = block;
    printf("Inserting block into heap: Address %p, Size %zu\n", (void*)block, block->size);
    heapify_up(heap_count);
    heap_count++;
}


/**
 * Extract the smallest block from the min-heap.
 */
static Block* heap_extract_min() {
    if (heap_count == 0) {
        printf("Heap is empty. No blocks available for allocation.\n");
        return NULL;
    }

    Block* min_block = heap[0];
    printf("Extracting smallest block from heap: Address %p, Size %zu\n", (void*)min_block, min_block->size);
    heap[0] = heap[--heap_count];
    heapify_down(0);
    return min_block;
}


/**
 * Initialize the memory manager with a fixed block of memory.
 */
void mm_init(size_t memory_size) {
    if (memory_size < MIN_BLOCK_SIZE) {
        fprintf(stderr, "Error: Memory size too small for initialization.\n");
        return;
    }

    heap_start = sbrk(memory_size);
    if (heap_start == (void*)-1) {
        fprintf(stderr, "Error: Unable to allocate memory using sbrk.\n");
        return;
    }

    heap_size = memory_size;
    Block* initial_block = (Block*)heap_start;
    initial_block->size = memory_size - sizeof(Block);
    initial_block->is_free = true;
    initial_block->next = NULL;

    heap_insert(initial_block); // Add the initial block to the heap
}

/**
 * Allocate a block of memory.
 */
void* mm_malloc(size_t size) {
    if (size == 0) {
        printf("Requested allocation size is 0. Returning NULL.\n");
        return NULL;
    }

    size = ALIGN(size);
    printf("Requested allocation of size %zu (aligned to %zu).\n", size, size);

    Block* block = heap_extract_min(); // Get the smallest suitable block
    if (!block || block->size < size) {
        printf("Allocation failed. Not enough memory available.\n");
        return NULL;
    }

    // Split the block if it's large enough
    if (block->size >= size + sizeof(Block) + ALIGN(1)) {
        Block* new_block = (Block*)((char*)block + sizeof(Block) + size);
        new_block->size = block->size - size - sizeof(Block);
        new_block->is_free = true;

        printf("Splitting block: Allocated size %zu, Remaining size %zu\n", size, new_block->size);
        heap_insert(new_block); // Return the remaining part to the heap
        block->size = size;
    }

    block->is_free = false;
    memset((char*)block + sizeof(Block), 0, size);
    printf("Allocation successful: Block at %p, Size %zu\n", (void*)block, block->size);
    return (char*)block + sizeof(Block);
}


/**
 * Free a previously allocated block of memory.
 */
void mm_free(void* ptr) {
    if (!ptr || ptr < heap_start || ptr >= (void*)((char*)heap_start + heap_size)) {
        printf("Invalid pointer passed to mm_free: %p\n", ptr);
        return;
    }

    Block* block = (Block*)((char*)ptr - sizeof(Block));
    block->is_free = true;

    printf("Freeing block at address %p, Size %zu\n", (void*)block, block->size);

    // Insert the freed block back into the heap
    heap_insert(block);

    // Coalesce adjacent free blocks
    for (size_t i = 0; i < heap_count; i++) {
        Block* current = heap[i];
        if (current->is_free) {
            Block* next = (Block*)((char*)current + sizeof(Block) + current->size);
            if ((void*)next < (void*)((char*)heap_start + heap_size) && next->is_free) {
                printf("Coalescing blocks: Current block %p (Size %zu) with Next block %p (Size %zu)\n",
                       (void*)current, current->size, (void*)next, next->size);

                current->size += sizeof(Block) + next->size;
                heap[i] = heap[--heap_count]; // Remove the merged block
                heapify_down(i);
                i = -1; // Restart coalescing
            }
        }
    }
}


/**
 * Reallocate a previously allocated block of memory.
 */
void* mm_realloc(void* ptr, size_t size) {
    if (!ptr) return mm_malloc(size);
    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }

    Block* block = (Block*)((char*)ptr - sizeof(Block));
    if (block->size >= size) return ptr;

    void* new_ptr = mm_malloc(size);
    if (new_ptr) {
        memcpy(new_ptr, ptr, block->size);
        mm_free(ptr);
    }

    return new_ptr;
}

size_t mm_metadata_size() {
    return sizeof(Block);
}


/**
 * Clean up the memory manager (optional for testing purposes).
 */
void mm_cleanup() {
    heap_start = NULL;
    heap_size = 0;
    heap_count = 0;
}
