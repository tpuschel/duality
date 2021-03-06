/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "../core/core.h"
#include "../core/check.h"

#include "string.h"

static size_t dy_print_id;

struct dy_print_data {
    struct dy_core_expr expr;
};

static struct dy_core_expr dy_print_type_of(struct dy_core_ctx *ctx, void *data);

static dy_ternary_t dy_print_is_equal(struct dy_core_ctx *ctx, void *data1, void *data2);

static bool dy_print_check(struct dy_core_ctx *ctx, void *data, struct dy_core_expr *result);

static bool dy_print_remove_mentions_in_type(struct dy_core_ctx *ctx, void *data, size_t id, enum dy_polarity current_polarity, struct dy_core_expr *result);

static bool dy_print_eval(struct dy_core_ctx *ctx, void *data, bool *is_value, struct dy_core_expr *result);

static bool dy_print_substitute(struct dy_core_ctx *ctx, void *data, size_t id, struct dy_core_expr sub, struct dy_core_expr *result);

static dy_ternary_t dy_print_is_subtype(struct dy_core_ctx *ctx, void *subtype, void *supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static bool dy_print_contains_this_variable(struct dy_core_ctx *ctx, void *data, size_t id);

static void dy_print_variable_appears_in_polarity(struct dy_core_ctx *ctx, void *data, size_t id, enum dy_polarity current_polarity, bool *positive, bool *negative);

static void *dy_print_retain(struct dy_core_ctx *ctx, void *data);

static void dy_print_release(struct dy_core_ctx *ctx, void *data);

static void dy_print_to_string(struct dy_core_ctx *ctx, void *data, dy_array_t *string);

static inline struct dy_core_custom dy_print_create(struct dy_print_data data);

static inline struct dy_core_custom dy_print_create_no_alloc(struct dy_print_data *data);

static inline void dy_print_register(dy_array_t *reg)
{
    struct dy_core_custom_shared s = {
        .type_of = dy_print_type_of,
        .is_equal = dy_print_is_equal,
        .check = dy_print_check,
        .remove_mentions_in_type = dy_print_remove_mentions_in_type,
        .eval = dy_print_eval,
        .substitute = dy_print_substitute,
        .is_subtype = dy_print_is_subtype,
        .contains_this_variable = dy_print_contains_this_variable,
        .variable_appears_in_polarity = dy_print_variable_appears_in_polarity,
        .retain = dy_print_retain,
        .release = dy_print_release,
        .to_string = dy_print_to_string
    };

    dy_print_id = dy_array_add(reg, &s);
}

struct dy_core_custom dy_print_create(struct dy_print_data data)
{
    return dy_print_create_no_alloc(dy_rc_new(&data, sizeof data, DY_ALIGNOF(data)));
}

struct dy_core_custom dy_print_create_no_alloc(struct dy_print_data *data)
{
    return (struct dy_core_custom){
        .id = dy_print_id,
        .data = data
    };
}

struct dy_core_expr dy_print_type_of(struct dy_core_ctx *ctx, void *data)
{
    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_VOID
    };
}

dy_ternary_t dy_print_is_equal(struct dy_core_ctx *ctx, void *data1, void *data2)
{
    return DY_NO;
}

bool dy_print_check(struct dy_core_ctx *ctx, void *data, struct dy_core_expr *result)
{
    return false;
}

bool dy_print_remove_mentions_in_type(struct dy_core_ctx *ctx, void *data, size_t id, enum dy_polarity current_polarity, struct dy_core_expr *result)
{
    return false;
}

bool dy_print_eval(struct dy_core_ctx *ctx, void *data, bool *is_value, struct dy_core_expr *result)
{
    const struct dy_print_data *d = data;

    if (d->expr.tag != DY_CORE_EXPR_CUSTOM || d->expr.custom.id != dy_string_id) {
        return false;
    }

    const struct dy_string_data *string_data = d->expr.custom.data;

    for (size_t i = 0; i < string_data->value.num_elems; ++i) {
        printf("%c", *(char *)dy_array_pos(&string_data->value, i));
    }
    printf("\n");

    *is_value = true;
    *result = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_VOID
    };
    return true;
}

bool dy_print_substitute(struct dy_core_ctx *ctx, void *data, size_t id, struct dy_core_expr sub, struct dy_core_expr *result)
{
    struct dy_print_data d = *(struct dy_print_data *)data;

    if (!dy_substitute(ctx, d.expr, id, sub, &d.expr)) {
        return false;
    }

    *result = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_CUSTOM,
        .custom = dy_print_create(d)
    };

    return true;
}

dy_ternary_t dy_print_is_subtype(struct dy_core_ctx *ctx, void *subtype, void *supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    return DY_NO;
}

bool dy_print_contains_this_variable(struct dy_core_ctx *ctx, void *data, size_t id)
{
    const struct dy_print_data *d = data;
    return dy_core_expr_contains_this_variable(ctx, id, d->expr);
}

void dy_print_variable_appears_in_polarity(struct dy_core_ctx *ctx, void *data, size_t id, enum dy_polarity current_polarity, bool *positive, bool *negative)
{
}

void *dy_print_retain(struct dy_core_ctx *ctx, void *data)
{
    return dy_rc_retain(data, DY_ALIGNOF(struct dy_print_data));
}

void dy_print_release(struct dy_core_ctx *ctx, void *data)
{
    dy_rc_release(data, DY_ALIGNOF(struct dy_print_data));
}

void dy_print_to_string(struct dy_core_ctx *ctx, void *data, dy_array_t *string)
{
    struct dy_print_data *d = data;

    add_string(string, DY_STR_LIT("<print: "));

    dy_core_expr_to_string(ctx, d->expr, string);

    add_string(string, DY_STR_LIT(">"));
}
