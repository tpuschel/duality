/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "../core/check.h"
#include "../core/eval.h"
#include "ast.h"

static const size_t dy_def_id = 1;

struct dy_def_data {
    size_t id;
    struct dy_core_expr arg;
    struct dy_core_expr body;
};

static struct dy_core_expr dy_def_type_of(void *data, struct dy_core_ctx *ctx);

static dy_ternary_t dy_def_is_equal(void *data, struct dy_core_ctx *ctx, struct dy_core_expr expr);

static bool dy_def_check(void *data, struct dy_core_ctx *ctx, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr *result);

static bool dy_def_remove_mentions_in_subtype(void *data, struct dy_core_ctx *ctx, size_t id, struct dy_core_expr *result);

static bool dy_def_remove_mentions_in_supertype(void *data, struct dy_core_ctx *ctx, size_t id, struct dy_core_expr *result);

static struct dy_core_expr dy_def_eval(void *data, struct dy_core_ctx *ctx, bool *is_value);

static bool dy_def_substitute(void *data, struct dy_core_ctx *ctx, size_t id, struct dy_core_expr sub, struct dy_core_expr *result);

static dy_ternary_t dy_def_is_subtype(void *data, struct dy_core_ctx *ctx, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static dy_ternary_t dy_def_is_supertype(void *data, struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static struct dy_core_expr dy_def_eliminate(void *data, struct dy_core_ctx *ctx, struct dy_core_expr expr, bool *is_value);

static bool dy_def_is_computation(void *data);

static bool dy_def_is_bound(void *data, size_t id);

static void dy_def_appears_in_polarity(void *data, size_t id, enum dy_core_polarity current_polarity, bool *in_positive, bool *in_negative);

static void *dy_def_retain(void *data);

static void dy_def_release(void *data);

static void dy_def_to_string(void *data, dy_array_t *string);

static inline struct dy_core_custom dy_def_create(struct dy_def_data data);

static inline struct dy_core_custom dy_def_create_no_alloc(const struct dy_def_data *data);

struct dy_core_custom dy_def_create(struct dy_def_data data)
{
    static const size_t pre_padding = DY_RC_PRE_PADDING(struct dy_def_data);
    static const size_t post_padding = DY_RC_POST_PADDING(struct dy_def_data);
    return dy_def_create_no_alloc(dy_rc_new(&data, sizeof data, pre_padding, post_padding));
}

struct dy_core_custom dy_def_create_no_alloc(const struct dy_def_data *data)
{
    return (struct dy_core_custom){
        .id = dy_def_id,
        .data = dy_def_retain(data),
        .can_be_eliminated = false,
        .type_of = dy_def_type_of,
        .is_equal = dy_def_is_equal,
        .check = dy_def_check,
        .remove_mentions_in_subtype = dy_def_remove_mentions_in_subtype,
        .remove_mentions_in_supertype = dy_def_remove_mentions_in_supertype,
        .eval = dy_def_eval,
        .substitute = dy_def_substitute,
        .is_subtype = dy_def_is_subtype,
        .is_supertype = dy_def_is_supertype,
        .eliminate = dy_def_eliminate,
        .is_computation = dy_def_is_computation,
        .is_bound = dy_def_is_bound,
        .appears_in_polarity = dy_def_appears_in_polarity,
        .retain = dy_def_retain,
        .release = dy_def_release,
        .to_string = dy_def_to_string
    };
}

struct dy_core_expr dy_def_type_of(void *data, struct dy_core_ctx *ctx)
{
    struct dy_def_data *def = data;

    struct dy_core_expr new_body;
    if (!substitute(ctx, def->body, def->id, def->arg, &new_body)) {
        new_body = dy_core_expr_retain(def->body);
    }

    struct dy_core_expr ret = dy_type_of(ctx, new_body);

    dy_core_expr_release(new_body);

    return ret;
}

dy_ternary_t dy_def_is_equal(void *data, struct dy_core_ctx *ctx, struct dy_core_expr expr)
{
    return DY_MAYBE;
}

bool dy_def_check(void *data, struct dy_core_ctx *ctx, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr *result)
{
    struct dy_def_data *def = data;

    struct dy_constraint c1;
    bool have_c1 = false;
    struct dy_core_expr new_arg;
    if (!dy_check_expr(ctx, def->arg, &c1, &have_c1, &new_arg)) {
        new_arg = dy_core_expr_retain(def->arg);
    }

    bool arg_is_value = false;
    struct dy_core_expr evaled_arg = dy_eval_expr(ctx, new_arg, &arg_is_value);

    dy_core_expr_release(new_arg);

    if (arg_is_value) {
        struct dy_core_expr new_body;
        if (!substitute(ctx, def->body, def->id, evaled_arg, &new_body)) {
            new_body = dy_core_expr_retain(def->body);
        }

        struct dy_constraint c2;
        bool have_c2 = false;
        struct dy_core_expr checked_body;
        if (dy_check_expr(ctx, new_body, &c2, &have_c2, &checked_body)) {
            dy_core_expr_release(new_body);
            new_body = checked_body;
        }

        dy_join_constraints(c1, have_c1, DY_CORE_POLARITY_POSITIVE, c2, have_c2, constraint, did_generate_constraint);

        *result = new_body;
        
        return true;
    } else {
        struct dy_core_expr type_of_arg = dy_type_of(ctx, evaled_arg);
        
        dy_array_add(&ctx->bindings, &(struct dy_core_binding){
            .id = def->id,
            .type = type_of_arg,
            .is_inference_var = false
        });

        struct dy_constraint c2;
        bool have_c2 = false;
        struct dy_core_expr checked_body;
        if (!dy_check_expr(ctx, def->body, &c2, &have_c2, &checked_body)) {
            checked_body = dy_core_expr_retain(def->body);
        }
        
        --ctx->bindings.num_elems;

        dy_join_constraints(c1, have_c1, DY_CORE_POLARITY_POSITIVE, c2, have_c2, constraint, did_generate_constraint);

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

bool dy_def_remove_mentions_in_subtype(void *data, struct dy_core_ctx *ctx, size_t id, struct dy_core_expr *result)
{
    if (dy_def_is_bound(data, id)) {
        *result = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_END,
            .end_polarity = DY_CORE_POLARITY_POSITIVE
        };
        return true;
    } else {
        return false;
    }
}

bool dy_def_remove_mentions_in_supertype(void *data, struct dy_core_ctx *ctx, size_t id, struct dy_core_expr *result)
{
    if (dy_def_is_bound(data, id)) {
        *result = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_END,
            .end_polarity = DY_CORE_POLARITY_NEGATIVE
        };
        return true;
    } else {
        return false;
    }
}

struct dy_core_expr dy_def_eval(void *data, struct dy_core_ctx *ctx, bool *is_value)
{
    struct dy_def_data *def = data;

    bool arg_is_value = false;
    struct dy_core_expr evaled_arg = dy_eval_expr(ctx, def->arg, &arg_is_value);

    if (!arg_is_value) {
        struct dy_def_data new_data = {
            .id = def->id,
            .arg = evaled_arg,
            .body = dy_core_expr_retain(def->body)
        };

        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_CUSTOM,
            .custom = dy_def_create(new_data)
        };
    }

    struct dy_core_expr new_body;
    if (!substitute(ctx, def->body, def->id, evaled_arg, &new_body)) {
        new_body = dy_core_expr_retain(def->body);
    }

    dy_core_expr_release(evaled_arg);

    struct dy_core_expr ret = dy_eval_expr(ctx, new_body, is_value);

    dy_core_expr_release(new_body);

    return ret;
}

bool dy_def_substitute(void *data, struct dy_core_ctx *ctx, size_t id, struct dy_core_expr sub, struct dy_core_expr *result)
{
    struct dy_def_data *def = data;

    struct dy_core_expr arg;
    bool arg_is_new = substitute(ctx, def->arg, id, sub, &arg);

    if (id == def->id) {
        if (!arg_is_new) {
            return false;
        }
        
        struct dy_def_data new_data = {
            .id = def->id,
            .arg = arg,
            .body = dy_core_expr_retain(def->body)
        };

        *result = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_CUSTOM,
            .custom = dy_def_create(new_data)
        };
        
        return true;
    }

    if (dy_core_expr_is_bound(def->id, sub)) {
        dy_bail("shit");
    }

    struct dy_core_expr body;
    bool body_is_new = substitute(ctx, def->body, id, sub, &body);
    
    if (!arg_is_new && !body_is_new) {
        return false;
    }
    
    if (!arg_is_new) {
        arg = dy_core_expr_retain(def->arg);
    }
    
    if (!body_is_new) {
        body = dy_core_expr_retain(def->body);
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

dy_ternary_t dy_def_is_subtype(void *data, struct dy_core_ctx *ctx, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    return DY_MAYBE;
}

dy_ternary_t dy_def_is_supertype(void *data, struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    return DY_MAYBE;
}

struct dy_core_expr dy_def_eliminate(void *data, struct dy_core_ctx *ctx, struct dy_core_expr expr, bool *is_value)
{
    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_CUSTOM,
        .custom = dy_def_create_no_alloc(data)
    };
}

bool dy_def_is_computation(void *data)
{
    return DY_YES;
}

bool dy_def_is_bound(void *data, size_t id)
{
    struct dy_def_data *def = data;

    if (dy_core_expr_is_bound(id, def->arg)) {
        return true;
    }

    if (id == def->id) {
        return false;
    }

    return dy_core_expr_is_bound(id, def->body);
}

void dy_def_appears_in_polarity(void *data, size_t id, enum dy_core_polarity current_polarity, bool *in_positive, bool *in_negative)
{
    return;
}

void *dy_def_retain(void *data)
{
    static const size_t pre_padding = DY_RC_PRE_PADDING(struct dy_def_data);
    static const size_t post_padding = DY_RC_POST_PADDING(struct dy_def_data);
    return dy_rc_retain(data, pre_padding, post_padding);
}

void dy_def_release(void *data)
{
    static const size_t pre_padding = DY_RC_PRE_PADDING(struct dy_def_data);
    static const size_t post_padding = DY_RC_POST_PADDING(struct dy_def_data);
    dy_rc_release(data, pre_padding, post_padding);
}

void dy_def_to_string(void *data, dy_array_t *string)
{
    dy_array_add(string, &(char){ 'd' });
    dy_array_add(string, &(char){ 'e' });
    dy_array_add(string, &(char){ 'f' });
}
