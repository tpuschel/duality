/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "../core/check.h"
#include "../core/eval.h"

static size_t dy_def_id;

struct dy_def_data {
    size_t id;
    struct dy_core_expr arg;
    struct dy_core_expr body;
};

static const size_t dy_def_data_align = DY_ALIGNOF(struct dy_def_data);

static struct dy_core_expr dy_def_type_of(struct dy_core_ctx *ctx, void *data);

static dy_ternary_t dy_def_is_equal(struct dy_core_ctx *ctx, void *data1, void *data2);

static bool dy_def_check(struct dy_core_ctx *ctx, void *data, struct dy_core_expr *result);

static bool dy_def_remove_mentions_in_type(struct dy_core_ctx *ctx, void *data, size_t id, enum dy_polarity current_polarity, struct dy_core_expr *result);

static bool dy_def_eval(struct dy_core_ctx *ctx, void *data, bool *is_value, struct dy_core_expr *result);

static bool dy_def_substitute(struct dy_core_ctx *ctx, void *data, size_t id, struct dy_core_expr sub, struct dy_core_expr *result);

static dy_ternary_t dy_def_is_subtype(struct dy_core_ctx *ctx, void *subtype, void *supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static bool dy_def_contains_this_variable(struct dy_core_ctx *ctx, void *data, size_t id);

static void dy_def_variable_appears_in_polarity(struct dy_core_ctx *ctx, void *data, size_t id, enum dy_polarity current_polarity, bool *positive, bool *negative);

static void *dy_def_retain(struct dy_core_ctx *ctx, void *data);

static void dy_def_release(struct dy_core_ctx *ctx, void *data);

static void dy_def_to_string(struct dy_core_ctx *ctx, void *data, dy_array_t *string);

static inline struct dy_core_custom dy_def_create(struct dy_def_data data);

static inline struct dy_core_custom dy_def_create_no_alloc(struct dy_def_data *data);

static inline void dy_def_register(dy_array_t *reg)
{
    struct dy_core_custom_shared s = {
        .type_of = dy_def_type_of,
        .is_equal = dy_def_is_equal,
        .check = dy_def_check,
        .remove_mentions_in_type = dy_def_remove_mentions_in_type,
        .eval = dy_def_eval,
        .substitute = dy_def_substitute,
        .is_subtype = dy_def_is_subtype,
        .contains_this_variable = dy_def_contains_this_variable,
        .variable_appears_in_polarity = dy_def_variable_appears_in_polarity,
        .retain = dy_def_retain,
        .release = dy_def_release,
        .to_string = dy_def_to_string
    };

    dy_def_id = dy_array_add(reg, &s);
}

struct dy_core_custom dy_def_create(struct dy_def_data data)
{
    return dy_def_create_no_alloc(dy_rc_new(&data, sizeof data, dy_def_data_align));
}

struct dy_core_custom dy_def_create_no_alloc(struct dy_def_data *data)
{
    return (struct dy_core_custom){
        .id = dy_def_id,
        .data = data
    };
}

struct dy_core_expr dy_def_type_of(struct dy_core_ctx *ctx, void *data)
{
    const struct dy_def_data *def = data;

    struct dy_core_expr new_body;
    if (!dy_substitute(ctx, def->body, def->id, def->arg, &new_body)) {
        new_body = dy_core_expr_retain(ctx, def->body);
    }

    struct dy_core_expr ret = dy_type_of(ctx, new_body);

    dy_core_expr_release(ctx, new_body);

    return ret;
}

dy_ternary_t dy_def_is_equal(struct dy_core_ctx *ctx, void *data1, void *data2)
{
    return DY_MAYBE;
}

bool dy_def_check(struct dy_core_ctx *ctx, void *data, struct dy_core_expr *result)
{
    const struct dy_def_data *def = data;

    size_t constraint_start1 = ctx->constraints.num_elems;
    struct dy_core_expr new_arg;
    if (!dy_check_expr(ctx, def->arg, &new_arg)) {
        new_arg = dy_core_expr_retain(ctx, def->arg);
    }

    bool arg_is_value = false;
    struct dy_core_expr evaled_arg;
    if (!dy_eval_expr(ctx, new_arg, &arg_is_value, &evaled_arg)) {
        evaled_arg = dy_core_expr_retain(ctx, new_arg);
    }

    dy_core_expr_release(ctx, new_arg);

    if (arg_is_value) {
        struct dy_core_expr new_body;
        if (!dy_substitute(ctx, def->body, def->id, evaled_arg, &new_body)) {
            new_body = dy_core_expr_retain(ctx, def->body);
        }

        dy_core_expr_release(ctx, evaled_arg);

        size_t constraint_start2 = ctx->constraints.num_elems;
        struct dy_core_expr checked_body;
        if (dy_check_expr(ctx, new_body, &checked_body)) {
            dy_core_expr_release(ctx, new_body);
            new_body = checked_body;
        }

        dy_join_constraints(ctx, constraint_start1, constraint_start2, DY_POLARITY_POSITIVE);

        *result = new_body;

        return true;
    } else {
        struct dy_core_expr type_of_arg = dy_type_of(ctx, evaled_arg);

        dy_array_add(&ctx->free_variables, &(struct dy_free_var){
            .id = def->id,
            .type = type_of_arg
        });

        size_t constraint_start2 = ctx->constraints.num_elems;
        struct dy_core_expr checked_body;
        if (!dy_check_expr(ctx, def->body, &checked_body)) {
            checked_body = dy_core_expr_retain(ctx, def->body);
        }

        --ctx->free_variables.num_elems;

        dy_join_constraints(ctx, constraint_start1, constraint_start2, DY_POLARITY_POSITIVE);

        dy_core_expr_release(ctx, type_of_arg);

        struct dy_def_data new_data = {
            .id = def->id,
            .arg = evaled_arg,
            .body = checked_body
        };

        *result = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_CUSTOM,
            .custom = dy_def_create(new_data)
        };

        return true;
    }
}

