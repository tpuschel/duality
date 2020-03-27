/*
 * Copyright 2017-2019 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <duality/support/allocator.h>

#include <duality/support/assert.h>

#include <stdlib.h>
#include <string.h>


void *dy_malloc(size_t size)
{
    void *p = malloc(size);
    dy_assert(p);
    return p;
}

void *dy_realloc(void *ptr, size_t size)
{
    void *p = realloc(ptr, size);
    dy_assert(p);
    return p;
}

void dy_free(void *ptr)
{
    free(ptr);
}

void *dy_alloc_and_copy(const void *ptr, size_t size)
{
    void *p = dy_malloc(size);
    memcpy(p, ptr, size);
    return p;
}
