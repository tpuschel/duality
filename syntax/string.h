/*
 * Copyright 2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "../core/check.h"
#include "../core/eval.h"
#include "ast.h"

/**
 * Support for strings. Just for testing puposes, for now.
 */

static size_t dy_string_id;

struct dy_string_data {
    dy_string_t value;
};

static struct dy_core_expr dy_string_type_of(void *data, struct dy_core_ctx *ctx);

static dy_ternary_t dy_string_is_equal(void *data, struct dy_core_ctx *ctx, struct dy_core_expr expr);

static bool dy_string_check(void *data, struct dy_core_ctx *ctx, struct dy_core_expr *result);

static bool dy_string_remove_mentions_in_subtype(void *data, struct dy_core_ctx *ctx, size_t id, struct dy_core_expr *result);

static bool dy_string_remove_mentions_in_supertype(void *data, struct dy_core_ctx *ctx, size_t id, struct dy_core_expr *result);

static struct dy_core_expr dy_string_eval(void *data, struct dy_core_ctx *ctx, bool *is_value);

static bool dy_string_substitute(void *data, struct dy_core_ctx *ctx, size_t id, struct dy_core_expr sub, struct dy_core_expr *result);

static dy_ternary_t dy_string_is_subtype(void *data, struct dy_core_ctx *ctx, struct dy_core_expr supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static dy_ternary_t dy_string_is_supertype(void *data, struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static struct dy_core_expr dy_string_eliminate(void *data, struct dy_core_ctx *ctx, struct dy_core_expr expr, bool *is_value);

static bool dy_string_is_computation(void *data, struct dy_core_ctx *ctx);

static bool dy_string_is_bound(void *data, struct dy_core_ctx *ctx, size_t id);

static void dy_string_appears_in_polarity(void *data, struct dy_core_ctx *ctx, size_t id, enum dy_core_polarity current_polarity, bool *in_positive, bool *in_negative);

static void *dy_string_retain(void *data, struct dy_core_ctx *ctx);

static void dy_string_release(void *data, struct dy_core_ctx *ctx);

static void dy_string_to_string(void *data, struct dy_core_ctx *ctx, dy_array_t *string);

static inline struct dy_core_custom dy_string_create(struct dy_string_data data);

static inline struct dy_core_custom dy_string_create_no_alloc(const struct dy_string_data *data);

static inline void dy_string_register(struct dy_core_ctx *ctx)
{
    struct dy_core_custom_shared s = {
        .can_be_eliminated = false,
        .type_of = dy_string_type_of,
        .is_equal = dy_string_is_equal,
        .check = dy_string_check,
        .remove_mentions_in_subtype = dy_string_remove_mentions_in_subtype,
        .remove_mentions_in_supertype = dy_string_remove_mentions_in_supertype,
        .eval = dy_string_eval,
        .substitute = dy_string_substitute,
        .is_subtype = dy_string_is_subtype,
        .is_supertype = dy_string_is_supertype,
        .eliminate = dy_string_eliminate,
        .is_computation = dy_string_is_computation,
        .is_bound = dy_string_is_bound,
        .appears_in_polarity = dy_string_appears_in_polarity,
        .retain = dy_string_retain,
        .release = dy_string_release,
        .to_string = dy_string_to_string
    };

    dy_string_id = dy_array_add(&ctx->custom_shared, &s);
}

struct dy_core_custom dy_string_create(struct dy_string_data data)
{
    static const size_t pre_padding = DY_RC_PRE_PADDING(struct dy_string_data);
    static const size_t post_padding = DY_RC_POST_PADDING(struct dy_string_data);
    return dy_string_create_no_alloc(dy_rc_new(&data, sizeof data, pre_padding, post_padding));
}

struct dy_core_custom dy_string_create_no_alloc(const struct dy_string_data *data)
{
    return (struct dy_core_custom){
        .id = dy_string_id,
        .data = dy_string_retain(data, NULL)
    };
}

struct dy_core_expr dy_string_type_of(void *data, struct dy_core_ctx *ctx)
{
    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_SYMBOL
    };
}

dy_ternary_t dy_string_is_equal(void *data, struct dy_core_ctx *ctx, struct dy_core_expr expr)
{
    struct dy_string_data *s = data;

    if (expr.tag == DY_CORE_EXPR_CUSTOM && expr.custom.id == dy_string_id) {
        struct dy_string_data *s2 = expr.custom.data;
        if (dy_string_are_equal(s->value, s2->value)) {
            return DY_YES;
        } else {
            return DY_NO;
        }
    }

    switch (expr.tag) {
    case DY_CORE_EXPR_EQUALITY_MAP_ELIM:
    case DY_CORE_EXPR_ALTERNATIVE:
    case DY_CORE_EXPR_TYPE_MAP_ELIM:
    case DY_CORE_EXPR_VARIABLE:
        return DY_MAYBE;
    default:
        return DY_NO;
    }

    return DY_NO;
}

bool dy_string_check(void *data, struct dy_core_ctx *ctx, struct dy_core_expr *result)
{
    return false;
}

bool dy_string_remove_mentions_in_subtype(void *data, struct dy_core_ctx *ctx, size_t id, struct dy_core_expr *result)
{
    return false;
}

bool dy_string_remove_mentions_in_supertype(void *data, struct dy_core_ctx *ctx, size_t id, struct dy_core_expr *result)
{
    return false;
}

struct dy_core_expr dy_string_eval(void *data, struct dy_core_ctx *ctx, bool *is_value)
{
    *is_value = true;

    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_CUSTOM,
        .custom = dy_string_create_no_alloc(data)
    };
}

