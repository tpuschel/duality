/*
 * Copyright 2017-2019 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <duality/support/allocator.h>

#include <stdlib.h>

static void *malloc_(size_t size, void *env);
static void *realloc_(void *ptr, size_t size, void *env);
static void free_(void *ptr, size_t size, void *env);

struct dy_allocator dy_allocator_stdlib()
{
    struct dy_allocator allocator = {
        .alloc = malloc_,
        .realloc = realloc_,
        .free = free_,
        .env = NULL
    };

    return allocator;
}


void *malloc_(size_t size, void *env)
{
    (void)env;

    return malloc(size);
}

void *realloc_(void *ptr, size_t size, void *env)
{
    (void)env;

    return realloc(ptr, size);
}

void free_(void *ptr, size_t size, void *env)
{
    (void)env;
    (void)size;

    free(ptr);
}
