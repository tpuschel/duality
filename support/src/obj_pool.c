/*
 * Copyright 2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <duality/support/obj_pool.h>

#include <duality/support/allocator.h>

#include <string.h>

struct slot {
    size_t ref_cnt;
    char rest[];
};

struct dy_obj_pool {
    size_t obj_size;
    size_t slot_padding;
};

static size_t compute_padding(size_t addr, size_t align);

dy_obj_pool_t *dy_obj_pool_create(size_t obj_size, size_t obj_align)
{
    size_t padding;
    if (obj_align >= _Alignof(struct slot)) {
        padding = compute_padding(sizeof(struct slot), obj_align);
    } else {
        padding = compute_padding(obj_size, _Alignof(struct slot));
    }

    struct dy_obj_pool pool = {
        .obj_size = obj_size,
        .slot_padding = padding
    };

    return dy_alloc_and_copy(&pool, sizeof pool);
}

void *dy_obj_pool_alloc(dy_obj_pool_t *pool)
{
    size_t data_offset = sizeof(struct slot) + pool->slot_padding;

    struct slot *slot = dy_calloc(1, data_offset + pool->obj_size);

    ++slot->ref_cnt;

    return (char *)slot + data_offset;
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

    size_t new_ref_cnt = --slot->ref_cnt;

    if (new_ref_cnt == 0) {
        dy_free(slot);
    }

    return new_ref_cnt;
}

void dy_obj_pool_destroy(dy_obj_pool_t *pool)
{
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
