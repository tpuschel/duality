/*
 * Copyright 2017-2019 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_ALLOCATOR_H
#define DY_ALLOCATOR_H

#include <stddef.h>
#include <string.h>

#include <duality/support/api.h>

struct dy_allocator {
    void *(*alloc)(size_t size, void *env);
    void *(*realloc)(void *ptr, size_t size, void *env);
    void (*free)(void *ptr, size_t size, void *env);
    void *env;
};

DY_SUPPORT_API struct dy_allocator dy_allocator_stdlib(void);

static inline void *dy_alloc(const void *ptr, size_t size, struct dy_allocator allocator)
{
    void *p = allocator.alloc(size, allocator.env);
    memcpy(p, ptr, size);
    return p;
}

#endif // DY_ALLOCATOR_H
