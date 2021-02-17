/*
 * Copyright 2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "../core/core.h"

static size_t dy_string_type_id;

static struct dy_core_expr dy_string_type_type_of(struct dy_core_ctx *ctx, void *data);

static dy_ternary_t dy_string_type_is_equal(struct dy_core_ctx *ctx, void *data1, void *data2);

static bool dy_string_type_check(struct dy_core_ctx *ctx, void *data, struct dy_core_expr *result);

static bool dy_string_type_remove_mentions_in_type(struct dy_core_ctx *ctx, void *data, size_t id, enum dy_polarity current_polarity, struct dy_core_expr *result);

static bool dy_string_type_eval(struct dy_core_ctx *ctx, void *data, bool *is_value, struct dy_core_expr *result);

static bool dy_string_type_substitute(struct dy_core_ctx *ctx, void *data, size_t id, struct dy_core_expr sub, struct dy_core_expr *result);

static dy_ternary_t dy_string_type_is_subtype(struct dy_core_ctx *ctx, void *subtype, void *supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static bool dy_string_type_contains_this_variable(struct dy_core_ctx *ctx, void *data, size_t id);

static void dy_string_type_variable_appears_in_polarity(struct dy_core_ctx *ctx, void *data, size_t id, enum dy_polarity current_polarity, bool *positive, bool *negative);

static void *dy_string_type_retain(struct dy_core_ctx *ctx, void *data);

static void dy_string_type_release(struct dy_core_ctx *ctx, void *data);

static void dy_string_type_to_string(struct dy_core_ctx *ctx, void *data, dy_array_t *string);

static inline struct dy_core_custom dy_string_type_create(void);

static inline void dy_string_type_register(dy_array_t *reg)
{
    struct dy_core_custom_shared s = {
        .type_of = dy_string_type_type_of,
        .is_equal = dy_string_type_is_equal,
        .check = dy_string_type_check,
        .remove_mentions_in_type = dy_string_type_remove_mentions_in_type,
        .eval = dy_string_type_eval,
        .substitute = dy_string_type_substitute,
        .is_subtype = dy_string_type_is_subtype,
        .contains_this_variable = dy_string_type_contains_this_variable,
        .variable_appears_in_polarity = dy_string_type_variable_appears_in_polarity,
        .retain = dy_string_type_retain,
        .release = dy_string_type_release,
        .to_string = dy_string_type_to_string
    };

    dy_string_type_id = dy_array_add(reg, &s);
}

struct dy_core_custom dy_string_type_create(void)
{
    return (struct dy_core_custom){
        .id = dy_string_type_id,
        .data = NULL
    };
}

struct dy_core_expr dy_string_type_type_of(struct dy_core_ctx *ctx, void *data)
{
    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_VOID
    };
}

dy_ternary_t dy_string_type_is_equal(struct dy_core_ctx *ctx, void *data1, void *data2)
{
    return DY_YES;
}

bool dy_string_type_check(struct dy_core_ctx *ctx, void *data, struct dy_core_expr *result)
{
    return false;
}

bool dy_string_type_remove_mentions_in_type(struct dy_core_ctx *ctx, void *data, size_t id, enum dy_polarity current_polarity, struct dy_core_expr *result)
{
    return false;
}

bool dy_string_type_eval(struct dy_core_ctx *ctx, void *data, bool *is_value, struct dy_core_expr *result)
{
    *is_value = true;
    return false;
}

bool dy_string_type_substitute(struct dy_core_ctx *ctx, void *data, size_t id, struct dy_core_expr sub, struct dy_core_expr *result)
{
    return false;
}

dy_ternary_t dy_string_type_is_subtype(struct dy_core_ctx *ctx, void *subtype, void *supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    return DY_YES;
}

bool dy_string_type_contains_this_variable(struct dy_core_ctx *ctx, void *data, size_t id)
{
    return false;
}

void dy_string_type_variable_appears_in_polarity(struct dy_core_ctx *ctx, void *data, size_t id, enum dy_polarity current_polarity, bool *positive, bool *negative)
{
}

void *dy_string_type_retain(struct dy_core_ctx *ctx, void *data)
{
    return data;
}

void dy_string_type_release(struct dy_core_ctx *ctx, void *data)
{

}

void dy_string_type_to_string(struct dy_core_ctx *ctx, void *data, dy_array_t *string)
{
    add_string(string, DY_STR_LIT("String"));
}
