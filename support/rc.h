/*
 * Copyright 2020-2021 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stddef.h>

/**
 * Implements reference-counting allocation functions.
 */

static inline void *dy_rc_alloc(size_t size, size_t alignment);

/**
 * Copies 'size' bytes from 'ptr' to a reference-counted slot in the heap.
 * Returns a pointer to the object portion of that slot.
 */
static inline void *dy_rc_new(void *ptr, size_t size, size_t alignment);

/**
 * Increments the reference count of the object pointed to by 'ptr'.
 * Returns 'ptr'.
 */
static inline void *dy_rc_retain(void *ptr, size_t alignment);

/**
 * Decrements the reference count of the object pointed to by 'ptr'.
 * Frees the object if the new reference count is 0.
 * Returns the new reference count.
 */
static inline size_t dy_rc_release(void *ptr, size_t alignment);

/**
 * Tries to resize the allocation pointed to by 'ptr' in place.
 * If that fails, allocates new space of size 'new_size' and copies over the old content.
 */
static inline void *dy_rc_realloc(void *ptr, size_t new_size, size_t alignment);

#ifndef DY_FREESTANDING

#    include <stdlib.h>
#    include <string.h>
#    include <assert.h>

#    include "util.h"

void *dy_rc_alloc(size_t size, size_t alignment)
{
    const size_t pre_padding = DY_COMPUTE_PADDING(sizeof(size_t), alignment);

    size_t *rc = calloc(1, sizeof *rc + pre_padding + size);
    assert(rc);

    *rc = 1;

    return (char *)(rc + 1) + pre_padding;
}

void *dy_rc_new(void *ptr, size_t size, size_t alignment)
{
    void *p = dy_rc_alloc(size, alignment);

    memcpy(p, ptr, size);

    return p;
}

void *dy_rc_retain(void *ptr, size_t alignment)
{
    const size_t pre_padding = DY_COMPUTE_PADDING(sizeof(size_t), alignment);

    size_t *rc = (void *)((char *)ptr - pre_padding - sizeof *rc);

    ++*rc;

    return ptr;
}

size_t dy_rc_release(void *ptr, size_t alignment)
{
    const size_t pre_padding = DY_COMPUTE_PADDING(sizeof(size_t), alignment);

    size_t *rc = (void *)((char *)ptr - pre_padding - sizeof *rc);

    size_t new_ref_cnt = --*rc;

    if (new_ref_cnt == 0) {
        free(rc);
    }

    return new_ref_cnt;
}

void *dy_rc_realloc(void *ptr, size_t new_size, size_t alignment)
{
    const size_t pre_padding = DY_COMPUTE_PADDING(sizeof(size_t), alignment);

    size_t *old = (void *)((char *)ptr - pre_padding - sizeof *old);

    size_t *new = realloc(old, sizeof *new + pre_padding + new_size);
    assert(new);

    return (char *)(new + 1) + pre_padding;
}

#endif // !DY_FREESTANDING
