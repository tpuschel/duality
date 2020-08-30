/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "../core/core.h"
#include "ast.h"

static const size_t dy_uv_id = 0;

struct dy_uv_data {
    struct dy_ast_literal var;
};

static struct dy_core_expr dy_uv_type_of(void *data, struct dy_core_ctx *ctx);

static dy_ternary_t dy_uv_is_equal(void *data, struct dy_core_ctx *ctx, struct dy_core_expr expr);

static bool dy_uv_check(void *data, struct dy_core_ctx *ctx, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr *result);

static bool dy_uv_remove_mentions_in_subtype(void *data, struct dy_core_ctx *ctx, size_t id, struct dy_core_expr *result);

static bool dy_uv_remove_mentions_in_supertype(void *data, struct dy_core_ctx *ctx, size_t id, struct dy_core_expr *result);

static struct dy_core_expr dy_uv_eval(void *data, struct dy_core_ctx *ctx, bool *is_value);

static bool dy_uv_substitute(void *data, struct dy_core_ctx *ctx, size_t id, struct dy_core_expr sub, struct dy_core_expr *result);

static dy_ternary_t dy_uv_is_subtype(void *data, struct dy_core_ctx *ctx, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static dy_ternary_t dy_uv_is_supertype(void *data, struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static struct dy_core_expr dy_uv_eliminate(void *data, struct dy_core_ctx *ctx, struct dy_core_expr expr, bool *is_value);

static bool dy_uv_is_computation(void *data);

static bool dy_uv_is_bound(void *data, size_t id);

static void dy_uv_appears_in_polarity(void *data, size_t id, enum dy_core_polarity current_polarity, bool *in_positive, bool *in_negative);

static void *dy_uv_retain(void *data);

static void dy_uv_release(void *data);

static void dy_uv_to_string(void *data, dy_array_t *string);

static inline struct dy_core_custom dy_uv_create(struct dy_uv_data data);

static inline struct dy_core_custom dy_uv_create_no_alloc(const struct dy_uv_data *data);

struct dy_core_custom dy_uv_create(struct dy_uv_data data)
{
    static const size_t pre_padding = DY_RC_PRE_PADDING(struct dy_uv_data);
    static const size_t post_padding = DY_RC_POST_PADDING(struct dy_uv_data);
    return dy_uv_create_no_alloc(dy_rc_new(&data, sizeof data, pre_padding, post_padding));
}

struct dy_core_custom dy_uv_create_no_alloc(const struct dy_uv_data *data)
{
    return (struct dy_core_custom){
        .id = dy_uv_id,
        .data = dy_uv_retain(data),
        .can_be_eliminated = false,
        .type_of = dy_uv_type_of,
        .is_equal = dy_uv_is_equal,
        .check = dy_uv_check,
        .remove_mentions_in_subtype = dy_uv_remove_mentions_in_subtype,
        .remove_mentions_in_supertype = dy_uv_remove_mentions_in_supertype,
        .eval = dy_uv_eval,
        .substitute = dy_uv_substitute,
        .is_subtype = dy_uv_is_subtype,
        .is_supertype = dy_uv_is_supertype,
        .eliminate = dy_uv_eliminate,
        .is_computation = dy_uv_is_computation,
        .is_bound = dy_uv_is_bound,
        .appears_in_polarity = dy_uv_appears_in_polarity,
        .retain = dy_uv_retain,
        .release = dy_uv_release,
        .to_string = dy_uv_to_string
    };
}

struct dy_core_expr dy_uv_type_of(void *data, struct dy_core_ctx *ctx)
{
    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_END,
        .end_polarity = DY_CORE_POLARITY_POSITIVE
    };
}

dy_ternary_t dy_uv_is_equal(void *data, struct dy_core_ctx *ctx, struct dy_core_expr expr)
{
    return DY_NO;
}

bool dy_uv_check(void *data, struct dy_core_ctx *ctx, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr *result)
{
    return false;
}

bool dy_uv_remove_mentions_in_subtype(void *data, struct dy_core_ctx *ctx, size_t id, struct dy_core_expr *result)
{
    return false;
}

bool dy_uv_remove_mentions_in_supertype(void *data, struct dy_core_ctx *ctx, size_t id, struct dy_core_expr *result)
{
    return false;
}

struct dy_core_expr dy_uv_eval(void *data, struct dy_core_ctx *ctx, bool *is_value)
{
    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_CUSTOM,
        .custom = dy_uv_create_no_alloc(data)
    };
}

bool dy_uv_substitute(void *data, struct dy_core_ctx *ctx, size_t id, struct dy_core_expr sub, struct dy_core_expr *result)
{
    return false;
}

dy_ternary_t dy_uv_is_subtype(void *data, struct dy_core_ctx *ctx, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    return DY_NO;
}

dy_ternary_t dy_uv_is_supertype(void *data, struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    return DY_NO;
}

struct dy_core_expr dy_uv_eliminate(void *data, struct dy_core_ctx *ctx, struct dy_core_expr expr, bool *is_value)
{
    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_CUSTOM,
        .custom = dy_uv_create_no_alloc(data)
    };
}

bool dy_uv_is_computation(void *data)
{
    return DY_NO;
}

bool dy_uv_is_bound(void *data, size_t id)
{
    return false;
}

void dy_uv_appears_in_polarity(void *data, size_t id, enum dy_core_polarity current_polarity, bool *in_positive, bool *in_negative)
{
    return;
}

void *dy_uv_retain(void *data)
{
    static const size_t pre_padding = DY_RC_PRE_PADDING(struct dy_uv_data);
    static const size_t post_padding = DY_RC_POST_PADDING(struct dy_uv_data);
    return dy_rc_retain(data, pre_padding, post_padding);
}

void dy_uv_release(void *data)
{
    static const size_t pre_padding = DY_RC_PRE_PADDING(struct dy_uv_data);
    static const size_t post_padding = DY_RC_POST_PADDING(struct dy_uv_data);
    dy_rc_release(data, pre_padding, post_padding);
}

void dy_uv_to_string(void *data, dy_array_t *string)
{
    struct dy_uv_data *d = data;

    for (size_t i = 0; i < d->var.value.size; ++i) {
        dy_array_add(string, d->var.value.ptr + i);
    }
}