bool dy_def_remove_mentions_in_type(struct dy_core_ctx *ctx, void *data, size_t id, enum dy_polarity current_polarity, struct dy_core_expr *result)
{
    if (dy_def_contains_this_variable(ctx, data, id)) {
        *result = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_ANY
        };
        return true;
    } else {
        return false;
    }
}

bool dy_def_eval(struct dy_core_ctx *ctx, void *data, bool *is_value, struct dy_core_expr *result)
{
    const struct dy_def_data *def = data;

    bool arg_is_value = false;
    struct dy_core_expr evaled_arg;
    if (!dy_eval_expr(ctx, def->arg, &arg_is_value, &evaled_arg)) {
        evaled_arg = dy_core_expr_retain(ctx, def->arg);
    }

    if (!arg_is_value) {
        struct dy_def_data new_data = {
            .id = def->id,
            .arg = evaled_arg,
            .body = dy_core_expr_retain(ctx, def->body)
        };

        *result = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_CUSTOM,
            .custom = dy_def_create(new_data)
        };

        return true;
    }

    struct dy_core_expr new_body;
    if (!dy_substitute(ctx, def->body, def->id, evaled_arg, &new_body)) {
        new_body = dy_core_expr_retain(ctx, def->body);
    }

    dy_core_expr_release(ctx, evaled_arg);

    bool ret = dy_eval_expr(ctx, new_body, is_value, result);

    dy_core_expr_release(ctx, new_body);

    return ret;
}

bool dy_def_substitute(struct dy_core_ctx *ctx, void *data, size_t id, struct dy_core_expr sub, struct dy_core_expr *result)
{
    const struct dy_def_data *def = data;

    struct dy_core_expr arg;
    bool arg_is_new = dy_substitute(ctx, def->arg, id, sub, &arg);

    if (id == def->id) {
        if (!arg_is_new) {
            return false;
        }

        struct dy_def_data new_data = {
            .id = def->id,
            .arg = arg,
            .body = dy_core_expr_retain(ctx, def->body)
        };

        *result = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_CUSTOM,
            .custom = dy_def_create(new_data)
        };

        return true;
    }

    if (dy_core_expr_contains_this_variable(ctx, def->id, sub)) {
        dy_bail("shit");
    }

    struct dy_core_expr body;
    bool body_is_new = dy_substitute(ctx, def->body, id, sub, &body);

    if (!arg_is_new && !body_is_new) {
        return false;
    }

    if (!arg_is_new) {
        arg = dy_core_expr_retain(ctx, def->arg);
    }

    if (!body_is_new) {
        body = dy_core_expr_retain(ctx, def->body);
    }

    struct dy_def_data new_data = {
        .id = def->id,
        .arg = arg,
        .body = body
    };

    *result = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_CUSTOM,
        .custom = dy_def_create(new_data)
    };

    return true;
}

dy_ternary_t dy_def_is_subtype(struct dy_core_ctx *ctx, void *subtype, void *supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    return DY_MAYBE;
}

bool dy_def_contains_this_variable(struct dy_core_ctx *ctx, void *data, size_t id)
{
    const struct dy_def_data *def = data;

    if (dy_core_expr_contains_this_variable(ctx, id, def->arg)) {
        return true;
    }

    if (id == def->id) {
        return false;
    }

    return dy_core_expr_contains_this_variable(ctx, id, def->body);
}

void dy_def_variable_appears_in_polarity(struct dy_core_ctx *ctx, void *data, size_t id, enum dy_polarity current_polarity, bool *positive, bool *negative)
{
}

void *dy_def_retain(struct dy_core_ctx *ctx, void *data)
{
    return dy_rc_retain(data, dy_def_data_align);
}

void dy_def_release(struct dy_core_ctx *ctx, void *data)
{
    const struct dy_def_data *d = data;

    if (dy_rc_release(data, dy_def_data_align) == 0) {
        dy_core_expr_release(ctx, d->arg);
        dy_core_expr_release(ctx, d->body);
    }
}

void dy_def_to_string(struct dy_core_ctx *ctx, void *data, dy_array_t *string)
{
    const struct dy_def_data *d = data;

    add_string(string, DY_STR_LIT("def "));
    add_size_t_decimal(string, d->id);
    add_string(string, DY_STR_LIT(" = "));
    dy_core_expr_to_string(ctx, d->arg, string);
    add_string(string, DY_STR_LIT("\n"));
    dy_core_expr_to_string(ctx, d->body, string);
}
