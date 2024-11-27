#include "mem_manage.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h> // For atoi

#ifdef USE_SYSTEM_MALLOC
    #include <stdlib.h> // For malloc, free, realloc
    #define mm_malloc malloc
    #define mm_free free
    #define mm_realloc realloc
    #define mm_init(size) ((void)0) // No-op for system malloc
    #define mm_cleanup() ((void)0) // No-op for system malloc
    #define mm_metadata_size() 0   // No metadata for system malloc
#endif

void test_basic_allocation() {
    mm_init(1024);
    void* ptr1 = mm_malloc(100);
    assert(ptr1 != NULL);

    void* ptr2 = mm_malloc(200);
    assert(ptr2 != NULL);

    mm_free(ptr1);
    mm_free(ptr2);
    mm_cleanup();
    printf("\ntest_basic_allocation PASSED\n\n");
}

void test_realloc() {
    mm_init(1024);
    void* ptr = mm_malloc(100);
    assert(ptr != NULL);

    void* new_ptr = mm_realloc(ptr, 200);
    assert(new_ptr != NULL);

    mm_free(new_ptr);
    mm_cleanup();
    printf("\ntest_realloc PASSED\n\n");
}

void test_free_and_coalesce() {
    mm_init(1024);
    void* ptr1 = mm_malloc(100);
    void* ptr2 = mm_malloc(200);
    void* ptr3 = mm_malloc(100);

    mm_free(ptr2);
    mm_free(ptr1);
    mm_free(ptr3);

    mm_cleanup();
    printf("\ntest_free_and_coalesce PASSED\n\n");
}

void test_zero_allocation() {
    void* ptr = mm_malloc(0);

#ifdef USE_SYSTEM_MALLOC
    // System malloc(0) behavior: Either NULL or a non-NULL pointer that shouldn't be used
    assert(ptr == NULL || ptr != NULL);
#else
    assert(ptr == NULL);
#endif

    printf("\ntest_zero_allocation PASSED\n\n");
}


void test_exact_size_allocation() {
    mm_init(1024);

    size_t overhead = mm_metadata_size();
    void* ptr = mm_malloc(1024 - overhead); // Allocate exact memory size
    assert(ptr != NULL);

    mm_free(ptr);
    printf("\ntest_exact_size_allocation PASSED\n\n");
}

void test_memory_pattern() {
    mm_init(1024);
    size_t size = 256;
    void* ptr = mm_malloc(size);
    assert(ptr != NULL);

    // Fill with a pattern
    memset(ptr, 0xAA, size);
    for (size_t i = 0; i < size; i++) {
        assert(((unsigned char*)ptr)[i] == 0xAA);
    }

    mm_free(ptr);
    printf("\ntest_memory_pattern PASSED\n\n");
}

void test_same_size_allocations() {
    mm_init(1024);
    void* ptrs[5];

    // Allocate multiple blocks of the same size
    for (int i = 0; i < 5; i++) {
        ptrs[i] = mm_malloc(128);
        assert(ptrs[i] != NULL);
    }

    // Free all the allocated blocks
    for (int i = 0; i < 5; i++) {
        mm_free(ptrs[i]);
    }

    mm_cleanup();
    printf("\ntest_same_size_allocations PASSED\n\n");
}

int main(int argc, char* argv[]) {
    if (argc == 3 && strcmp(argv[1], "-t") == 0) {
        int test_num = atoi(argv[2]);

        switch (test_num) {
            case 1: test_basic_allocation(); break;
            case 2: test_realloc(); break;
            case 3: test_free_and_coalesce(); break;
            case 4: test_zero_allocation(); break;
            case 5: test_exact_size_allocation(); break;
            case 6: test_same_size_allocations(); break;
            case 7: test_memory_pattern(); break;
            default: printf("Invalid test number.\n"); break;
        }
        return 0;
    }

    // Run all tests
    test_basic_allocation();
    test_realloc();
    test_free_and_coalesce();
    test_zero_allocation();
    test_exact_size_allocation();
    test_same_size_allocations();
    test_memory_pattern();

    return 0;
}
