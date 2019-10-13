/*
 * Copyright 2017-2019 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_STRING_H
#define DY_STRING_H

#include <duality/support/api.h>

#include <stdbool.h>
#include <stddef.h>

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

DY_SUPPORT_API bool dy_string_are_equal(dy_string_t s1, dy_string_t s2);

DY_SUPPORT_API bool dy_string_matches_prefix(dy_string_t s, dy_string_t prefix);

DY_SUPPORT_API bool dy_string_first_matches_one_of(dy_string_t s, dy_string_t chars);

DY_SUPPORT_API dy_string_t dy_string_advance_by(dy_string_t s, size_t n);

DY_SUPPORT_API bool dy_string_matches_one_of(char c, dy_string_t s);

#endif // DY_STRING_H
