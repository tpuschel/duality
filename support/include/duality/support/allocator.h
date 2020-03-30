/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_ALLOCATOR_H
#define DY_ALLOCATOR_H

#include <stddef.h>

#include <duality/support/api.h>

DY_SUPPORT_API void *dy_malloc(size_t size);

DY_SUPPORT_API void *dy_calloc(size_t count, size_t size);

DY_SUPPORT_API void *dy_realloc(void *ptr, size_t size);

DY_SUPPORT_API void dy_free(void *ptr);

DY_SUPPORT_API void *dy_alloc_and_copy(const void *ptr, size_t size);

#endif // DY_ALLOCATOR_H
