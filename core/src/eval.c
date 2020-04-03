/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <duality/core/eval.h>

#include "substitute.h"

#include <duality/core/is_subtype.h>
#include <duality/core/type_of.h>
#include <duality/core/constraint.h>
#include <duality/core/check.h>

#include <duality/support/assert.h>

#include <stdio.h>

dy_ternary_t dy_eval_expr(struct dy_core_ctx ctx, struct dy_core_expr expr, struct dy_core_expr *new_expr)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_EXPR_MAP:
        return dy_eval_expr_map(ctx, expr.expr_map, new_expr);
    case DY_CORE_EXPR_TYPE_MAP:
        return dy_eval_type_map(ctx, expr.type_map, new_expr);
    case DY_CORE_EXPR_EXPR_MAP_ELIM:
        return dy_eval_expr_map_elim(ctx, expr.expr_map_elim, new_expr);
    case DY_CORE_EXPR_TYPE_MAP_ELIM:
        return dy_eval_type_map_elim(ctx, expr.type_map_elim, new_expr);
    case DY_CORE_EXPR_BOTH:
        return dy_eval_both(ctx, expr.both, new_expr);
    case DY_CORE_EXPR_ONE_OF:
        return dy_eval_one_of(ctx, expr.one_of, new_expr);
    case DY_CORE_EXPR_UNKNOWN:
        *new_expr = dy_core_expr_retain(ctx.expr_pool, expr);
        return DY_YES;
    case DY_CORE_EXPR_END:
        *new_expr = dy_core_expr_retain(ctx.expr_pool, expr);
        return DY_YES;
    case DY_CORE_EXPR_STRING:
        *new_expr = dy_core_expr_retain(ctx.expr_pool, expr);
        return DY_YES;
    case DY_CORE_EXPR_TYPE_OF_STRINGS:
        *new_expr = dy_core_expr_retain(ctx.expr_pool, expr);
        return DY_YES;
    case DY_CORE_EXPR_PRINT:
        *new_expr = dy_core_expr_retain(ctx.expr_pool, expr);
        return DY_YES;
    }

    DY_IMPOSSIBLE_ENUM();
}

dy_ternary_t dy_eval_expr_map(struct dy_core_ctx ctx, struct dy_core_expr_map expr_map, struct dy_core_expr *new_expr)
{
    struct dy_core_expr new_e1;
    dy_ternary_t result = dy_eval_expr(ctx, *expr_map.e1, &new_e1);
    if (result == DY_NO) {
        return DY_NO;
    }

    *new_expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_EXPR_MAP,
        .expr_map = {
            .e1 = dy_core_expr_new(ctx.expr_pool, new_e1),
            .e2 = dy_core_expr_retain_ptr(ctx.expr_pool, expr_map.e2),
            .polarity = expr_map.polarity,
            .is_implicit = expr_map.is_implicit,
        }
    };

    return result;
}

dy_ternary_t dy_eval_type_map(struct dy_core_ctx ctx, struct dy_core_type_map type_map, struct dy_core_expr *new_expr)
{
    struct dy_core_expr new_type;
    dy_ternary_t result = dy_eval_expr(ctx, *type_map.arg_type, &new_type);
    if (result == DY_NO) {
        return DY_NO;
    }

    *new_expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_TYPE_MAP,
        .type_map = {
            .arg_id = type_map.arg_id,
            .arg_type = dy_core_expr_new(ctx.expr_pool, new_type),
            .expr = dy_core_expr_retain_ptr(ctx.expr_pool, type_map.expr),
            .polarity = type_map.polarity,
            .is_implicit = type_map.is_implicit,
        }
    };

    return result;
}

