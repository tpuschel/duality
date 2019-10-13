/*
 * Copyright 2017-2019 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <duality/core/are_equal.h>

#include <duality/support/assert.h>
#include "substitute.h"

static dy_ternary_t is_equal_to_both(struct dy_check_ctx ctx, struct dy_core_expr expr, struct dy_core_pair both);

static dy_ternary_t is_equal_to_any_of(struct dy_check_ctx ctx, struct dy_core_expr expr, struct dy_core_pair any_of);

static dy_ternary_t pair_is_equal_to_pair(struct dy_check_ctx ctx, struct dy_core_pair p1, struct dy_core_pair p2);

static dy_ternary_t value_map_is_equal(struct dy_check_ctx ctx, struct dy_core_value_map value_map, struct dy_core_expr expr);

static dy_ternary_t value_map_is_equal_to_value_map(struct dy_check_ctx ctx, struct dy_core_value_map value_map1, struct dy_core_value_map value_map2);

static dy_ternary_t type_map_is_equal(struct dy_check_ctx ctx, struct dy_core_type_map type_map, struct dy_core_expr expr);

static dy_ternary_t type_map_is_equal_to_type_map(struct dy_check_ctx ctx, struct dy_core_type_map type_map1, struct dy_core_type_map type_map2);

static dy_ternary_t unknown_is_equal(struct dy_check_ctx ctx, struct dy_core_unknown unknown, struct dy_core_expr expr);

static dy_ternary_t value_map_elim_is_equal(struct dy_check_ctx ctx, struct dy_core_value_map_elim elim, struct dy_core_expr expr);

static dy_ternary_t type_map_elim_is_equal(struct dy_check_ctx ctx, struct dy_core_type_map_elim elim, struct dy_core_expr expr);

static dy_ternary_t one_of_is_equal(struct dy_check_ctx ctx, struct dy_core_pair one_of, struct dy_core_expr expr);

dy_ternary_t dy_are_equal(struct dy_check_ctx ctx, struct dy_core_expr e1, struct dy_core_expr e2)
{
    if (e1.tag == DY_CORE_EXPR_BOTH) {
        return is_equal_to_both(ctx, e2, e1.both);
    }

    if (e2.tag == DY_CORE_EXPR_BOTH) {
        return is_equal_to_both(ctx, e1, e2.both);
    }

    if (e1.tag == DY_CORE_EXPR_ANY_OF) {
        return is_equal_to_any_of(ctx, e2, e1.any_of);
    }

    if (e2.tag == DY_CORE_EXPR_ANY_OF) {
        return is_equal_to_any_of(ctx, e1, e2.any_of);
    }

    if (e2.tag == DY_CORE_EXPR_UNKNOWN) {
        return unknown_is_equal(ctx, e2.unknown, e1);
    }

    if (e2.tag == DY_CORE_EXPR_VALUE_MAP_ELIM) {
        return value_map_elim_is_equal(ctx, e2.value_map_elim, e1);
    }

    if (e2.tag == DY_CORE_EXPR_TYPE_MAP_ELIM) {
        return type_map_elim_is_equal(ctx, e2.type_map_elim, e1);
    }

    if (e2.tag == DY_CORE_EXPR_ONE_OF) {
        return one_of_is_equal(ctx, e2.one_of, e1);
    }

    switch (e1.tag) {
    case DY_CORE_EXPR_VALUE_MAP:
        return value_map_is_equal(ctx, e1.value_map, e2);
    case DY_CORE_EXPR_TYPE_MAP:
        return type_map_is_equal(ctx, e1.type_map, e2);
    case DY_CORE_EXPR_VALUE_MAP_ELIM:
        return value_map_elim_is_equal(ctx, e1.value_map_elim, e2);
    case DY_CORE_EXPR_TYPE_MAP_ELIM:
        return type_map_elim_is_equal(ctx, e1.type_map_elim, e2);
    case DY_CORE_EXPR_UNKNOWN:
        return unknown_is_equal(ctx, e1.unknown, e2);
    case DY_CORE_EXPR_ONE_OF:
        return one_of_is_equal(ctx, e1.one_of, e2);
    case DY_CORE_EXPR_TYPE_OF_TYPES:
        if (e2.tag == DY_CORE_EXPR_TYPE_OF_TYPES) {
            return DY_YES;
        } else {
            return DY_NO;
        }
    case DY_CORE_EXPR_STRING:
        if (e2.tag != DY_CORE_EXPR_STRING) {
            return DY_NO;
        }

        if (dy_string_are_equal(e1.string, e2.string)) {
            return DY_YES;
        } else {
            return DY_NO;
        }
    case DY_CORE_EXPR_TYPE_OF_STRINGS:
        if (e2.tag == DY_CORE_EXPR_TYPE_OF_STRINGS) {
            return DY_YES;
        } else {
            return DY_NO;
        }
    case DY_CORE_EXPR_BOTH:
    case DY_CORE_EXPR_ANY_OF:
        dy_bail("should not be reachable!");
    }

    DY_IMPOSSIBLE_ENUM();
}

dy_ternary_t is_equal_to_both(struct dy_check_ctx ctx, struct dy_core_expr expr, struct dy_core_pair both)
{
    if (expr.tag == DY_CORE_EXPR_BOTH) {
        return pair_is_equal_to_pair(ctx, expr.both, both);
    }

    dy_ternary_t first_res = dy_are_equal(ctx, expr, *both.first);

    dy_ternary_t second_res = dy_are_equal(ctx, expr, *both.second);

    return dy_ternary_conjunction(first_res, second_res);
}

dy_ternary_t is_equal_to_any_of(struct dy_check_ctx ctx, struct dy_core_expr expr, struct dy_core_pair any_of)
{
    if (expr.tag == DY_CORE_EXPR_ANY_OF) {
        return pair_is_equal_to_pair(ctx, expr.any_of, any_of);
    }

    dy_ternary_t first_res = dy_are_equal(ctx, expr, *any_of.first);

    dy_ternary_t second_res = dy_are_equal(ctx, expr, *any_of.second);

    return dy_ternary_disjunction(first_res, second_res);
}

dy_ternary_t pair_is_equal_to_pair(struct dy_check_ctx ctx, struct dy_core_pair p1, struct dy_core_pair p2)
{
    dy_ternary_t first_res = dy_are_equal(ctx, *p1.first, *p2.first);

    dy_ternary_t second_res = dy_are_equal(ctx, *p1.second, *p2.second);

    return dy_ternary_conjunction(first_res, second_res);
}

dy_ternary_t value_map_is_equal(struct dy_check_ctx ctx, struct dy_core_value_map value_map, struct dy_core_expr expr)
{
    if (expr.tag != DY_CORE_EXPR_VALUE_MAP) {
        return DY_NO;
    }

    return value_map_is_equal_to_value_map(ctx, value_map, expr.value_map);
}

dy_ternary_t value_map_is_equal_to_value_map(struct dy_check_ctx ctx, struct dy_core_value_map value_map1, struct dy_core_value_map value_map2)
{
    if (value_map1.polarity != value_map2.polarity) {
        return DY_NO;
    }

    dy_ternary_t first_res = dy_are_equal(ctx, *value_map1.e1, *value_map2.e1);

    dy_ternary_t second_res = dy_are_equal(ctx, *value_map1.e2, *value_map2.e2);

    return dy_ternary_conjunction(first_res, second_res);
}

dy_ternary_t type_map_is_equal(struct dy_check_ctx ctx, struct dy_core_type_map type_map, struct dy_core_expr expr)
{
    if (expr.tag != DY_CORE_EXPR_TYPE_MAP) {
        return DY_NO;
    }

    return type_map_is_equal_to_type_map(ctx, type_map, expr.type_map);
}

dy_ternary_t type_map_is_equal_to_type_map(struct dy_check_ctx ctx, struct dy_core_type_map type_map1, struct dy_core_type_map type_map2)
{
    if (type_map1.polarity != type_map2.polarity) {
        return DY_NO;
    }

    struct dy_core_expr id_expr = {
        .tag = DY_CORE_EXPR_UNKNOWN,
        .unknown = type_map1.arg
    };

    return dy_are_equal(ctx, *type_map1.expr, substitute(ctx, type_map2.arg.id, id_expr, *type_map2.expr));
}

dy_ternary_t one_of_is_equal(struct dy_check_ctx ctx, struct dy_core_pair one_of, struct dy_core_expr expr)
{
    if (expr.tag != DY_CORE_EXPR_ONE_OF) {
        return DY_MAYBE;
    }

    dy_ternary_t first_res = dy_are_equal(ctx, *one_of.first, *expr.one_of.first);

    dy_ternary_t second_res = dy_are_equal(ctx, *one_of.second, *expr.one_of.second);

    return dy_ternary_conjunction(first_res, second_res);
}

dy_ternary_t unknown_is_equal(struct dy_check_ctx ctx, struct dy_core_unknown unknown, struct dy_core_expr expr)
{
    if (expr.tag != DY_CORE_EXPR_UNKNOWN) {
        return DY_MAYBE;
    }

    if (unknown.id == expr.unknown.id) {
        return DY_YES;
    } else {
        return DY_MAYBE;
    }
}

dy_ternary_t value_map_elim_is_equal(struct dy_check_ctx ctx, struct dy_core_value_map_elim elim, struct dy_core_expr expr)
{
    if (expr.tag != DY_CORE_EXPR_VALUE_MAP_ELIM) {
        return DY_MAYBE;
    }

    dy_ternary_t first_res = dy_are_equal(ctx, *elim.expr, *expr.value_map_elim.expr);

    dy_ternary_t second_res = value_map_is_equal_to_value_map(ctx, elim.value_map, expr.value_map_elim.value_map);

    return dy_ternary_conjunction(first_res, second_res);
}

dy_ternary_t type_map_elim_is_equal(struct dy_check_ctx ctx, struct dy_core_type_map_elim elim, struct dy_core_expr expr)
{
    if (expr.tag != DY_CORE_EXPR_TYPE_MAP_ELIM) {
        return DY_MAYBE;
    }

    dy_ternary_t first_res = dy_are_equal(ctx, *elim.expr, *expr.type_map_elim.expr);

    dy_ternary_t second_res = type_map_is_equal_to_type_map(ctx, elim.type_map, expr.type_map_elim.type_map);

    return dy_ternary_conjunction(first_res, second_res);
}
