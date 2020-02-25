/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <duality/core/eval.h>

#include "substitute.h"

#include <duality/core/is_subtype.h>
#include <duality/core/type_of.h>
#include <duality/support/assert.h>
#include <duality/core/constraint.h>

#include <stdio.h>

static struct dy_core_expr *alloc_expr(struct dy_check_ctx ctx, struct dy_core_expr expr);

dy_ternary_t dy_eval_expr(struct dy_check_ctx ctx, struct dy_core_expr expr, struct dy_core_expr *new_expr)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_VALUE_MAP:
        return dy_eval_value_map(ctx, expr.value_map, new_expr);
    case DY_CORE_EXPR_TYPE_MAP:
        return dy_eval_type_map(ctx, expr.type_map, new_expr);
    case DY_CORE_EXPR_VALUE_MAP_ELIM:
        return dy_eval_value_map_elim(ctx, expr.value_map_elim, new_expr);
    case DY_CORE_EXPR_TYPE_MAP_ELIM:
        return dy_eval_type_map_elim(ctx, expr.type_map_elim, new_expr);
    case DY_CORE_EXPR_BOTH:
        return dy_eval_both(ctx, expr.both, new_expr);
    case DY_CORE_EXPR_ONE_OF:
        return dy_eval_one_of(ctx, expr.one_of, new_expr);
    case DY_CORE_EXPR_UNKNOWN:
        *new_expr = expr;
        return DY_YES;
    case DY_CORE_EXPR_END:
        *new_expr = expr;
        return DY_YES;
    case DY_CORE_EXPR_STRING:
        *new_expr = expr;
        return DY_YES;
    case DY_CORE_EXPR_TYPE_OF_STRINGS:
        *new_expr = expr;
        return DY_YES;
    case DY_CORE_EXPR_PRINT:
        *new_expr = expr;
        return DY_YES;
    }

    DY_IMPOSSIBLE_ENUM();
}

dy_ternary_t dy_eval_value_map(struct dy_check_ctx ctx, struct dy_core_value_map value_map, struct dy_core_expr *new_expr)
{
    struct dy_core_expr new_e1;
    dy_ternary_t result = dy_eval_expr(ctx, *value_map.e1, &new_e1);
    if (result == DY_NO) {
        return DY_NO;
    }

    *new_expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_VALUE_MAP,
        .value_map = {
            .e1 = alloc_expr(ctx, new_e1),
            .e2 = value_map.e2,
            .polarity = value_map.polarity,
            .is_implicit = value_map.is_implicit,
        }
    };

    return result;
}

dy_ternary_t dy_eval_type_map(struct dy_check_ctx ctx, struct dy_core_type_map type_map, struct dy_core_expr *new_expr)
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
            .arg_type = alloc_expr(ctx, new_type),
            .expr = type_map.expr,
            .polarity = type_map.polarity,
            .is_implicit = type_map.is_implicit,
        }
    };

    return result;
}

