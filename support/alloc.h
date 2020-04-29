/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_ALLOC_H
#define DY_ALLOC_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/**
 * This file contains wrappers around standard
 * library memory allocation functions that
 * assert() on NULL.
 *
 * Duality deliberately does not try to recover
 * from OOM situations.
 */

static inline void *dy_malloc(size_t size);

static inline void *dy_calloc(size_t count, size_t size);

static inline void *dy_realloc(void *ptr, size_t size);

static inline void dy_free(void *ptr);

void *dy_malloc(size_t size)
{
    void *p = malloc(size);
    assert(p);
    return p;
}

void *dy_calloc(size_t count, size_t size)
{
    void *p = calloc(count, size);
    assert(p);
    return p;
}

void *dy_realloc(void *ptr, size_t size)
{
    void *p = realloc(ptr, size);
    assert(p);
    return p;
}

void dy_free(void *ptr)
{
    free(ptr);
}

#endif // DY_ALLOC_H
