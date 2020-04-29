/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_STRING_H
#define DY_STRING_H

#include <stdbool.h>
#include <stddef.h>

/**
 * Implements the perennial string type that knows its own size :).
 */

typedef struct dy_string {
    const char *ptr;
    size_t size;
} dy_string_t;

#define DY_STR_LIT_INIT(x)     \
    {                          \
        .ptr = x,              \
        .size = (sizeof x) - 1 \
    }

#define DY_STR_LIT(x) (dy_string_t) DY_STR_LIT_INIT(x)

static inline bool dy_string_are_equal(dy_string_t s1, dy_string_t s2);

static inline bool dy_string_matches_prefix(dy_string_t s, dy_string_t prefix);

static inline bool dy_string_first_matches_one_of(dy_string_t s, dy_string_t chars);

static inline dy_string_t dy_string_advance_by(dy_string_t s, size_t n);

static inline bool dy_string_matches_one_of(char c, dy_string_t s);

bool dy_string_are_equal(dy_string_t s1, dy_string_t s2)
{
    if (s1.size != s2.size) {
        return false;
    }

    for (size_t i = 0; i < s1.size; ++i) {
        if (s1.ptr[i] != s2.ptr[i]) {
            return false;
        }
    }

    return true;
}

bool dy_string_matches_prefix(dy_string_t s, dy_string_t prefix)
{
    if (s.size < prefix.size) {
        return false;
    }

    for (size_t i = 0; i < prefix.size; ++i) {
        if (s.ptr[i] != prefix.ptr[i]) {
            return false;
        }
    }

    return true;
}

bool dy_string_first_matches_one_of(dy_string_t s, dy_string_t chars)
{
    if (s.size == 0) {
        return false;
    }

    for (size_t i = 0; i < chars.size; ++i) {
        if (s.ptr[0] == chars.ptr[i]) {
            return true;
        }
    }

    return false;
}

dy_string_t dy_string_advance_by(dy_string_t s, size_t n)
{
    return (dy_string_t){
        .ptr = s.ptr + n,
        .size = s.size - n
    };
}

bool dy_string_matches_one_of(char c, dy_string_t s)
{
    for (size_t i = 0; i < s.size; ++i) {
        if (c == s.ptr[i]) {
            return true;
        }
    }

    return false;
}

#endif // DY_STRING_H
