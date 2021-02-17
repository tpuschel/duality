/*
 * Copyright 2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "../core/check.h"
#include "../core/eval.h"
#include "string_type.h"

/**
 * Support for strings. Just for testing puposes, for now.
 */

static size_t dy_string_id;

struct dy_string_data {
    dy_array_t value;
};

static struct dy_core_expr dy_string_type_of(struct dy_core_ctx *ctx, void *data);

static dy_ternary_t dy_string_is_equal(struct dy_core_ctx *ctx, void *data1, void *data2);

static bool dy_string_check(struct dy_core_ctx *ctx, void *data, struct dy_core_expr *result);

static bool dy_string_remove_mentions_in_type(struct dy_core_ctx *ctx, void *data, size_t id, enum dy_polarity current_polarity, struct dy_core_expr *result);

static bool dy_string_eval(struct dy_core_ctx *ctx, void *data, bool *is_value, struct dy_core_expr *result);

static bool dy_string_substitute(struct dy_core_ctx *ctx, void *data, size_t id, struct dy_core_expr sub, struct dy_core_expr *result);

static dy_ternary_t dy_string_is_subtype(struct dy_core_ctx *ctx, void *subtype, void *supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static bool dy_string_contains_this_variable(struct dy_core_ctx *ctx, void *data, size_t id);

static void dy_string_variable_appears_in_polarity(struct dy_core_ctx *ctx, void *data, size_t id, enum dy_polarity current_polarity, bool *positive, bool *negative);

static void *dy_string_retain(struct dy_core_ctx *ctx, void *data);

static void dy_string_release(struct dy_core_ctx *ctx, void *data);

static void dy_string_to_string(struct dy_core_ctx *ctx, void *data, dy_array_t *string);

static inline struct dy_core_custom dy_string_create(struct dy_string_data data);

static inline struct dy_core_custom dy_string_create_no_alloc(struct dy_string_data *data);

static inline void dy_string_register(dy_array_t *reg)
{
    struct dy_core_custom_shared s = {
        .type_of = dy_string_type_of,
        .is_equal = dy_string_is_equal,
        .check = dy_string_check,
        .remove_mentions_in_type = dy_string_remove_mentions_in_type,
        .eval = dy_string_eval,
        .substitute = dy_string_substitute,
        .is_subtype = dy_string_is_subtype,
        .contains_this_variable = dy_string_contains_this_variable,
        .variable_appears_in_polarity = dy_string_variable_appears_in_polarity,
        .retain = dy_string_retain,
        .release = dy_string_release,
        .to_string = dy_string_to_string
    };

    dy_string_id = dy_array_add(reg, &s);
}

struct dy_core_custom dy_string_create(struct dy_string_data data)
{
    return dy_string_create_no_alloc(dy_rc_new(&data, sizeof data, DY_ALIGNOF(data)));
}

struct dy_core_custom dy_string_create_no_alloc(struct dy_string_data *data)
{
    return (struct dy_core_custom){
        .id = dy_string_id,
        .data = data
    };
}

struct dy_core_expr dy_string_type_of(struct dy_core_ctx *ctx, void *data)
{
    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_CUSTOM,
        .custom = dy_string_type_create()
    };
}

dy_ternary_t dy_string_is_equal(struct dy_core_ctx *ctx, void *data1, void *data2)
{
    const struct dy_string_data *s1 = data1;
    const struct dy_string_data *s2 = data2;

    dy_string_t view1 = {
        .ptr = s1->value.buffer,
        .size = s1->value.num_elems
    };

    dy_string_t view2 = {
        .ptr = s2->value.buffer,
        .size = s2->value.num_elems
    };

    if (dy_string_are_equal(view1, view2)) {
        return DY_YES;
    } else {
        return DY_NO;
    }
}

bool dy_string_check(struct dy_core_ctx *ctx, void *data, struct dy_core_expr *result)
{
    return false;
}

bool dy_string_remove_mentions_in_type(struct dy_core_ctx *ctx, void *data, size_t id, enum dy_polarity current_polarity, struct dy_core_expr *result)
{
    return false;
}

bool dy_string_eval(struct dy_core_ctx *ctx, void *data, bool *is_value, struct dy_core_expr *result)
{
    *is_value = true;
    return false;
}

bool dy_string_substitute(struct dy_core_ctx *ctx, void *data, size_t id, struct dy_core_expr sub, struct dy_core_expr *result)
{
    return false;
}

dy_ternary_t dy_string_is_subtype(struct dy_core_ctx *ctx, void *subtype, void *supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    return dy_string_is_equal(ctx, subtype, supertype);
}

bool dy_string_contains_this_variable(struct dy_core_ctx *ctx, void *data, size_t id)
{
    return false;
}

void dy_string_variable_appears_in_polarity(struct dy_core_ctx *ctx, void *data, size_t id, enum dy_polarity current_polarity, bool *positive, bool *negative)
{
}

void *dy_string_retain(struct dy_core_ctx *ctx, void *data)
{
    return dy_rc_retain(data, DY_ALIGNOF(struct dy_string_data));
}

void dy_string_release(struct dy_core_ctx *ctx, void *data)
{
    dy_rc_release(data, DY_ALIGNOF(struct dy_string_data));
}

void dy_string_to_string(struct dy_core_ctx *ctx, void *data, dy_array_t *string)
{
    struct dy_string_data *s = data;

    dy_array_add(string, &(char){ '\"' });

    for (size_t i = 0; i < s->value.num_elems; ++i) {
        dy_array_add(string, (char *)s->value.buffer + i);
    }

    dy_array_add(string, &(char){ '\"' });
}
