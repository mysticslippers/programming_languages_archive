#define _DEFAULT_SOURCE

#include <stdio.h>
#include <unistd.h>

#include "mem_internals.h"
#include "mem.h"
#include "util.h"

void debug_block(struct block_header *b, const char *fmt, ...);
void debug(const char *fmt, ...);

extern inline block_size size_from_capacity(block_capacity cap);


extern inline block_capacity capacity_from_size(block_size sz);

static bool block_is_big_enough(size_t query, struct block_header *block) { return block->capacity.bytes >= query; }

static size_t pages_count(size_t mem) { return mem / getpagesize() + ((mem % getpagesize()) > 0); }

static size_t round_pages(size_t mem) { return getpagesize() * pages_count(mem); }

static void block_init(void *restrict addr, block_size block_sz, void *restrict next) {
    *((struct block_header *) addr) = (struct block_header) {
            .next = next,
            .capacity = capacity_from_size(block_sz),
            .is_free = true
    };
}

static size_t region_actual_size(size_t query) { return size_max(round_pages(query), REGION_MIN_SIZE); }

extern inline bool region_is_invalid(const struct region *r);

static void *map_pages(void const *addr, size_t length, int additional_flags) {
    return mmap((void *) addr, length, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | additional_flags, -1, 0);
}

static struct region alloc_region(void const *addr, size_t query) {
    size_t size = region_actual_size(size_from_capacity((block_capacity) {.bytes = query}).bytes);
    void *region_addr = map_pages(addr, size, MAP_FIXED_NOREPLACE);
    if (region_addr == MAP_FAILED) {
        region_addr = map_pages(addr, size, 0);
        if (region_addr == MAP_FAILED) {
            return REGION_INVALID;
        }
    }
    block_init(region_addr, (block_size) {.bytes = size}, NULL);
    return (struct region) {
            .addr = region_addr,
            .size = size,
            .extends = (region_addr == addr)
    };
}

static void *block_after(struct block_header const *block);

void *heap_init(size_t initial) {
    const struct region region = alloc_region(HEAP_START, initial);
    if (region_is_invalid(&region)) return NULL;

    return region.addr;
}

void heap_term() {
    struct block_header *current_header = (struct block_header *) HEAP_START;
    while (current_header != NULL) {
        size_t current_region_size = 0;
        void *current_region_start = current_header;
        while (current_header->next == block_after(current_header)) {
            current_region_size += size_from_capacity(current_header->capacity).bytes;
            current_header = current_header->next;
        }
        current_region_size += size_from_capacity(current_header->capacity).bytes;
        current_header = current_header->next;
        int unmapping_result = munmap(current_region_start, current_region_size);
        if (unmapping_result == -1) {
            fprintf(stderr, "Unmapping failed!");
        }
    }
}

#define BLOCK_MIN_CAPACITY 24

static bool block_splittable(struct block_header *restrict block, size_t query) {
    return block->is_free &&
           query + offsetof(struct block_header, contents) + BLOCK_MIN_CAPACITY <= block->capacity.bytes;
}

static bool split_if_too_big(struct block_header *block, size_t query) {
    if (block_splittable(block, query)) {
        void *new_block = block->contents + query;
        const block_size new_size = (block_size) {.bytes = (block->capacity.bytes - query)};

        block_init(new_block, new_size, block->next);
        block->next = new_block;
        block->capacity.bytes = query;

        return true;
    }
    return false;
}

static void *block_after(struct block_header const *block) {
    return (void *) (block->contents + block->capacity.bytes);
}

static bool blocks_continuous(
        struct block_header const *fst,
        struct block_header const *snd) {
    return (void *) snd == block_after(fst);
}

static bool mergeable(struct block_header const *restrict fst, struct block_header const *restrict snd) {
    return fst->is_free && snd->is_free && blocks_continuous(fst, snd);
}

static bool try_merge_with_next(struct block_header *block) {
    if (block->next == NULL || !mergeable(block, block->next)) {
        return false;
    }
    block->capacity.bytes += size_from_capacity(block->next->capacity).bytes;
    block->next = block->next->next;
    return true;
}

struct block_search_result {
    enum {
        BSR_FOUND_GOOD_BLOCK, BSR_REACHED_END_NOT_FOUND, BSR_CORRUPTED
    } type;
    struct block_header *block;
};


static struct block_search_result find_good_or_last(struct block_header *restrict block, size_t sz) {
    if (!block) return (struct block_search_result) {BSR_CORRUPTED, NULL};
    struct block_header *last_block = NULL;
    while (block) {
        while (try_merge_with_next(block));
        if (block->is_free && block_is_big_enough(sz, block))
            return (struct block_search_result) {BSR_FOUND_GOOD_BLOCK, block};
        last_block = block;
        block = block->next;
    }
    return (struct block_search_result) {BSR_REACHED_END_NOT_FOUND, last_block};
}

static struct block_search_result try_memalloc_existing(size_t query, struct block_header *block) {
    size_t adjusted_query = (query < BLOCK_MIN_CAPACITY) ? BLOCK_MIN_CAPACITY : query;

    struct block_search_result result = find_good_or_last(block, adjusted_query);

    if (result.type == BSR_FOUND_GOOD_BLOCK) {
        split_if_too_big(result.block, adjusted_query);
        result.block -> is_free = false;
    }

        return result;
    }

static struct block_header *grow_heap(struct block_header *restrict last, size_t query) {
    void *new_region = block_after(last);
    struct region new = alloc_region(new_region, size_max(query, BLOCK_MIN_CAPACITY));
    if (region_is_invalid(&new)) {
        return NULL;
    }
    block_init(new.addr, (block_size) {new.size}, NULL);
    if (last == NULL) {
        return NULL;
    }
    last->next = (struct block_header *) new.addr;
    if (last->is_free && try_merge_with_next(last)) return last;
    else { return last->next; }
}

static struct block_header *memalloc(size_t query, struct block_header *heap_start) {
    if (heap_start == NULL) {
        return NULL;
    }

    struct block_search_result res = try_memalloc_existing(query, heap_start);

    if (res.type != BSR_CORRUPTED) {
        if (res.type == BSR_FOUND_GOOD_BLOCK) {
            return res.block;
        }
        struct block_header *grown_block = grow_heap(res.block, query);
        if (grown_block == NULL) {
            return NULL;
        }
        res = try_memalloc_existing(query, grown_block);
        if (res.type == BSR_FOUND_GOOD_BLOCK) {
            return res.block;
        }
    }
    return NULL;
}

void *_malloc(size_t query) {
    struct block_header *const addr = memalloc(query, (struct block_header *) HEAP_START);
    if (addr) return addr->contents;
    else return NULL;
}

static struct block_header *block_get_header(void *contents) {
    return (struct block_header *) (((uint8_t *) contents) - offsetof(struct block_header, contents));
}

void _free(void *mem) {
    if (mem == NULL) return;
    struct block_header *header = block_get_header(mem);
    header->is_free = true;
    while (try_merge_with_next(header));
}
