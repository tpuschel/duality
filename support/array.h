/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_ARRAY_H
#define DY_ARRAY_H

#include <stddef.h>

#include "assert.h"
#include "overflow.h"
#include "alloc.h"

typedef struct dy_array dy_array_t;

static inline dy_array_t *dy_array_create(size_t elem_size, size_t capacity);

static inline size_t dy_array_add(dy_array_t *array, const void *value);

static inline void dy_array_prepend_keep_order(dy_array_t *array, const void *value);

static inline void dy_array_remove(dy_array_t *array, size_t index);

static inline void dy_array_remove_keep_order(dy_array_t *array, size_t index);

static inline void dy_array_get(const dy_array_t *array, size_t index, void *value);

static inline void *dy_array_get_ptr(const dy_array_t *array, size_t index);

static inline void dy_array_set(dy_array_t *array, size_t index, const void *value);

static inline void dy_array_insert_keep_order(dy_array_t *array, size_t index, const void *value);

static inline void *dy_array_buffer(const dy_array_t *array);

static inline size_t dy_array_size(const dy_array_t *array);

static inline size_t dy_array_capacity(const dy_array_t *array);

static inline void dy_array_set_excess_capacity(dy_array_t *array, size_t excess_capacity);

static inline void *dy_array_excess_buffer(const dy_array_t *array);

static inline void dy_array_add_to_size(dy_array_t *array, size_t added_size);

static inline void dy_array_set_size(dy_array_t *array, size_t size);

static inline size_t dy_array_pop(dy_array_t *array, void *value);

static inline void dy_array_destroy_instance(dy_array_t *array);

static inline void dy_array_destroy(dy_array_t *array);

struct dy_array {
    char *buffer;
    size_t elem_size;
    size_t num_elems;
    size_t capacity;
};

static inline void *pos(const dy_array_t *array, size_t i);
static inline void *last(const dy_array_t *array);

dy_array_t *dy_array_create(size_t elem_size, size_t capacity)
{
    dy_assert(elem_size != 0);

    size_t capacity_in_bytes;
    dy_assert(!dy_size_t_mul_overflow(elem_size, capacity, &capacity_in_bytes));

    dy_array_t array = {
        .buffer = dy_malloc(capacity_in_bytes),
        .elem_size = elem_size,
        .num_elems = 0,
        .capacity = capacity
    };

    dy_array_t *p = dy_malloc(sizeof array);
    memcpy(p, &array, sizeof *p);
    return p;
}

size_t dy_array_add(dy_array_t *array, const void *value)
{
    size_t size = array->num_elems;
    dy_array_insert_keep_order(array, size, value);
    return size;
}

void dy_array_prepend_keep_order(dy_array_t *array, const void *value)
{
    dy_array_insert_keep_order(array, 0, value);
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

void dy_array_insert_keep_order(dy_array_t *array, size_t index, const void *value)
{
    dy_assert(index <= array->num_elems);

    if (array->num_elems == array->capacity) {
        if (array->capacity == 0) {
            array->capacity = 8;
        } else {
            dy_assert(!dy_size_t_mul_overflow(array->capacity, 2, &array->capacity));
        }

        size_t capacity_in_bytes;
        dy_assert(!dy_size_t_mul_overflow(array->elem_size, array->capacity, &capacity_in_bytes));

        array->buffer = dy_realloc(array->buffer, capacity_in_bytes);
    }

    memmove(pos(array, index + 1), pos(array, index), array->elem_size * (array->num_elems - index));
    memcpy(pos(array, index), value, array->elem_size);
    ++array->num_elems;
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

void dy_array_set_excess_capacity(dy_array_t *array, size_t excess_capacity)
{
    size_t current_excess_capacity = array->capacity - array->num_elems;

    if (current_excess_capacity >= excess_capacity) {
        return;
    }

    size_t added_capacity = excess_capacity - current_excess_capacity;

    dy_assert(!dy_size_t_add_overflow(array->capacity, added_capacity, &array->capacity));

    size_t capacity_in_bytes;
    dy_assert(!dy_size_t_mul_overflow(array->elem_size, array->capacity, &capacity_in_bytes));

    array->buffer = dy_realloc(array->buffer, capacity_in_bytes);
}

void dy_array_add_to_size(dy_array_t *array, size_t added_size)
{
    dy_assert(!dy_size_t_add_overflow(array->num_elems, added_size, &array->num_elems));
}

void *dy_array_excess_buffer(const dy_array_t *array)
{
    return pos(array, array->num_elems);
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
    dy_free(array);
}

void dy_array_destroy(dy_array_t *array)
{
    dy_free(array->buffer);

    dy_array_destroy_instance(array);
}

#endif // DY_ARRAY_H
