/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "../core/core.h"

static size_t dy_uv_id;

struct dy_uv_data {
    dy_array_t var;
};

static struct dy_core_expr dy_uv_type_of(struct dy_core_ctx *ctx, void *data);

static dy_ternary_t dy_uv_is_equal(struct dy_core_ctx *ctx, void *data1, void *data2);

static bool dy_uv_check(struct dy_core_ctx *ctx, void *data, struct dy_core_expr *result);

static bool dy_uv_remove_mentions_in_type(struct dy_core_ctx *ctx, void *data, size_t id, enum dy_polarity current_polarity, struct dy_core_expr *result);

static bool dy_uv_eval(struct dy_core_ctx *ctx, void *data, bool *is_value, struct dy_core_expr *result);

static bool dy_uv_substitute(struct dy_core_ctx *ctx, void *data, size_t id, struct dy_core_expr sub, struct dy_core_expr *result);

static dy_ternary_t dy_uv_is_subtype(struct dy_core_ctx *ctx, void *subtype, void *supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static bool dy_uv_contains_this_variable(struct dy_core_ctx *ctx, void *data, size_t id);

static void dy_uv_variable_appears_in_polarity(struct dy_core_ctx *ctx, void *data, size_t id, enum dy_polarity current_polarity, bool *positive, bool *negative);

static void *dy_uv_retain(struct dy_core_ctx *ctx, void *data);

static void dy_uv_release(struct dy_core_ctx *ctx, void *data);

static void dy_uv_to_string(struct dy_core_ctx *ctx, void *data, dy_array_t *string);

static inline struct dy_core_custom dy_uv_create(struct dy_uv_data data);

static inline struct dy_core_custom dy_uv_create_no_alloc(struct dy_uv_data *data);

static inline void dy_uv_register(dy_array_t *reg)
{
    struct dy_core_custom_shared s = {
        .type_of = dy_uv_type_of,
        .is_equal = dy_uv_is_equal,
        .check = dy_uv_check,
        .remove_mentions_in_type = dy_uv_remove_mentions_in_type,
        .eval = dy_uv_eval,
        .substitute = dy_uv_substitute,
        .is_subtype = dy_uv_is_subtype,
        .contains_this_variable = dy_uv_contains_this_variable,
        .variable_appears_in_polarity = dy_uv_variable_appears_in_polarity,
        .retain = dy_uv_retain,
        .release = dy_uv_release,
        .to_string = dy_uv_to_string
    };

    dy_uv_id = dy_array_add(reg, &s);
}

struct dy_core_custom dy_uv_create(struct dy_uv_data data)
{
    return dy_uv_create_no_alloc(dy_rc_new(&data, sizeof data, DY_ALIGNOF(data)));
}

struct dy_core_custom dy_uv_create_no_alloc(struct dy_uv_data *data)
{
    return (struct dy_core_custom){
        .id = dy_uv_id,
        .data = data
    };
}

struct dy_core_expr dy_uv_type_of(struct dy_core_ctx *ctx, void *data)
{
    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_ANY
    };
}

dy_ternary_t dy_uv_is_equal(struct dy_core_ctx *ctx, void *data1, void *data2)
{
    return DY_NO;
}

bool dy_uv_check(struct dy_core_ctx *ctx, void *data, struct dy_core_expr *result)
{
    return false;
}

bool dy_uv_remove_mentions_in_type(struct dy_core_ctx *ctx, void *data, size_t id, enum dy_polarity current_polarity, struct dy_core_expr *result)
{
    return false;
}

bool dy_uv_eval(struct dy_core_ctx *ctx, void *data, bool *is_value, struct dy_core_expr *result)
{
    return false;
}

bool dy_uv_substitute(struct dy_core_ctx *ctx, void *data, size_t id, struct dy_core_expr sub, struct dy_core_expr *result)
{
    return false;
}

dy_ternary_t dy_uv_is_subtype(struct dy_core_ctx *ctx, void *subtype, void *supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    return DY_NO;
}

bool dy_uv_contains_this_variable(struct dy_core_ctx *ctx, void *data, size_t id)
{
    return false;
}

void dy_uv_variable_appears_in_polarity(struct dy_core_ctx *ctx, void *data, size_t id, enum dy_polarity current_polarity, bool *positive, bool *negative)
{
}

void *dy_uv_retain(struct dy_core_ctx *ctx, void *data)
{
    return dy_rc_retain(data, DY_ALIGNOF(struct dy_uv_data));
}

void dy_uv_release(struct dy_core_ctx *ctx, void *data)
{
    dy_rc_release(data, DY_ALIGNOF(struct dy_uv_data));
}

void dy_uv_to_string(struct dy_core_ctx *ctx, void *data, dy_array_t *string)
{
    struct dy_uv_data *d = data;

    for (size_t i = 0; i < d->var.num_elems; ++i) {
        dy_array_add(string, (char *)d->var.buffer + i);
    }
}
