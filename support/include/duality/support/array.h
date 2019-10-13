/*
 * Copyright 2017-2019 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_ARRAY_H
#define DY_ARRAY_H

#include <duality/support/api.h>
#include <duality/support/allocator.h>

typedef struct dy_array dy_array_t;

DY_SUPPORT_API dy_array_t *dy_array_create(struct dy_allocator allocator, size_t elem_size, size_t capacity);

DY_SUPPORT_API dy_array_t *dy_array_create_with_seperate_allocators(struct dy_allocator instance_allocator, struct dy_allocator space_allocator, size_t elem_size, size_t capacity);

DY_SUPPORT_API size_t dy_array_add(dy_array_t *array, const void *value);

DY_SUPPORT_API void *dy_array_add_uninit(dy_array_t *array);

DY_SUPPORT_API void dy_array_prepend(dy_array_t *array, const void *value);

DY_SUPPORT_API void dy_array_remove(dy_array_t *array, size_t index);

DY_SUPPORT_API void dy_array_remove_keep_order(dy_array_t *array, size_t index);

DY_SUPPORT_API void dy_array_get(const dy_array_t *array, size_t index, void *value);

DY_SUPPORT_API void *dy_array_get_ptr(const dy_array_t *array, size_t index);

DY_SUPPORT_API void dy_array_set(dy_array_t *array, size_t index, const void *value);

DY_SUPPORT_API void *dy_array_buffer(const dy_array_t *array);

DY_SUPPORT_API size_t dy_array_size(const dy_array_t *array);

DY_SUPPORT_API size_t dy_array_capacity(const dy_array_t *array);

DY_SUPPORT_API void dy_array_set_size(dy_array_t *array, size_t size);

DY_SUPPORT_API size_t dy_array_pop(dy_array_t *array, void *value);

DY_SUPPORT_API struct dy_allocator dy_array_instance_allocator(dy_array_t *array);

DY_SUPPORT_API struct dy_allocator dy_array_space_allocator(dy_array_t *array);

DY_SUPPORT_API void dy_array_destroy_instance(dy_array_t *array);

DY_SUPPORT_API void dy_array_destroy(dy_array_t *array);

#endif // DY_ARRAY_H
