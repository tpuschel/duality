/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core.h"
#include "substitute.h"
#include "type_of.h"
#include "constraint.h"
#include "is_subtype.h"

/**
 * Evaluates/reduces/normalizes/executes/whatever the given expression.
 * Returns the new expression and if that new expression is a value (fully reduced) or not.
 */

static inline struct dy_core_expr dy_eval_expr(struct dy_core_ctx *ctx, struct dy_core_expr expr, bool *is_value);

static inline struct dy_core_expr dy_eval_equality_map(struct dy_core_ctx *ctx, struct dy_core_equality_map equality_map, bool *is_value);

static inline struct dy_core_expr dy_eval_type_map(struct dy_core_ctx *ctx, struct dy_core_type_map type_map, bool *is_value);

static inline struct dy_core_expr dy_eval_equality_map_elim(struct dy_core_ctx *ctx, struct dy_core_equality_map_elim elim, bool *is_value);

static inline struct dy_core_expr dy_eval_type_map_elim(struct dy_core_ctx *ctx, struct dy_core_type_map_elim elim, bool *is_value);

static inline struct dy_core_expr dy_eval_junction(struct dy_core_ctx *ctx, struct dy_core_junction junction, bool *is_value);

static inline struct dy_core_expr dy_eval_alternative(struct dy_core_ctx *ctx, struct dy_core_alternative alternative, bool *is_value);

static inline struct dy_core_expr dy_eval_recursion(struct dy_core_ctx *ctx, struct dy_core_recursion rec, bool *is_value);

struct dy_core_expr dy_eval_expr(struct dy_core_ctx *ctx, struct dy_core_expr expr, bool *is_value)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_EQUALITY_MAP:
        return dy_eval_equality_map(ctx, expr.equality_map, is_value);
    case DY_CORE_EXPR_TYPE_MAP:
        return dy_eval_type_map(ctx, expr.type_map, is_value);
    case DY_CORE_EXPR_EQUALITY_MAP_ELIM:
        return dy_eval_equality_map_elim(ctx, expr.equality_map_elim, is_value);
    case DY_CORE_EXPR_TYPE_MAP_ELIM:
        return dy_eval_type_map_elim(ctx, expr.type_map_elim, is_value);
    case DY_CORE_EXPR_JUNCTION:
        return dy_eval_junction(ctx, expr.junction, is_value);
    case DY_CORE_EXPR_ALTERNATIVE:
        return dy_eval_alternative(ctx, expr.alternative, is_value);
    case DY_CORE_EXPR_VARIABLE:
        *is_value = true;
        return dy_core_expr_retain(ctx, expr);
    case DY_CORE_EXPR_END:
        *is_value = true;
        return dy_core_expr_retain(ctx, expr);
    case DY_CORE_EXPR_INFERENCE_TYPE_MAP:
        dy_bail("Should not happen");
    case DY_CORE_EXPR_RECURSION:
        return dy_eval_recursion(ctx, expr.recursion, is_value);
    case DY_CORE_EXPR_CUSTOM: {
        const struct dy_core_custom_shared *s = dy_array_pos(ctx->custom_shared, expr.custom.id);
        return s->eval(expr.custom.data, ctx, is_value);
    }
    case DY_CORE_EXPR_SYMBOL:
        *is_value = true;
        return dy_core_expr_retain(ctx, expr);
    }

    dy_bail("Impossible object type.");
}

struct dy_core_expr dy_eval_equality_map(struct dy_core_ctx *ctx, struct dy_core_equality_map equality_map, bool *is_value)
{
    equality_map.e1 = dy_core_expr_new(dy_eval_expr(ctx, *equality_map.e1, is_value));
    dy_core_expr_retain_ptr(equality_map.e2);

    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_EQUALITY_MAP,
        .equality_map = equality_map
    };
}