bool dy_string_substitute(void *data, struct dy_core_ctx *ctx, size_t id, struct dy_core_expr sub, struct dy_core_expr *result)
{
    return false;
}

dy_ternary_t dy_string_is_subtype(void *data, struct dy_core_ctx *ctx, struct dy_core_expr supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    return DY_NO;
}

dy_ternary_t dy_string_is_supertype(void *data, struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    return DY_NO;
}

struct dy_core_expr dy_string_eliminate(void *data, struct dy_core_ctx *ctx, struct dy_core_expr expr, bool *is_value)
{
    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_CUSTOM,
        .custom = dy_string_create_no_alloc(data)
    };
}

bool dy_string_is_computation(void *data, struct dy_core_ctx *ctx)
{
    return false;
}

bool dy_string_is_bound(void *data, struct dy_core_ctx *ctx, size_t id)
{
    return false;
}

void dy_string_appears_in_polarity(void *data, struct dy_core_ctx *ctx, size_t id, enum dy_core_polarity current_polarity, bool *in_positive, bool *in_negative)
{
    return;
}

void *dy_string_retain(void *data, struct dy_core_ctx *ctx)
{
    static const size_t pre_padding = DY_RC_PRE_PADDING(struct dy_string_data);
    static const size_t post_padding = DY_RC_POST_PADDING(struct dy_string_data);
    return dy_rc_retain(data, pre_padding, post_padding);
}

void dy_string_release(void *data, struct dy_core_ctx *ctx)
{
    static const size_t pre_padding = DY_RC_PRE_PADDING(struct dy_string_data);
    static const size_t post_padding = DY_RC_POST_PADDING(struct dy_string_data);
    dy_rc_release(data, pre_padding, post_padding);
}

void dy_string_to_string(void *data, struct dy_core_ctx *ctx, dy_array_t *string)
{
    struct dy_string_data *s = data;

    dy_array_add(string, &(char){ '\"' });

    for (size_t i = 0; i < s->value.size; ++i) {
        dy_array_add(string, s->value.ptr + i);
    }

    dy_array_add(string, &(char){ '\"' });
}