dy_ternary_t dy_eval_value_map_elim(struct dy_check_ctx ctx, struct dy_core_value_map_elim elim, struct dy_core_expr *new_expr)
{
    struct dy_core_expr left;
    dy_ternary_t left_result = dy_eval_expr(ctx, *elim.expr, &left);

    struct dy_core_expr right;
    dy_ternary_t right_result = dy_eval_expr(ctx, *elim.value_map.e1, &right);

    struct dy_core_expr type;
    dy_ternary_t type_result = dy_eval_expr(ctx, *elim.value_map.e2, &type);

    if (left_result == DY_NO || right_result == DY_NO || type_result == DY_NO) {
        return DY_NO;
    }

    if (left_result == DY_MAYBE || right_result == DY_MAYBE || type_result == DY_MAYBE) {
        *new_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_VALUE_MAP_ELIM,
            .value_map_elim = {
                .id = elim.id,
                .expr = alloc_expr(ctx, left),
                .value_map = {
                    .e1 = alloc_expr(ctx, right),
                    .e2 = alloc_expr(ctx, type),
                    .polarity = elim.value_map.polarity,
                    .is_implicit = elim.value_map.is_implicit,
                },
            }
        };

        return DY_MAYBE;
    }

    struct dy_core_expr value_map = {
        .tag = DY_CORE_EXPR_VALUE_MAP,
        .value_map = {
            .e1 = alloc_expr(ctx, right),
            .e2 = alloc_expr(ctx, type),
            .polarity = elim.value_map.polarity,
            .is_implicit = elim.value_map.is_implicit,
        }
    };

    bool check_elim = true;
    for (size_t i = 0, size = dy_array_size(ctx.successful_elims); i < size; ++i) {
        size_t id;
        dy_array_get(ctx.successful_elims, i, &id);

        if (id == elim.id) {
            check_elim = false;
            break;
        }
    }

    if (check_elim) {
        struct dy_constraint constraint;
        bool have_constraint = false;
        dy_ternary_t result = dy_is_subtype_no_transformation(ctx, dy_type_of(ctx, left), value_map, &constraint, &have_constraint);
        if (result == DY_NO) {
            return DY_NO;
        }

        if (have_constraint) {
            fprintf(stderr, "Constraint on eval??\n");
            return DY_NO;
        }

        if (result == DY_MAYBE) {
            *new_expr = (struct dy_core_expr){
                .tag = DY_CORE_EXPR_VALUE_MAP_ELIM,
                .value_map_elim = {
                    .id = elim.id,
                    .expr = alloc_expr(ctx, left),
                    .value_map = value_map.value_map,
                }
            };

            return DY_MAYBE;
        }
    }

    if (left.tag == DY_CORE_EXPR_PRINT) {
        fprintf(stderr, "<duality print> ");
        for (size_t i = 0; i < right.string.size; ++i) {
            fprintf(stderr, "%c", right.string.ptr[i]);
        }
        fprintf(stderr, "\n");

        *new_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_STRING,
            .string = DY_STR_LIT("<print successful>")
        };

        return DY_YES;
    }

    if (left.tag == DY_CORE_EXPR_VALUE_MAP) {
        return dy_eval_expr(ctx, *left.value_map.e2, new_expr);
    }

    if (left.tag == DY_CORE_EXPR_TYPE_MAP) {
        struct dy_core_expr e = substitute(ctx, left.type_map.arg_id, right, *left.type_map.expr);
        struct dy_constraint constraint;
        bool have_constraint = false;
        if (!dy_check_expr(ctx, e, &e, &constraint, &have_constraint)) {
            return DY_NO;
        }
        if (have_constraint) {
            fprintf(stderr, "Constraint on eval??\n");
            return DY_NO;
        }

        return dy_eval_expr(ctx, e, new_expr);
    }

    if (left.tag == DY_CORE_EXPR_BOTH) {
        struct dy_constraint constraint;
        bool have_constraint = false;
        dy_ternary_t result = dy_is_subtype_no_transformation(ctx, dy_type_of(ctx, *left.both.e1), value_map, &constraint, &have_constraint);
        if (have_constraint) {
            fprintf(stderr, "Constraint on eval??\n");
            return DY_NO;
        }
        if (result == DY_YES) {
            struct dy_core_value_map_elim new_elim = {
                .id = (*ctx.running_id)++,
                .expr = left.both.e1,
                .value_map = value_map.value_map,
            };

            return dy_eval_value_map_elim(ctx, new_elim, new_expr);
        }

        struct dy_core_value_map_elim new_elim = {
            .id = (*ctx.running_id)++,
            .expr = left.both.e2,
            .value_map = value_map.value_map,
        };

        return dy_eval_value_map_elim(ctx, new_elim, new_expr);
    }

    *new_expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_VALUE_MAP_ELIM,
        .value_map_elim = elim
    };

    return DY_MAYBE;
}

dy_ternary_t dy_eval_type_map_elim(struct dy_check_ctx ctx, struct dy_core_type_map_elim elim, struct dy_core_expr *new_expr)
{
    return DY_NO;
}

dy_ternary_t dy_eval_both(struct dy_check_ctx ctx, struct dy_core_both both, struct dy_core_expr *new_expr)
{
    struct dy_core_expr e1;
    dy_ternary_t first_result = dy_eval_expr(ctx, *both.e1, &e1);

    struct dy_core_expr e2;
    dy_ternary_t second_result = dy_eval_expr(ctx, *both.e2, &e2);

    if (first_result == DY_NO || second_result == DY_NO) {
        return DY_NO;
    }

    *new_expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_BOTH,
        .both = {
            .e1 = alloc_expr(ctx, e1),
            .e2 = alloc_expr(ctx, e2),
            .polarity = both.polarity,
        }
    };

    if (first_result == DY_MAYBE || second_result == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

dy_ternary_t dy_eval_one_of(struct dy_check_ctx ctx, struct dy_core_one_of one_of, struct dy_core_expr *new_expr)
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
                .first = alloc_expr(ctx, first),
                .second = one_of.second,
            }
        };

        return DY_MAYBE;
    }

    return dy_eval_expr(ctx, *one_of.second, new_expr);
}

struct dy_core_expr *alloc_expr(struct dy_check_ctx ctx, struct dy_core_expr expr)
{
    return dy_alloc(&expr, sizeof expr, ctx.allocator);
}
