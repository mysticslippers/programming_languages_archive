#define _DEFAULT_SOURCE

#include <stdio.h>
#include <unistd.h>

#include "mem.h"
#include "mem_internals.h"
#include "util.h"

void debug_block(struct block_header *b, const char *fmt, ...);
void debug(const char *fmt, ...);

extern inline block_size size_from_capacity(block_capacity cap);
extern inline block_capacity capacity_from_size(block_size sz);
extern inline bool region_is_invalid(const struct region *r);

#define BLOCK_MIN_CAPACITY 24

struct block_search_result {
    enum {
        BSR_FOUND_GOOD_BLOCK,
        BSR_REACHED_END_NOT_FOUND,
        BSR_CORRUPTED
    } type;
    struct block_header *block;
};

static size_t pages_count(size_t mem) {
    const size_t page_size = (size_t)getpagesize();
    return mem / page_size + (mem % page_size != 0);
}

static size_t round_pages(size_t mem) {
    return (size_t)getpagesize() * pages_count(mem);
}

static size_t region_actual_size(size_t query) {
    return size_max(round_pages(query), REGION_MIN_SIZE);
}

static size_t normalize_query(size_t query) {
    return query < BLOCK_MIN_CAPACITY ? BLOCK_MIN_CAPACITY : query;
}

static void block_init(void *restrict addr, block_size block_sz, void *restrict next) {
    *((struct block_header *)addr) = (struct block_header) {
        .next = next,
        .capacity = capacity_from_size(block_sz),
        .is_free = true
    };
}

static void *block_after(const struct block_header *block) {
    return (void *)(block->contents + block->capacity.bytes);
}

static struct block_header *block_get_header(void *contents) {
    return (struct block_header *)((uint8_t *)contents - offsetof(struct block_header, contents));
}

static bool block_is_big_enough(const struct block_header *block, size_t query) {
    return block->capacity.bytes >= query;
}

static bool blocks_continuous(const struct block_header *fst, const struct block_header *snd) {
    return (void *)snd == block_after(fst);
}

static bool mergeable(const struct block_header *fst, const struct block_header *snd) {
    return fst->is_free && snd->is_free && blocks_continuous(fst, snd);
}

static bool try_merge_with_next(struct block_header *block) {
    if (block == NULL || block->next == NULL) {
        return false;
    }

    if (!mergeable(block, block->next)) {
        return false;
    }

    block->capacity.bytes += size_from_capacity(block->next->capacity).bytes;
    block->next = block->next->next;
    return true;
}

static bool block_splittable(const struct block_header *block, size_t query) {
    return block->is_free &&
           query + offsetof(struct block_header, contents) + BLOCK_MIN_CAPACITY <= block->capacity.bytes;
}

static bool split_if_too_big(struct block_header *block, size_t query) {
    if (!block_splittable(block, query)) {
        return false;
    }

    void *new_block_addr = block->contents + query;
    const block_size new_block_size = {
        .bytes = block->capacity.bytes - query
    };

    block_init(new_block_addr, new_block_size, block->next);
    block->next = (struct block_header *)new_block_addr;
    block->capacity.bytes = query;

    return true;
}

static void *map_pages(void const *addr, size_t length, int additional_flags) {
    return mmap(
        (void *)addr,
        length,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS | additional_flags,
        -1,
        0
    );
}

static struct region alloc_region(void const *addr, size_t query) {
    const size_t size = region_actual_size(size_from_capacity((block_capacity){ .bytes = query }).bytes);

    void *region_addr = map_pages(addr, size, MAP_FIXED_NOREPLACE);
    if (region_addr == MAP_FAILED) {
        region_addr = map_pages(addr, size, 0);
        if (region_addr == MAP_FAILED) {
            return REGION_INVALID;
        }
    }

    block_init(region_addr, (block_size){ .bytes = size }, NULL);

    return (struct region){
        .addr = region_addr,
        .size = size,
        .extends = (region_addr == addr)
    };
}

static struct block_search_result find_good_or_last(struct block_header *block, size_t query) {
    if (block == NULL) {
        return (struct block_search_result){ .type = BSR_CORRUPTED, .block = NULL };
    }

    struct block_header *last = NULL;

    while (block != NULL) {
        while (try_merge_with_next(block)) {
        }

        if (block->is_free && block_is_big_enough(block, query)) {
            return (struct block_search_result){ .type = BSR_FOUND_GOOD_BLOCK, .block = block };
        }

        last = block;
        block = block->next;
    }

    return (struct block_search_result){ .type = BSR_REACHED_END_NOT_FOUND, .block = last };
}

static struct block_search_result try_memalloc_existing(size_t query, struct block_header *heap_start) {
    const size_t adjusted_query = normalize_query(query);
    struct block_search_result result = find_good_or_last(heap_start, adjusted_query);

    if (result.type == BSR_FOUND_GOOD_BLOCK) {
        split_if_too_big(result.block, adjusted_query);
        result.block->is_free = false;
    }

    return result;
}

static struct block_header *grow_heap(struct block_header *last, size_t query) {
    if (last == NULL) {
        return NULL;
    }

    const size_t adjusted_query = normalize_query(query);
    void *desired_addr = block_after(last);

    struct region new_region = alloc_region(desired_addr, adjusted_query);
    if (region_is_invalid(&new_region)) {
        return NULL;
    }

    last->next = (struct block_header *)new_region.addr;

    if (last->is_free && try_merge_with_next(last)) {
        return last;
    }

    return last->next;
}

static struct block_header *memalloc(size_t query, struct block_header *heap_start) {
    if (heap_start == NULL) {
        return NULL;
    }

    struct block_search_result result = try_memalloc_existing(query, heap_start);
    if (result.type == BSR_CORRUPTED) {
        return NULL;
    }

    if (result.type == BSR_FOUND_GOOD_BLOCK) {
        return result.block;
    }

    struct block_header *grown_block = grow_heap(result.block, query);
    if (grown_block == NULL) {
        return NULL;
    }

    result = try_memalloc_existing(query, grown_block);
    if (result.type == BSR_FOUND_GOOD_BLOCK) {
        return result.block;
    }

    return NULL;
}

void *heap_init(size_t initial_size) {
    const struct region region = alloc_region(HEAP_START, initial_size);
    if (region_is_invalid(&region)) {
        return NULL;
    }

    return region.addr;
}

void heap_term(void) {
    struct block_header *current = (struct block_header *)HEAP_START;

    while (current != NULL) {
        void *region_start = current;
        size_t region_size = size_from_capacity(current->capacity).bytes;

        while (current->next != NULL && current->next == block_after(current)) {
            current = current->next;
            region_size += size_from_capacity(current->capacity).bytes;
        }

        struct block_header *next_region = current->next;

        if (munmap(region_start, region_size) == -1) {
            fprintf(stderr, "Unmapping failed!\n");
        }

        current = next_region;
    }
}

void *_malloc(size_t query) {
    struct block_header *block = memalloc(query, (struct block_header *)HEAP_START);
    return block == NULL ? NULL : block->contents;
}

void _free(void *mem) {
    if (mem == NULL) {
        return;
    }

    struct block_header *header = block_get_header(mem);
    header->is_free = true;

    while (try_merge_with_next(header)) {
    }
}
