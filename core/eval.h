/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_EVAL_H
#define DY_EVAL_H

#include "core.h"
#include "substitute.h"
#include "type_of.h"
#include "constraint.h"
#include "is_subtype.h"

static inline struct dy_core_expr dy_eval_expr(struct dy_core_ctx *ctx, struct dy_core_expr expr, bool *is_value);

static inline struct dy_core_expr dy_eval_expr_map(struct dy_core_ctx *ctx, struct dy_core_expr_map expr_map, bool *is_value);

static inline struct dy_core_expr dy_eval_type_map(struct dy_core_ctx *ctx, struct dy_core_type_map type_map, bool *is_value);

static inline struct dy_core_expr dy_eval_expr_map_elim(struct dy_core_ctx *ctx, struct dy_core_expr_map_elim elim, bool *is_value);

static inline struct dy_core_expr dy_eval_type_map_elim(struct dy_core_ctx *ctx, struct dy_core_type_map_elim elim, bool *is_value);

static inline struct dy_core_expr dy_eval_both(struct dy_core_ctx *ctx, struct dy_core_both both, bool *is_value);

static inline struct dy_core_expr dy_eval_one_of(struct dy_core_ctx *ctx, struct dy_core_one_of one_of, bool *is_value);

static inline struct dy_core_expr dy_eval_recursion(struct dy_core_ctx *ctx, struct dy_core_recursion rec, bool *is_value);

struct dy_core_expr dy_eval_expr(struct dy_core_ctx *ctx, struct dy_core_expr expr, bool *is_value)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_EXPR_MAP:
        return dy_eval_expr_map(ctx, expr.expr_map, is_value);
    case DY_CORE_EXPR_TYPE_MAP:
        return dy_eval_type_map(ctx, expr.type_map, is_value);
    case DY_CORE_EXPR_EXPR_MAP_ELIM:
        return dy_eval_expr_map_elim(ctx, expr.expr_map_elim, is_value);
    case DY_CORE_EXPR_TYPE_MAP_ELIM:
        return dy_eval_type_map_elim(ctx, expr.type_map_elim, is_value);
    case DY_CORE_EXPR_BOTH:
        return dy_eval_both(ctx, expr.both, is_value);
    case DY_CORE_EXPR_ONE_OF:
        return dy_eval_one_of(ctx, expr.one_of, is_value);
    case DY_CORE_EXPR_VARIABLE:
        *is_value = true;
        return dy_core_expr_retain(expr);
    case DY_CORE_EXPR_INFERENCE_VARIABLE:
        *is_value = true;
        return dy_core_expr_retain(expr);
    case DY_CORE_EXPR_END:
        *is_value = true;
        return dy_core_expr_retain(expr);
    case DY_CORE_EXPR_INFERENCE_TYPE_MAP:
        dy_bail("should not happen");
    case DY_CORE_EXPR_RECURSION:
        return dy_eval_recursion(ctx, expr.recursion, is_value);
    case DY_CORE_EXPR_CUSTOM:
        return expr.custom.eval(expr.custom.data, ctx, is_value);
    case DY_CORE_EXPR_SYMBOL:
        *is_value = false;
        return dy_core_expr_retain(expr);
    }

    DY_IMPOSSIBLE_ENUM();
}

struct dy_core_expr dy_eval_expr_map(struct dy_core_ctx *ctx, struct dy_core_expr_map expr_map, bool *is_value)
{
    expr_map.e1 = dy_core_expr_new(dy_eval_expr(ctx, *expr_map.e1, is_value));
    dy_core_expr_retain_ptr(expr_map.e2);

    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_EXPR_MAP,
        .expr_map = expr_map
    };
}

struct dy_core_expr dy_eval_type_map(struct dy_core_ctx *ctx, struct dy_core_type_map type_map, bool *is_value)
{
    type_map.binding.type = dy_core_expr_new(dy_eval_expr(ctx, *type_map.binding.type, is_value));
    dy_core_expr_retain_ptr(type_map.expr);

    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_TYPE_MAP,
        .type_map = type_map
    };
}

struct dy_core_expr dy_eval_expr_map_elim(struct dy_core_ctx *ctx, struct dy_core_expr_map_elim elim, bool *is_value)
{
    bool left_is_value = false;
    struct dy_core_expr left = dy_eval_expr(ctx, *elim.expr, &left_is_value);

    bool right_is_value = false;
    struct dy_core_expr right = dy_eval_expr(ctx, *elim.map.e1, &right_is_value);

    bool type_is_value = false;
    struct dy_core_expr type = dy_eval_expr(ctx, *elim.map.e2, &type_is_value);

    struct dy_core_expr expr_map = {
        .tag = DY_CORE_EXPR_EXPR_MAP,
        .expr_map = {
            .e1 = dy_core_expr_new(dy_core_expr_retain(right)),
            .e2 = dy_core_expr_new(dy_core_expr_retain(type)),
            .polarity = elim.map.polarity,
            .is_implicit = elim.map.is_implicit,
        }
    };

