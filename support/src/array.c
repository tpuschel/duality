/*
 * Copyright 2017-2019 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <duality/support/array.h>

#include <duality/support/assert.h>
#include <duality/support/overflow.h>

#include <string.h>

struct dy_array {
    char *buffer;
    size_t elem_size;
    size_t num_elems;
    size_t capacity;
    struct dy_allocator instance_allocator;
    struct dy_allocator space_allocator;
};

static void *pos(const dy_array_t *array, size_t i);
static void *last(const dy_array_t *array);

dy_array_t *dy_array_create(struct dy_allocator allocator, size_t elem_size, size_t capacity)
{
    return dy_array_create_with_seperate_allocators(allocator, allocator, elem_size, capacity);
}

dy_array_t *dy_array_create_with_seperate_allocators(struct dy_allocator instance_allocator, struct dy_allocator space_allocator, size_t elem_size, size_t capacity)
{
    size_t capacity_in_bytes;
    dy_assert(!dy_size_t_mul_overflow(elem_size, capacity, &capacity_in_bytes));

    dy_array_t array = {
        .buffer = space_allocator.alloc(capacity_in_bytes, space_allocator.env),
        .elem_size = elem_size,
        .num_elems = 0,
        .capacity = capacity,
        .instance_allocator = instance_allocator,
        .space_allocator = space_allocator,
    };

    return dy_alloc(&array, sizeof array, instance_allocator);
}

size_t dy_array_add(dy_array_t *array, const void *value)
{
    size_t size = array->num_elems;
    memcpy(dy_array_add_uninit(array), value, array->elem_size);
    return size;
}

void *dy_array_add_uninit(dy_array_t *array)
{
    if (array->num_elems == array->capacity) {
        dy_assert(!dy_size_t_mul_overflow(array->capacity, 2, &array->capacity));

        size_t capacity_in_bytes;
        dy_assert(!dy_size_t_mul_overflow(array->elem_size, array->capacity, &capacity_in_bytes));

        array->buffer = array->space_allocator.realloc(array->buffer, capacity_in_bytes, array->space_allocator.env);
    }

    ++array->num_elems;

    return last(array);
}

void dy_array_prepend(dy_array_t *array, const void *value)
{
    if (array->num_elems == array->capacity) {
        dy_assert(!dy_size_t_mul_overflow(array->capacity, 2, &array->capacity));

        size_t capacity_in_bytes;
        dy_assert(!dy_size_t_mul_overflow(array->elem_size, array->capacity, &capacity_in_bytes));

        array->buffer = array->space_allocator.realloc(array->buffer, capacity_in_bytes, array->space_allocator.env);
    }

    memmove(pos(array, 1), pos(array, 0), array->elem_size * (array->num_elems - 1));
    memcpy(pos(array, 0), value, array->elem_size);
    ++array->elem_size;
}

void dy_array_remove(dy_array_t *array, size_t index)
{
    memmove(pos(array, index), last(array), array->elem_size);
    --array->num_elems;
}

void dy_array_remove_keep_order(dy_array_t *array, size_t index)
{
    memcpy(pos(array, index), pos(array, index + 1), array->elem_size * (array->num_elems - index - 1));
    --array->num_elems;
}

void dy_array_get(const dy_array_t *array, size_t index, void *value)
{
    memcpy(value, pos(array, index), array->elem_size);
}

void *dy_array_get_ptr(const dy_array_t *array, size_t index)
{
    return pos(array, index);
}

void dy_array_set(dy_array_t *array, size_t index, const void *value)
{
    memcpy(pos(array, index), value, array->elem_size);
}

void *dy_array_buffer(const dy_array_t *array)
{
    return array->buffer;
}

size_t dy_array_size(const dy_array_t *array)
{
    return array->num_elems;
}

size_t dy_array_capacity(const dy_array_t *array)
{
    return array->capacity;
}

void dy_array_set_size(dy_array_t *array, size_t size)
{
    array->num_elems = size;
}

struct dy_allocator dy_array_instance_allocator(dy_array_t *array)
{
    return array->instance_allocator;
}

struct dy_allocator dy_array_space_allocator(dy_array_t *array)
{
    return array->space_allocator;
}

size_t dy_array_pop(dy_array_t *array, void *value)
{
    memcpy(value, last(array), array->elem_size);
    --array->num_elems;
    return array->num_elems;
}

void *pos(const dy_array_t *array, size_t i)
{
    dy_assert(i < array->capacity + 1);
    return &array->buffer[i * array->elem_size];
}

void *last(const dy_array_t *array)
{
    return pos(array, array->num_elems - 1);
}

void dy_array_destroy_instance(dy_array_t *array)
{
    array->instance_allocator.free(array, sizeof *array, array->instance_allocator.env);
}

void dy_array_destroy(dy_array_t *array)
{
    size_t capacity_in_bytes;
    dy_assert(!dy_size_t_mul_overflow(array->elem_size, array->capacity, &capacity_in_bytes));

    array->space_allocator.free(array->buffer, capacity_in_bytes, array->space_allocator.env);

    dy_array_destroy_instance(array);
}