dy_ternary_t dy_eval_expr_map_elim(struct dy_core_ctx ctx, struct dy_core_expr_map_elim elim, struct dy_core_expr *new_expr)
{
    struct dy_core_expr left;
    dy_ternary_t left_result = dy_eval_expr(ctx, *elim.expr, &left);
    if (left_result == DY_NO) {
        return DY_NO;
    }

    struct dy_core_expr right;
    dy_ternary_t right_result = dy_eval_expr(ctx, *elim.expr_map.e1, &right);
    if (right_result == DY_NO) {
        dy_core_expr_release(ctx.expr_pool, left);
        return DY_NO;
    }

    struct dy_core_expr type;
    dy_ternary_t type_result = dy_eval_expr(ctx, *elim.expr_map.e2, &type);
    if (type_result == DY_NO) {
        dy_core_expr_release(ctx.expr_pool, left);
        dy_core_expr_release(ctx.expr_pool, right);
        return DY_NO;
    }

    if (left_result == DY_MAYBE || right_result == DY_MAYBE || type_result == DY_MAYBE) {
        *new_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_EXPR_MAP_ELIM,
            .expr_map_elim = {
                .expr = dy_core_expr_new(ctx.expr_pool, left),
                .expr_map = {
                    .e1 = dy_core_expr_new(ctx.expr_pool, right),
                    .e2 = dy_core_expr_new(ctx.expr_pool, type),
                    .polarity = elim.expr_map.polarity,
                    .is_implicit = elim.expr_map.is_implicit,
                },
            }
        };

        return DY_MAYBE;
    }

    struct dy_core_expr expr_map = {
        .tag = DY_CORE_EXPR_EXPR_MAP,
        .expr_map = {
            .e1 = dy_core_expr_new(ctx.expr_pool, dy_core_expr_retain(ctx.expr_pool, right)),
            .e2 = dy_core_expr_new(ctx.expr_pool, dy_core_expr_retain(ctx.expr_pool, type)),
            .polarity = elim.expr_map.polarity,
            .is_implicit = elim.expr_map.is_implicit,
        }
    };

    dy_core_expr_release(ctx.expr_pool, type);

    struct dy_core_expr type_of_left = dy_type_of(ctx, left);

    struct dy_constraint constraint;
    bool have_constraint = false;
    dy_ternary_t result = dy_is_subtype_no_transformation(ctx, type_of_left, expr_map, &constraint, &have_constraint);

    dy_core_expr_release(ctx.expr_pool, type_of_left);

    if (result == DY_NO) {
        dy_core_expr_release(ctx.expr_pool, left);
        dy_core_expr_release(ctx.expr_pool, right);
        dy_core_expr_release(ctx.expr_pool, expr_map);
        return DY_NO;
    }

    dy_assert(!have_constraint);

    if (result == DY_MAYBE) {
        dy_core_expr_release(ctx.expr_pool, right);

        *new_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_EXPR_MAP_ELIM,
            .expr_map_elim = {
                .expr = dy_core_expr_new(ctx.expr_pool, left),
                .expr_map = expr_map.expr_map,
            }
        };

        return DY_MAYBE;
    }

    if (left.tag == DY_CORE_EXPR_PRINT) {
        for (size_t i = 0; i < right.string.size; ++i) {
            fprintf(stderr, "%c", right.string.ptr[i]);
        }
        fprintf(stderr, "\n");

        dy_core_expr_release(ctx.expr_pool, left);
        dy_core_expr_release(ctx.expr_pool, expr_map);

        *new_expr = right;

        return DY_YES;
    }

    if (left.tag == DY_CORE_EXPR_EXPR_MAP) {
        dy_core_expr_release(ctx.expr_pool, expr_map);
        dy_core_expr_release(ctx.expr_pool, right);

        result = dy_eval_expr(ctx, *left.expr_map.e2, new_expr);

        dy_core_expr_release(ctx.expr_pool, left);

        return result;
    }

    if (left.tag == DY_CORE_EXPR_TYPE_MAP) {
        dy_core_expr_release(ctx.expr_pool, expr_map);

        struct dy_core_expr e = substitute(ctx, left.type_map.arg_id, right, *left.type_map.expr);

        dy_core_expr_release(ctx.expr_pool, left);
        dy_core_expr_release(ctx.expr_pool, right);

        struct dy_core_expr new_e;
        result = dy_check_expr(ctx, e, &new_e, &constraint, &have_constraint);

        dy_core_expr_release(ctx.expr_pool, e);

        if (!result) {
            return DY_NO;
        }

        dy_assert(!have_constraint);

        result = dy_eval_expr(ctx, new_e, new_expr);

        dy_core_expr_release(ctx.expr_pool, new_e);

        return result;
    }

    dy_core_expr_release(ctx.expr_pool, right);

    if (left.tag == DY_CORE_EXPR_BOTH) {
        struct dy_core_expr_map_elim new_elim = {
            .expr = left.both.e1,
            .expr_map = expr_map.expr_map,
        };

        result = dy_eval_expr_map_elim(ctx, new_elim, new_expr);

        if (result != DY_NO) {
            dy_core_expr_release(ctx.expr_pool, left);
            dy_core_expr_release(ctx.expr_pool, expr_map);
            return result;
        }

        new_elim.expr = left.both.e2;

        result = dy_eval_expr_map_elim(ctx, new_elim, new_expr);

        dy_core_expr_release(ctx.expr_pool, left);
        dy_core_expr_release(ctx.expr_pool, expr_map);

        return result;
    }

    *new_expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_EXPR_MAP_ELIM,
        .expr_map_elim = {
            .expr = dy_core_expr_new(ctx.expr_pool, left),
            .expr_map = expr_map.expr_map,
        }
    };

    return DY_MAYBE;
}

dy_ternary_t dy_eval_type_map_elim(struct dy_core_ctx ctx, struct dy_core_type_map_elim elim, struct dy_core_expr *new_expr)
{
    dy_bail("Not yet implemented.");
}

dy_ternary_t dy_eval_both(struct dy_core_ctx ctx, struct dy_core_both both, struct dy_core_expr *new_expr)
{
    struct dy_core_expr e1;
    dy_ternary_t first_result = dy_eval_expr(ctx, *both.e1, &e1);
    if (first_result == DY_NO) {
        return DY_NO;
    }

    struct dy_core_expr e2;
    dy_ternary_t second_result = dy_eval_expr(ctx, *both.e2, &e2);
    if (second_result == DY_NO) {
        dy_core_expr_release(ctx.expr_pool, e1);
        return DY_NO;
    }

    *new_expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_BOTH,
        .both = {
            .e1 = dy_core_expr_new(ctx.expr_pool, e1),
            .e2 = dy_core_expr_new(ctx.expr_pool, e2),
            .polarity = both.polarity,
        }
    };

    if (first_result == DY_MAYBE || second_result == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

dy_ternary_t dy_eval_one_of(struct dy_core_ctx ctx, struct dy_core_one_of one_of, struct dy_core_expr *new_expr)
{
    struct dy_core_expr first;
    dy_ternary_t first_result = dy_eval_expr(ctx, *one_of.first, &first);
    if (first_result == DY_YES) {
        *new_expr = first;
        return DY_YES;
    }

    if (first_result == DY_MAYBE) {
        *new_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_ONE_OF,
            .one_of = {
                .first = dy_core_expr_new(ctx.expr_pool, first),
                .second = dy_core_expr_retain_ptr(ctx.expr_pool, one_of.second),
            }
        };

        return DY_MAYBE;
    }

    return dy_eval_expr(ctx, *one_of.second, new_expr);
}
