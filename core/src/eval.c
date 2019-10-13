/*
 * Copyright 2017-2019 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <duality/core/eval.h>

#include "substitute.h"

#include <duality/support/assert.h>
#include <duality/core/is_subtype.h>
#include <duality/core/type_of.h>

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
    case DY_CORE_EXPR_ANY_OF:
        return dy_eval_any_of(ctx, expr.any_of, new_expr);
    case DY_CORE_EXPR_UNKNOWN:
        *new_expr = expr;
        return DY_YES;
    case DY_CORE_EXPR_TYPE_OF_TYPES:
        *new_expr = expr;
        return DY_YES;
    case DY_CORE_EXPR_STRING:
        *new_expr = expr;
        return DY_YES;
    case DY_CORE_EXPR_TYPE_OF_STRINGS:
        *new_expr = expr;
        return DY_YES;
    }

    DY_IMPOSSIBLE_ENUM();
}

dy_ternary_t dy_eval_value_map(struct dy_check_ctx ctx, struct dy_core_value_map value_map, struct dy_core_expr *new_expr)
{
    struct dy_core_expr new_e1;
    dy_ternary_t is_valid = dy_eval_expr(ctx, *value_map.e1, &new_e1);
    if (is_valid == DY_NO) {
        return DY_NO;
    }

    *new_expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_VALUE_MAP,
        .value_map = {
            .e1 = alloc_expr(ctx, new_e1),
            .e2 = value_map.e2,
            .polarity = value_map.polarity,
        }
    };

    return is_valid;
}

dy_ternary_t dy_eval_type_map(struct dy_check_ctx ctx, struct dy_core_type_map type_map, struct dy_core_expr *new_expr)
{
    struct dy_core_expr new_type;
    dy_ternary_t is_valid = dy_eval_expr(ctx, *type_map.arg.type, &new_type);
    if (is_valid == DY_NO) {
        return DY_NO;
    }

    *new_expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_TYPE_MAP,
        .type_map = {
            .arg = {
                .id = type_map.arg.id,
                .type = alloc_expr(ctx, new_type),
            },
            .expr = type_map.expr,
            .polarity = type_map.polarity,
        }
    };

    return is_valid;
}

dy_ternary_t dy_eval_value_map_elim(struct dy_check_ctx ctx, struct dy_core_value_map_elim elim, struct dy_core_expr *new_expr)
{
    struct dy_core_expr left;
    dy_ternary_t left_is_valid = dy_eval_expr(ctx, *elim.expr, &left);
    if (left_is_valid == DY_NO) {
        return DY_NO;
    }

    struct dy_core_expr right;
    dy_ternary_t right_is_valid = dy_eval_expr(ctx, *elim.value_map.e1, &right);
    if (right_is_valid == DY_NO) {
        return DY_NO;
    }

    if (left.tag == DY_CORE_EXPR_VALUE_MAP) {
        return dy_eval_expr(ctx, *left.value_map.e2, new_expr);
    }

    if (left.tag == DY_CORE_EXPR_TYPE_MAP) {
        struct dy_core_expr result = substitute(ctx, left.type_map.arg.id, right, *left.type_map.expr);

        struct dy_core_expr checked_result;
        dy_ternary_t res = dy_check_expr(ctx, result, &checked_result);
        if (res == DY_NO) {
            return DY_NO;
        }

        return dy_eval_expr(ctx, checked_result, new_expr);
    }

    if (left.tag == DY_CORE_EXPR_BOTH) {
        struct dy_core_expr value_map = {
            .tag = DY_CORE_EXPR_VALUE_MAP,
            .value_map = elim.value_map
        };

        dy_ternary_t will_work = dy_is_subtype(ctx, dy_type_of(ctx, *left.both.first), value_map);
        if (will_work == DY_YES) {
            elim.expr = left.both.first;
            return dy_eval_value_map_elim(ctx, elim, new_expr);
        }
        if (will_work == DY_MAYBE) {
            *new_expr = (struct dy_core_expr){
                .tag = DY_CORE_EXPR_VALUE_MAP_ELIM,
                .value_map_elim = elim
            };

            return DY_MAYBE;
        }

        elim.expr = left.both.second;
        return dy_eval_value_map_elim(ctx, elim, new_expr);
    }

    *new_expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_VALUE_MAP_ELIM,
        .value_map_elim = elim
    };

    return DY_NO;
}

dy_ternary_t dy_eval_type_map_elim(struct dy_check_ctx ctx, struct dy_core_type_map_elim elim, struct dy_core_expr *new_expr)
{
    return DY_YES;
}

dy_ternary_t dy_eval_both(struct dy_check_ctx ctx, struct dy_core_pair both, struct dy_core_expr *new_expr)
{
    struct dy_core_expr first;
    dy_ternary_t first_is_valid = dy_eval_expr(ctx, *both.first, &first);
    if (first_is_valid == DY_NO) {
        return DY_NO;
    }

    struct dy_core_expr second;
    dy_ternary_t second_is_valid = dy_eval_expr(ctx, *both.second, &second);
    if (second_is_valid == DY_NO) {
        return DY_NO;
    }

    *new_expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_BOTH,
        .both = {
            .first = alloc_expr(ctx, first),
            .second = alloc_expr(ctx, second),
        }
    };

    if (first_is_valid == DY_MAYBE || second_is_valid == DY_MAYBE) {
        return DY_MAYBE;
    } else {
        return DY_YES;
    }
}

dy_ternary_t dy_eval_one_of(struct dy_check_ctx ctx, struct dy_core_pair one_of, struct dy_core_expr *new_expr)
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

dy_ternary_t dy_eval_any_of(struct dy_check_ctx ctx, struct dy_core_pair any_of, struct dy_core_expr *new_expr)
{
    struct dy_core_expr first;
    dy_ternary_t first_is_valid = dy_eval_expr(ctx, *any_of.first, &first);
    if (first_is_valid == DY_NO) {
        return DY_NO;
    }

    struct dy_core_expr second;
    dy_ternary_t second_is_valid = dy_eval_expr(ctx, *any_of.second, &second);
    if (second_is_valid == DY_NO) {
        return DY_NO;
    }

    *new_expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_ANY_OF,
        .any_of = {
            .first = alloc_expr(ctx, first),
            .second = alloc_expr(ctx, second),
        }
    };

    if (first_is_valid == DY_MAYBE || second_is_valid == DY_MAYBE) {
        return DY_MAYBE;
    } else {
        return DY_YES;
    }
}

struct dy_core_expr *alloc_expr(struct dy_check_ctx ctx, struct dy_core_expr expr)
{
    return dy_alloc(&expr, sizeof expr, ctx.allocator);
}