    if (!left_is_value || !right_is_value || !type_is_value) {
        dy_core_expr_release(right);
        dy_core_expr_release(type);

        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_EXPR_MAP_ELIM,
            .expr_map_elim = {
                .expr = dy_core_expr_new(left),
                .map = expr_map.expr_map,
                .check_result = elim.check_result,
            }
        };
    }

    dy_core_expr_release(type);

    if (elim.check_result == DY_MAYBE) {
        struct dy_core_expr type_of_left = dy_type_of(ctx, left);

        struct dy_constraint constraint;
        bool have_constraint = false;
        elim.check_result = dy_is_subtype_no_transformation(ctx, type_of_left, expr_map, &constraint, &have_constraint);

        dy_assert(!have_constraint);

        dy_core_expr_release(type_of_left);
    }

    if (elim.check_result != DY_YES) {
        dy_core_expr_release(right);

        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_EXPR_MAP_ELIM,
            .expr_map_elim = {
                .expr = dy_core_expr_new(left),
                .map = expr_map.expr_map,
                .check_result = elim.check_result,
            }
        };
    }

    if (left.tag == DY_CORE_EXPR_EXPR_MAP) {
        dy_core_expr_release(expr_map);
        dy_core_expr_release(right);

        struct dy_core_expr new_expr = dy_eval_expr(ctx, *left.expr_map.e2, is_value);

        dy_core_expr_release(left);

        return new_expr;
    }

    if (left.tag == DY_CORE_EXPR_TYPE_MAP) {
        dy_core_expr_release(expr_map);

        struct dy_core_expr e = substitute(*left.type_map.expr, left.type_map.binding.id, right);

        dy_core_expr_release(left);
        dy_core_expr_release(right);

        struct dy_core_expr new_expr = dy_eval_expr(ctx, e, is_value);

        dy_core_expr_release(e);

        return new_expr;
    }

    dy_core_expr_release(right);

    if (left.tag == DY_CORE_EXPR_BOTH) {
        struct dy_core_expr_map_elim new_elim = {
            .expr = left.both.e1,
            .map = expr_map.expr_map,
            .check_result = DY_MAYBE
        };

        struct dy_core_expr new_expr = dy_eval_expr_map_elim(ctx, new_elim, is_value);

        if (*is_value) {
            dy_core_expr_release(left);
            dy_core_expr_release(expr_map);
            return new_expr;
        }

        new_elim.expr = left.both.e2;

        new_expr = dy_eval_expr_map_elim(ctx, new_elim, is_value);

        dy_core_expr_release(left);
        dy_core_expr_release(expr_map);

        return new_expr;
    }

    if (left.tag == DY_CORE_EXPR_CUSTOM && left.custom.can_be_eliminated) {
        return left.custom.eliminate(left.custom.data, ctx, right, is_value);
    }

    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_EXPR_MAP_ELIM,
        .expr_map_elim = {
            .expr = dy_core_expr_new(left),
            .map = expr_map.expr_map,
            .check_result = elim.check_result,
        }
    };
}

struct dy_core_expr dy_eval_type_map_elim(struct dy_core_ctx *ctx, struct dy_core_type_map_elim elim, bool *is_value)
{
    dy_bail("Not yet implemented.");
}

struct dy_core_expr dy_eval_both(struct dy_core_ctx *ctx, struct dy_core_both both, bool *is_value)
{
    bool e1_is_value = false;
    both.e1 = dy_core_expr_new(dy_eval_expr(ctx, *both.e1, &e1_is_value));

    bool e2_is_value = false;
    both.e2 = dy_core_expr_new(dy_eval_expr(ctx, *both.e2, &e2_is_value));

    *is_value = e1_is_value && e2_is_value;

    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_BOTH,
        .both = both
    };
}

struct dy_core_expr dy_eval_one_of(struct dy_core_ctx *ctx, struct dy_core_one_of one_of, bool *is_value)
{
    bool first_is_value = false;
    struct dy_core_expr first = dy_eval_expr(ctx, *one_of.first, &first_is_value);
    if (first_is_value) {
        *is_value = true;
        return first;
    }

    bool second_is_value = false;
    struct dy_core_expr second = dy_eval_expr(ctx, *one_of.second, &second_is_value);
    if (second_is_value) {
        dy_core_expr_release(first);
        *is_value = true;
        return second;
    }

    one_of.first = dy_core_expr_new(first);
    one_of.second = dy_core_expr_new(second);

    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_ONE_OF,
        .one_of = one_of
    };
}

struct dy_core_expr dy_eval_recursion(struct dy_core_ctx *ctx, struct dy_core_recursion recursion, bool *is_value)
{
    struct dy_core_expr rec_expr = {
        .tag = DY_CORE_EXPR_RECURSION,
        .recursion = recursion
    };

    struct dy_core_expr e = substitute(*recursion.map.expr, recursion.map.binding.id, rec_expr);

    struct dy_core_expr new_e = dy_eval_expr(ctx, e, is_value);

    dy_core_expr_release(e);

    return new_e;
}

#endif // DY_EVAL_H
