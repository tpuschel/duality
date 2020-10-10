/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "overflow.h"
#include "rc.h"
#include "string.h"

/**
 * This file implements dynamically
 * growable arrays.
 */

typedef struct dy_array {
    void *buffer;
    size_t elem_size;
    size_t num_elems;
    size_t capacity;
    size_t pre_padding;
    size_t post_padding;
} dy_array_t;

static inline dy_array_t dy_array_create(size_t elem_size, size_t alignment, size_t capacity);

static inline size_t dy_array_add(dy_array_t *array, const void *value);

static inline void dy_array_prepend_keep_order(dy_array_t *array, const void *value);

static inline void dy_array_remove(dy_array_t *array, size_t index);

static inline void dy_array_remove_keep_order(dy_array_t *array, size_t index);

static inline void dy_array_get(dy_array_t array, size_t index, void *value);

static inline void dy_array_set(dy_array_t array, size_t index, const void *value);

static inline void dy_array_insert_keep_order(dy_array_t *array, size_t index, const void *value);

static inline void dy_array_set_excess_capacity(dy_array_t *array, size_t excess_capacity);

static inline void *dy_array_excess_buffer(dy_array_t array);

static inline void dy_array_add_to_size(dy_array_t *array, size_t added_size);

static inline size_t dy_array_pop(dy_array_t *array, void *value);

static inline void dy_array_destroy(dy_array_t array);

static inline void *dy_array_pos(dy_array_t array, size_t i);
static inline void *dy_array_last(dy_array_t array);

static inline dy_string_t dy_array_view(const dy_array_t *array);

dy_array_t dy_array_create(size_t elem_size, size_t alignment, size_t capacity)
{
    assert(elem_size != 0);

    size_t capacity_in_bytes;
    assert(!dy_size_t_mul_overflow(elem_size, capacity, &capacity_in_bytes));

    size_t pre_padding = DY_COMPUTE_PADDING(sizeof(size_t), alignment);
    size_t post_padding = DY_COMPUTE_PADDING(sizeof(size_t) + pre_padding + elem_size, DY_ALIGNOF(size_t));

    return (dy_array_t){
        .buffer = dy_rc_alloc(capacity_in_bytes, pre_padding, post_padding),
        .elem_size = elem_size,
        .num_elems = 0,
        .capacity = capacity,
        .pre_padding = pre_padding,
        .post_padding = post_padding
    };
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
    memmove(dy_array_pos(*array, index), dy_array_last(*array), array->elem_size);
    --array->num_elems;
}

void dy_array_remove_keep_order(dy_array_t *array, size_t index)
{
    memcpy(dy_array_pos(*array, index), dy_array_pos(*array, index + 1), array->elem_size * (array->num_elems - index - 1));
    --array->num_elems;
}

void dy_array_insert_keep_order(dy_array_t *array, size_t index, const void *value)
{
    assert(index <= array->num_elems);

    if (array->num_elems == array->capacity) {
        if (array->capacity == 0) {
            array->capacity = 8;
        } else {
            assert(!dy_size_t_mul_overflow(array->capacity, 2, &array->capacity));
        }

        size_t capacity_in_bytes;
        assert(!dy_size_t_mul_overflow(array->elem_size, array->capacity, &capacity_in_bytes));

        array->buffer = dy_rc_realloc(array->buffer, capacity_in_bytes, array->pre_padding, array->post_padding);
    }

    memmove(dy_array_pos(*array, index + 1), dy_array_pos(*array, index), array->elem_size * (array->num_elems - index));
    memcpy(dy_array_pos(*array, index), value, array->elem_size);
    ++array->num_elems;
}

void dy_array_get(dy_array_t array, size_t index, void *value)
{
    memcpy(value, dy_array_pos(array, index), array.elem_size);
}

void dy_array_set(dy_array_t array, size_t index, const void *value)
{
    memcpy(dy_array_pos(array, index), value, array.elem_size);
}

void dy_array_set_excess_capacity(dy_array_t *array, size_t excess_capacity)
{
    size_t current_excess_capacity = array->capacity - array->num_elems;

    if (current_excess_capacity >= excess_capacity) {
        return;
    }

    size_t added_capacity = excess_capacity - current_excess_capacity;

    assert(!dy_size_t_add_overflow(array->capacity, added_capacity, &array->capacity));

    size_t capacity_in_bytes;
    assert(!dy_size_t_mul_overflow(array->elem_size, array->capacity, &capacity_in_bytes));

    array->buffer = dy_rc_realloc(array->buffer, capacity_in_bytes, array->pre_padding, array->post_padding);
}

void dy_array_add_to_size(dy_array_t *array, size_t added_size)
{
    assert(!dy_size_t_add_overflow(array->num_elems, added_size, &array->num_elems));
}

void *dy_array_excess_buffer(dy_array_t array)
{
    return dy_array_pos(array, array.num_elems);
}

size_t dy_array_pop(dy_array_t *array, void *value)
{
    memcpy(value, dy_array_last(*array), array->elem_size);
    --array->num_elems;
    return array->num_elems;
}

void *dy_array_pos(dy_array_t array, size_t i)
{
    assert(i < array.capacity + 1);
    return (char *)array.buffer + (i * array.elem_size);
}

void *dy_array_last(dy_array_t array)
{
    return dy_array_pos(array, array.num_elems - 1);
}

void dy_array_destroy(dy_array_t array)
{
    dy_rc_release(array.buffer, array.pre_padding, array.post_padding);
}

dy_string_t dy_array_view(const dy_array_t *array)
{
    return (dy_string_t){
        .ptr = array->buffer,
        .size = array->num_elems
    };
}