struct dy_core_expr dy_eval_type_map(struct dy_core_ctx *ctx, struct dy_core_type_map type_map, bool *is_value)
{
    type_map.type = dy_core_expr_new(dy_eval_expr(ctx, *type_map.type, is_value));
    dy_core_expr_retain_ptr(type_map.expr);

    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_TYPE_MAP,
        .type_map = type_map
    };
}

struct dy_core_expr dy_eval_equality_map_elim(struct dy_core_ctx *ctx, struct dy_core_equality_map_elim elim, bool *is_value)
{
    if (elim.check_result == DY_NO) {
        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_EQUALITY_MAP_ELIM,
            .equality_map_elim = elim
        };
    }

    bool left_is_value = false;
    struct dy_core_expr left = dy_eval_expr(ctx, *elim.expr, &left_is_value);

    bool right_is_value = false;
    struct dy_core_expr right = dy_eval_expr(ctx, *elim.map.e1, &right_is_value);

    bool type_is_value = false;
    struct dy_core_expr type = dy_eval_expr(ctx, *elim.map.e2, &type_is_value);

    struct dy_core_expr equality_map = {
        .tag = DY_CORE_EXPR_EQUALITY_MAP,
        .equality_map = {
            .e1 = dy_core_expr_new(dy_core_expr_retain(ctx, right)),
            .e2 = dy_core_expr_new(dy_core_expr_retain(ctx, type)),
            .polarity = elim.map.polarity,
            .is_implicit = elim.map.is_implicit
        }
    };

    if (!left_is_value || !right_is_value || !type_is_value) {
        dy_core_expr_release(ctx, right);
        dy_core_expr_release(ctx, type);

        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_EQUALITY_MAP_ELIM,
            .equality_map_elim = {
                .expr = dy_core_expr_new(left),
                .map = equality_map.equality_map,
                .check_result = elim.check_result,
                .id = elim.id
            }
        };
    }

    dy_core_expr_release(ctx, type);

    if (elim.check_result == DY_MAYBE) {
        struct dy_core_expr type_of_left = dy_type_of(ctx, left);

        size_t x = ctx->constraints.num_elems;
        elim.check_result = dy_is_subtype_no_transformation(ctx, type_of_left, equality_map);

        assert(x == ctx->constraints.num_elems);

        dy_core_expr_release(ctx, type_of_left);
    }

    if (elim.check_result != DY_YES) {
        dy_core_expr_release(ctx, right);

        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_EQUALITY_MAP_ELIM,
            .equality_map_elim = {
                .expr = dy_core_expr_new(left),
                .map = equality_map.equality_map,
                .check_result = elim.check_result,
                .id = elim.id
            }
        };
    }

    if (left.tag == DY_CORE_EXPR_EQUALITY_MAP) {
        if (left.equality_map.is_implicit != elim.map.is_implicit) {
            dy_bail("Bug: Implicitness mismatch!");
        }

        dy_core_expr_release(ctx, equality_map);
        dy_core_expr_release(ctx, right);

        struct dy_core_expr new_expr = dy_eval_expr(ctx, *left.equality_map.e2, is_value);

        dy_core_expr_release(ctx, left);

        return new_expr;
    }

    if (left.tag == DY_CORE_EXPR_TYPE_MAP) {
        if (left.type_map.is_implicit != elim.map.is_implicit) {
            dy_bail("Bug: Implicitness mismatch!");
        }

        dy_core_expr_release(ctx, equality_map);

        struct dy_core_expr e;
        if (!substitute(ctx, *left.type_map.expr, left.type_map.id, right, &e)) {
            e = dy_core_expr_retain(ctx, *left.type_map.expr);
        }

        dy_core_expr_release(ctx, left);
        dy_core_expr_release(ctx, right);

        struct dy_core_expr new_expr = dy_eval_expr(ctx, e, is_value);

        dy_core_expr_release(ctx, e);

        return new_expr;
    }

    dy_core_expr_release(ctx, right);

    if (left.tag == DY_CORE_EXPR_JUNCTION) {
        struct dy_core_expr type_of_junction_e1 = dy_type_of(ctx, *left.junction.e1);

        size_t x = ctx->constraints.num_elems;
        dy_ternary_t res1 = dy_is_subtype_no_transformation(ctx, type_of_junction_e1, equality_map);
        assert(x == ctx->constraints.num_elems);

        dy_core_expr_release(ctx, type_of_junction_e1);

        if (res1 == DY_YES) {
            dy_core_expr_release_ptr(ctx, left.junction.e2);

            struct dy_core_equality_map_elim new_elim = {
                .expr = left.junction.e1,
                .map = equality_map.equality_map,
                .check_result = DY_YES
            };

            return dy_eval_equality_map_elim(ctx, new_elim, is_value);
        }

        dy_core_expr_release_ptr(ctx, left.junction.e1);

        struct dy_core_equality_map_elim new_elim = {
            .expr = left.junction.e2,
            .map = equality_map.equality_map,
            .check_result = DY_YES
        };

        return dy_eval_equality_map_elim(ctx, new_elim, is_value);
    }

    if (left.tag == DY_CORE_EXPR_CUSTOM) {
        const struct dy_core_custom_shared *s = dy_array_pos(ctx->custom_shared, left.custom.id);
        if (s->can_be_eliminated) {
            return s->eliminate(left.custom.data, ctx, right, is_value);
        }
    }

    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_EQUALITY_MAP_ELIM,
        .equality_map_elim = {
            .expr = dy_core_expr_new(left),
            .map = equality_map.equality_map,
            .check_result = elim.check_result,
            .id = elim.id
        }
    };
}

