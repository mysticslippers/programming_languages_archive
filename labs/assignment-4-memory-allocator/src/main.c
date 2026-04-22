#include <assert.h>
#include <stdio.h>
#include <sys/mman.h>

#include "mem.h"
#include "mem_internals.h"

static struct block_header *get_header(void *contents) {
    return (struct block_header *)((uint8_t *)contents - offsetof(struct block_header, contents));
}

static void test_simple_allocation(void) {
    void *heap = heap_init(4096);
    assert(heap != NULL);

    void *block = _malloc(1024);
    assert(block != NULL);

    _free(block);
    heap_term();

    printf("Test 1: simple allocation - PASSED\n");
}

static void test_free_one_of_many(void) {
    void *heap = heap_init(4096);
    assert(heap != NULL);

    void *a = _malloc(256);
    void *b = _malloc(256);
    void *c = _malloc(256);

    assert(a != NULL);
    assert(b != NULL);
    assert(c != NULL);

    _free(b);
    assert(get_header(b)->is_free);

    heap_term();
    printf("Test 2: free one block of many - PASSED\n");
}

static void test_free_two_of_many(void) {
    void *heap = heap_init(4096);
    assert(heap != NULL);

    void *a = _malloc(256);
    void *b = _malloc(256);
    void *c = _malloc(256);

    assert(a != NULL);
    assert(b != NULL);
    assert(c != NULL);

    _free(a);
    _free(b);

    assert(get_header(a)->is_free);
    assert(get_header(b)->is_free);

    heap_term();
    printf("Test 3: free two blocks of many - PASSED\n");
}

static void test_region_extension_contiguous(void) {
    void *heap = heap_init(REGION_MIN_SIZE);
    assert(heap != NULL);

    void *a = _malloc(REGION_MIN_SIZE / 2);
    void *b = _malloc(REGION_MIN_SIZE);
    void *c = _malloc(REGION_MIN_SIZE / 2);

    assert(a != NULL);
    assert(b != NULL);
    assert(c != NULL);

    _free(b);
    _free(a);

    void *d = _malloc(REGION_MIN_SIZE);
    assert(d != NULL);

    _free(c);
    _free(d);

    heap_term();
    printf("Test 4: heap grows contiguously - PASSED\n");
}

static void test_region_extension_non_contiguous(void) {
    void *heap = heap_init(4096);
    assert(heap != NULL);

    void *guard = mmap(
        (void *)(HEAP_START + REGION_MIN_SIZE),
        REGION_MIN_SIZE,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE,
        -1,
        0
    );

    void *a = _malloc(100000);
    void *b = _malloc(100000);

    assert(a != NULL);
    assert(b != NULL);

    _free(a);
    _free(b);

    if (guard != MAP_FAILED) {
        munmap(guard, REGION_MIN_SIZE);
    }

    heap_term();
    printf("Test 5: heap grows non-contiguously - PASSED\n");
}

int main(void) {
    test_simple_allocation();
    test_free_one_of_many();
    test_free_two_of_many();
    test_region_extension_contiguous();
    test_region_extension_non_contiguous();
    return 0;
}
