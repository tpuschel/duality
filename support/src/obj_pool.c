/*
 * Copyright 2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <duality/support/obj_pool.h>

#include <duality/support/array.h>
#include <duality/support/allocator.h>
#include <duality/support/assert.h>

#include <string.h>
#include <limits.h>

#include <stdio.h>

#ifdef DY_HAVE_UNISTD
#    include <unistd.h>
#endif

struct slot {
    size_t ref_cnt;
    char rest[];
};

struct block {
    void *base;
    void *end;
};

struct dy_obj_pool {
    dy_array_t *blocks;
    size_t obj_size;
    size_t slot_padding;
    size_t page_size;
    dy_obj_pool_is_parent_cb is_parent_cb;
};

static size_t compute_padding(size_t addr, size_t align);
static size_t get_page_size(void);
static void print_parents(dy_obj_pool_t *pool, const void *child);

dy_obj_pool_t *dy_obj_pool_create(size_t obj_size, size_t obj_align)
{
    size_t padding;
    if (obj_align >= _Alignof(struct slot)) {
        padding = compute_padding(sizeof(struct slot), obj_align);
    } else {
        padding = compute_padding(obj_size, _Alignof(struct slot));
    }

    struct dy_obj_pool pool = {
        .blocks = dy_array_create(sizeof(struct block), 8),
        .obj_size = obj_size,
        .slot_padding = padding,
        .page_size = get_page_size(),
        .is_parent_cb = NULL
    };

    return dy_alloc_and_copy(&pool, sizeof pool);
}

void *dy_obj_pool_alloc(dy_obj_pool_t *pool)
{
    for (size_t i = 0, size = dy_array_size(pool->blocks); i < size; ++i) {
        struct block *block = dy_array_get_ptr(pool->blocks, i);

        struct slot *slot = block->base;

        for (;;) {
            char *data = slot->rest + pool->slot_padding;

            void *slot_end = data + pool->obj_size;

            if (slot_end > block->end) {
                break;
            }

            if (slot->ref_cnt != 0) {
                slot = slot_end;
                continue;
            }

            ++slot->ref_cnt;

            return data;
        }

        // TODO: Try to enlarge block if mmap()ed.
    }

    // TODO: Use mmap in conjunction with mremap or something like that.
    struct slot *slot = dy_calloc(1, pool->page_size);

    struct block block = {
        .base = slot,
        .end = (char *)slot + pool->page_size
    };

    dy_array_add(pool->blocks, &block);

    void *data = slot->rest + pool->slot_padding;

    ++slot->ref_cnt;

    return data;
}

void *dy_obj_pool_new(dy_obj_pool_t *pool, const void *ptr)
{
    void *p = dy_obj_pool_alloc(pool);
    memcpy(p, ptr, pool->obj_size);
    return p;
}

void *dy_obj_pool_retain(dy_obj_pool_t *pool, const void *ptr)
{
    char *char_ptr = ptr;

    struct slot *slot = (void *)(char_ptr - pool->slot_padding - sizeof(struct slot));

    ++slot->ref_cnt;

    return char_ptr;
}

size_t dy_obj_pool_release(dy_obj_pool_t *pool, const void *ptr)
{
    char *char_ptr = ptr;

    struct slot *slot = (void *)(char_ptr - pool->slot_padding - sizeof(struct slot));

    if (slot->ref_cnt == 0) {
        if (pool->is_parent_cb != NULL) {
            fprintf(stderr, "Alive objects referencing this one:\n");
            print_parents(pool, ptr);
        }

        dy_bail("Trying to release a non-existent object.");
    }

    --slot->ref_cnt;

    return slot->ref_cnt;
}

void dy_obj_pool_set_is_parent_cb(dy_obj_pool_t *pool, dy_obj_pool_is_parent_cb cb)
{
    pool->is_parent_cb = cb;
}

void dy_obj_pool_destroy(dy_obj_pool_t *pool)
{
    for (size_t i = 0, size = dy_array_size(pool->blocks); i < size; ++i) {
        struct block *block = dy_array_get_ptr(pool->blocks, i);

        dy_free(block->base);
    }

    dy_array_destroy(pool->blocks);

    dy_free(pool);
}

size_t compute_padding(size_t addr, size_t align)
{
    size_t padding = 0;
    size_t remainder = addr % align;
    if (remainder) {
        padding = align - remainder;
    }

    return padding;
}

size_t get_page_size(void)
{
#ifdef DY_HAVE_UNISTD
    long ret = sysconf(_SC_PAGESIZE);
    dy_assert(ret >= 0);
    return (size_t)ret;
#else
    // Just for now
    return 0x1000;
#endif
}

void print_parents(dy_obj_pool_t *pool, const void *child)
{
    for (size_t i = 0, size = dy_array_size(pool->blocks); i < size; ++i) {
        struct block *block = dy_array_get_ptr(pool->blocks, i);

        struct slot *slot = block->base;

        for (;;) {
            char *data = slot->rest + pool->slot_padding;

            void *slot_end = data + pool->obj_size;

            if (slot_end > block->end) {
                break;
            }

            if (slot->ref_cnt != 0) {
                if (pool->is_parent_cb(data, child)) {
                    fprintf(stderr, "%p\n", (void *)data);
                }
            }

            slot = slot_end;
        }
    }
}