struct dy_core_expr dy_eval_type_map_elim(struct dy_core_ctx *ctx, struct dy_core_type_map_elim elim, bool *is_value)
{
    dy_bail("Not yet implemented.");
}

struct dy_core_expr dy_eval_junction(struct dy_core_ctx *ctx, struct dy_core_junction junction, bool *is_value)
{
    bool e1_is_value = false;
    junction.e1 = dy_core_expr_new(dy_eval_expr(ctx, *junction.e1, &e1_is_value));

    bool e2_is_value = false;
    junction.e2 = dy_core_expr_new(dy_eval_expr(ctx, *junction.e2, &e2_is_value));

    *is_value = e1_is_value && e2_is_value;

    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_JUNCTION,
        .junction = junction
    };
}

struct dy_core_expr dy_eval_alternative(struct dy_core_ctx *ctx, struct dy_core_alternative alternative, bool *is_value)
{
    bool first_is_value = false;
    struct dy_core_expr first = dy_eval_expr(ctx, *alternative.first, &first_is_value);
    if (first_is_value) {
        *is_value = true;
        return first;
    }

    bool second_is_value = false;
    struct dy_core_expr second = dy_eval_expr(ctx, *alternative.second, &second_is_value);
    if (second_is_value) {
        dy_core_expr_release(ctx, first);
        *is_value = true;
        return second;
    }

    alternative.first = dy_core_expr_new(first);
    alternative.second = dy_core_expr_new(second);

    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_ALTERNATIVE,
        .alternative = alternative
    };
}

struct dy_core_expr dy_eval_recursion(struct dy_core_ctx *ctx, struct dy_core_recursion recursion, bool *is_value)
{
    struct dy_core_expr evaled_body = dy_eval_expr(ctx, *recursion.expr, is_value);

    recursion.expr = dy_core_expr_new(evaled_body);

    struct dy_core_expr rec_expr = {
        .tag = DY_CORE_EXPR_RECURSION,
        .recursion = recursion
    };

    struct dy_core_expr substituted_body;
    if (!substitute(ctx, evaled_body, recursion.id, rec_expr, &substituted_body)) {
        substituted_body = dy_core_expr_retain(ctx, evaled_body);
    }

    dy_core_expr_release_ptr(ctx, recursion.expr);

    return substituted_body;
}
