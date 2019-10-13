/*
 * Copyright 2017-2019 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <duality/core/check.h>

#include <duality/core/is_subtype.h>
#include <duality/core/type_of.h>

#include <duality/support/array.h>
#include <duality/support/assert.h>

static struct dy_core_expr *alloc_expr(struct dy_check_ctx ctx, struct dy_core_expr expr);

static struct dy_core_expr s_type_type = {
    .tag = DY_CORE_EXPR_TYPE_OF_TYPES
};

dy_ternary_t dy_check_expr(struct dy_check_ctx ctx, struct dy_core_expr expr, struct dy_core_expr *new_expr)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_VALUE_MAP: {
        struct dy_core_value_map value_map;
        dy_ternary_t result = dy_check_value_map(ctx, expr.value_map, &value_map);

        *new_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_VALUE_MAP,
            .value_map = value_map
        };

        return result;
    }
    case DY_CORE_EXPR_TYPE_MAP: {
        struct dy_core_type_map type_map;
        dy_ternary_t result = dy_check_type_map(ctx, expr.type_map, &type_map);

        *new_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_TYPE_MAP,
            .type_map = type_map
        };

        return result;
    }
    case DY_CORE_EXPR_VALUE_MAP_ELIM: {
        struct dy_core_value_map_elim elim;
        dy_ternary_t result = dy_check_value_map_elim(ctx, expr.value_map_elim, &elim);

        *new_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_VALUE_MAP_ELIM,
            .value_map_elim = elim
        };

        return result;
    }
    case DY_CORE_EXPR_TYPE_MAP_ELIM: {
        struct dy_core_type_map_elim elim;
        dy_ternary_t result = dy_check_type_map_elim(ctx, expr.type_map_elim, &elim);

        *new_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_TYPE_MAP_ELIM,
            .type_map_elim = elim
        };

        return result;
    }
    case DY_CORE_EXPR_BOTH: {
        struct dy_core_pair both;
        dy_ternary_t result = dy_check_both(ctx, expr.both, &both);

        *new_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_BOTH,
            .both = both
        };

        return result;
    }
    case DY_CORE_EXPR_ONE_OF:
        return dy_check_one_of(ctx, expr.one_of, new_expr);
    case DY_CORE_EXPR_ANY_OF: {
        struct dy_core_pair any_of;
        dy_ternary_t result = dy_check_any_of(ctx, expr.any_of, &any_of);

        *new_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_ANY_OF,
            .any_of = any_of
        };

        return result;
    }
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

dy_ternary_t dy_check_value_map(struct dy_check_ctx ctx, struct dy_core_value_map value_map, struct dy_core_value_map *new_value_map)
{
    struct dy_core_expr e1;
    dy_ternary_t res1 = dy_check_expr(ctx, *value_map.e1, &e1);

    struct dy_core_expr e2;
    dy_ternary_t res2 = dy_check_expr(ctx, *value_map.e2, &e2);

    *new_value_map = (struct dy_core_value_map){
        .e1 = alloc_expr(ctx, e1),
        .e2 = alloc_expr(ctx, e2),
        .polarity = value_map.polarity
    };

    return dy_ternary_conjunction(res1, res2);
}

dy_ternary_t dy_check_type_map(struct dy_check_ctx ctx, struct dy_core_type_map type_map, struct dy_core_type_map *new_type_map)
{
    struct dy_core_expr type;
    dy_ternary_t arg_res = dy_check_expr(ctx, *type_map.arg.type, &type);

    struct dy_core_expr expr;
    dy_ternary_t expr_res = dy_check_expr(ctx, *type_map.expr, &expr);

    *new_type_map = (struct dy_core_type_map){
        .arg = {
            .id = type_map.arg.id,
            .type = alloc_expr(ctx, type),
        },
        .expr = alloc_expr(ctx, expr),
        .polarity = type_map.polarity
    };

    return dy_ternary_conjunction(arg_res, expr_res);
}

dy_ternary_t dy_check_value_map_elim(struct dy_check_ctx ctx, struct dy_core_value_map_elim elim, struct dy_core_value_map_elim *new_elim)
{
    struct dy_core_expr expr;
    dy_ternary_t left_res = dy_check_expr(ctx, *elim.expr, &expr);

    struct dy_core_value_map value_map;
    dy_ternary_t right_res = dy_check_value_map(ctx, elim.value_map, &value_map);

    struct dy_core_expr value_map_expr = {
        .tag = DY_CORE_EXPR_VALUE_MAP,
        .value_map = value_map
    };

    dy_ternary_t subtype_res = dy_is_subtype(ctx, dy_type_of(ctx, expr), value_map_expr);

    *new_elim = (struct dy_core_value_map_elim){
        .expr = alloc_expr(ctx, expr),
        .value_map = value_map
    };

    return dy_ternary_conjunction(dy_ternary_conjunction(left_res, right_res), subtype_res);
}

dy_ternary_t dy_check_type_map_elim(struct dy_check_ctx ctx, struct dy_core_type_map_elim elim, struct dy_core_type_map_elim *new_elim)
{
    struct dy_core_expr expr;
    dy_ternary_t left_res = dy_check_expr(ctx, *elim.expr, &expr);

    struct dy_core_type_map type_map;
    dy_ternary_t right_res = dy_check_type_map(ctx, elim.type_map, &type_map);

    struct dy_core_expr type_map_expr = {
        .tag = DY_CORE_EXPR_TYPE_MAP,
        .type_map = type_map
    };

    dy_ternary_t subtype_res = dy_is_subtype(ctx, dy_type_of(ctx, *elim.expr), type_map_expr);

    *new_elim = (struct dy_core_type_map_elim){
        .expr = alloc_expr(ctx, expr),
        .type_map = type_map
    };

    return dy_ternary_conjunction(dy_ternary_conjunction(left_res, right_res), subtype_res);
}

dy_ternary_t dy_check_both(struct dy_check_ctx ctx, struct dy_core_pair both, struct dy_core_pair *new_both)
{
    struct dy_core_expr first;
    dy_ternary_t res1 = dy_check_expr(ctx, *both.first, &first);

    struct dy_core_expr second;
    dy_ternary_t res2 = dy_check_expr(ctx, *both.second, &second);

    *new_both = (struct dy_core_pair){
        .first = alloc_expr(ctx, first),
        .second = alloc_expr(ctx, second)
    };

    return dy_ternary_conjunction(res1, res2);
}

dy_ternary_t dy_check_any_of(struct dy_check_ctx ctx, struct dy_core_pair any_of, struct dy_core_pair *new_any_of)
{
    struct dy_core_expr first;
    dy_ternary_t res1 = dy_check_expr(ctx, *any_of.first, &first);

    struct dy_core_expr second;
    dy_ternary_t res2 = dy_check_expr(ctx, *any_of.second, &second);

    *new_any_of = (struct dy_core_pair){
        .first = alloc_expr(ctx, first),
        .second = alloc_expr(ctx, second)
    };

    return dy_ternary_conjunction(res1, res2);
}

dy_ternary_t dy_check_one_of(struct dy_check_ctx ctx, struct dy_core_pair one_of, struct dy_core_expr *new_expr)
{
    struct dy_core_expr first;
    dy_ternary_t res1 = dy_check_expr(ctx, *one_of.first, &first);

    struct dy_core_expr second;
    dy_ternary_t res2 = dy_check_expr(ctx, *one_of.second, &second);

    if (res1 == DY_YES) {
        *new_expr = first;
        return DY_YES;
    }

    if (res1 == DY_MAYBE) {
        *new_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_ONE_OF,
            .one_of = {
                .first = alloc_expr(ctx, first),
                .second = alloc_expr(ctx, second),
            }
        };

        return DY_MAYBE;
    }

    if (res2 == DY_YES) {
        *new_expr = second;
        return DY_YES;
    }

    if (res2 == DY_MAYBE) {
        *new_expr = second;
        return DY_MAYBE;
    }

    *new_expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_ONE_OF,
        .one_of = {
            .first = alloc_expr(ctx, first),
            .second = alloc_expr(ctx, second),
        }
    };

    return DY_NO;
}

struct dy_core_expr *alloc_expr(struct dy_check_ctx ctx, struct dy_core_expr expr)
{
    return dy_alloc(&expr, sizeof expr, ctx.allocator);
}
