/*
 * Copyright 2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_RC_H
#define DY_RC_H

#include <stddef.h>

/**
 * Implements reference-counting allocation functions.
 */

static inline void *dy_rc_alloc(size_t size, size_t pre_padding, size_t post_padding);

/**
 * Copies 'size' bytes from 'ptr' to a reference-counted slot in the heap.
 * Returns a pointer to the object portion of that slot.
 */
static inline const void *dy_rc_new(const void *ptr, size_t size, size_t pre_padding, size_t post_padding);

/**
 * Increments the reference count of the object pointed to by 'ptr'.
 * Returns 'ptr'.
 */
static inline const void *dy_rc_retain(const void *ptr, size_t pre_padding, size_t post_padding);

/**
 * Decrements the reference count of the object pointed to by 'ptr'.
 * Frees the object if the new reference count is 0.
 * Returns the new reference count.
 */
static inline size_t dy_rc_release(const void *ptr, size_t pre_padding, size_t post_padding);

/**
 * Tries to resize the allocation pointed to by 'ptr' in place.
 * If that fails, allocates new space of size 'new_size' and copies over the old content.
 *
 * Note that relocation is only possible *if the reference count is 1*, meaning the allocation is not shared.
 */
static inline void *dy_rc_realloc(void *ptr, size_t new_size, size_t pre_padding, size_t post_padding);

#ifndef DY_FREESTANDING

#    include <stdlib.h>
#    include <string.h>
#    include <assert.h>

#    include "util.h"

/**
 * New/retain/release all take padding parameters that depend
 * on the alignment and size of the object to be allocated.
 *
 * Use the below macros to precompute the required paddings
 * for objects of a given type.
 */

#    define DY_RC_PRE_PADDING(type) \
        DY_COMPUTE_PADDING(sizeof(size_t), DY_ALIGNOF(type))

#    define DY_RC_POST_PADDING(type) \
        DY_COMPUTE_PADDING(sizeof(size_t) + DY_RC_PRE_PADDING(type) + sizeof(type), DY_ALIGNOF(size_t))

void *dy_rc_alloc(size_t size, size_t pre_padding, size_t post_padding)
{
    size_t *rc = calloc(1, sizeof *rc + pre_padding + size);
    assert(rc);

    *rc = 1;

    return (char *)(rc + 1) + pre_padding;
}

const void *dy_rc_new(const void *ptr, size_t size, size_t pre_padding, size_t post_padding)
{
    void *p = dy_rc_alloc(size, pre_padding, post_padding);

    memcpy(p, ptr, size);

    return p;
}

const void *dy_rc_retain(const void *ptr, size_t pre_padding, size_t post_padding)
{
    size_t *rc = (const char *)ptr - pre_padding - sizeof *rc;

    ++*rc;

    return ptr;
}

size_t dy_rc_release(const void *ptr, size_t pre_padding, size_t post_padding)
{
    size_t *rc = (const char *)ptr - pre_padding - sizeof *rc;

    size_t new_ref_cnt = --*rc;

    if (new_ref_cnt == 0) {
        free(rc);
    }

    return new_ref_cnt;
}

void *dy_rc_realloc(void *ptr, size_t new_size, size_t pre_padding, size_t post_padding)
{
    size_t *old_rc = (char *)ptr - pre_padding - sizeof *old_rc;
    assert(*old_rc == 1);

    size_t *new_rc = realloc(old_rc, sizeof *new_rc + pre_padding + new_size);
    assert(new_rc);

    return (char *)(new_rc + 1) + pre_padding;
}

#endif // !DY_FREESTANDING

#endif // DY_RC_H
