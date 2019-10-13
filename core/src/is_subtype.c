/*
 * Copyright 2017-2019 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <duality/core/is_subtype.h>

#include <duality/core/are_equal.h>
#include <duality/core/type_of.h>

#include <duality/support/assert.h>

static dy_ternary_t value_map_is_subtype(struct dy_check_ctx ctx, struct dy_core_value_map value_map, struct dy_core_expr supertype);
static dy_ternary_t positive_value_map_is_subtype(struct dy_check_ctx ctx, struct dy_core_value_map value_map, struct dy_core_expr supertype);
static dy_ternary_t negative_value_map_is_subtype(struct dy_check_ctx ctx, struct dy_core_value_map value_map, struct dy_core_expr supertype);

static dy_ternary_t type_map_is_subtype(struct dy_check_ctx ctx, struct dy_core_type_map type_map, struct dy_core_expr supertype);
static dy_ternary_t positive_type_map_is_subtype(struct dy_check_ctx ctx, struct dy_core_type_map type_map, struct dy_core_expr supertype);
static dy_ternary_t negative_type_map_is_subtype(struct dy_check_ctx ctx, struct dy_core_type_map type_map, struct dy_core_expr supertype);

static dy_ternary_t both_is_subtype(struct dy_check_ctx ctx, struct dy_core_pair both, struct dy_core_expr expr);
static dy_ternary_t any_of_is_subtype(struct dy_check_ctx ctx, struct dy_core_pair any_of, struct dy_core_expr expr);

static dy_ternary_t is_sutype_of_both(struct dy_check_ctx ctx, struct dy_core_expr expr, struct dy_core_pair both);
static dy_ternary_t is_subtype_of_any_of(struct dy_check_ctx ctx, struct dy_core_expr expr, struct dy_core_pair any_of);

static dy_ternary_t pair_is_subtype_of_pair(struct dy_check_ctx ctx, struct dy_core_pair p1, struct dy_core_pair p2);

dy_ternary_t dy_is_subtype(struct dy_check_ctx ctx, struct dy_core_expr subtype, struct dy_core_expr supertype)
{
    if (subtype.tag == DY_CORE_EXPR_BOTH) {
        return both_is_subtype(ctx, subtype.both, supertype);
    }

    if (supertype.tag == DY_CORE_EXPR_BOTH) {
        return is_sutype_of_both(ctx, subtype, supertype.both);
    }

    if (subtype.tag == DY_CORE_EXPR_ANY_OF) {
        return any_of_is_subtype(ctx, subtype.any_of, supertype);
    }

    if (supertype.tag == DY_CORE_EXPR_ANY_OF) {
        return is_subtype_of_any_of(ctx, subtype, supertype.any_of);
    }

    if (supertype.tag == DY_CORE_EXPR_VALUE_MAP_ELIM || supertype.tag == DY_CORE_EXPR_TYPE_MAP_ELIM || supertype.tag == DY_CORE_EXPR_UNKNOWN || supertype.tag == DY_CORE_EXPR_ONE_OF) {
        return dy_are_equal(ctx, subtype, supertype);
    }

    switch (subtype.tag) {
    case DY_CORE_EXPR_VALUE_MAP:
        return value_map_is_subtype(ctx, subtype.value_map, supertype);
    case DY_CORE_EXPR_TYPE_MAP:
        return type_map_is_subtype(ctx, subtype.type_map, supertype);
    case DY_CORE_EXPR_VALUE_MAP_ELIM:
        // fallthrough
    case DY_CORE_EXPR_TYPE_MAP_ELIM:
        // fallthrough
    case DY_CORE_EXPR_ONE_OF:
        // fallthrough
    case DY_CORE_EXPR_UNKNOWN:
        return dy_are_equal(ctx, subtype, supertype);
    case DY_CORE_EXPR_TYPE_OF_TYPES:
        if (supertype.tag == DY_CORE_EXPR_TYPE_OF_TYPES) {
            return DY_YES;
        } else {
            return DY_NO;
        }
    case DY_CORE_EXPR_TYPE_OF_STRINGS:
        if (supertype.tag == DY_CORE_EXPR_TYPE_OF_STRINGS) {
            return DY_YES;
        } else {
            return DY_NO;
        }
    case DY_CORE_EXPR_BOTH:
    case DY_CORE_EXPR_ANY_OF:
    case DY_CORE_EXPR_STRING:
        dy_bail("Should not be reachable!");
    }

    DY_IMPOSSIBLE_ENUM();
}

dy_ternary_t value_map_is_subtype(struct dy_check_ctx ctx, struct dy_core_value_map value_map, struct dy_core_expr supertype)
{
    switch (value_map.polarity) {
    case DY_CORE_POLARITY_POSITIVE:
        return positive_value_map_is_subtype(ctx, value_map, supertype);
    case DY_CORE_POLARITY_NEGATIVE:
        return negative_value_map_is_subtype(ctx, value_map, supertype);
    }

    DY_IMPOSSIBLE_ENUM();
}

dy_ternary_t positive_value_map_is_subtype(struct dy_check_ctx ctx, struct dy_core_value_map value_map, struct dy_core_expr supertype)
{
    if (supertype.tag == DY_CORE_EXPR_VALUE_MAP) {
        dy_ternary_t are_equal = dy_are_equal(ctx, *value_map.e1, *supertype.value_map.e1);

        dy_ternary_t is_subtype = dy_is_subtype(ctx, *value_map.e2, *supertype.value_map.e2);

        return dy_ternary_conjunction(are_equal, is_subtype);
    }

    if (supertype.tag == DY_CORE_EXPR_TYPE_MAP && supertype.type_map.polarity == DY_CORE_POLARITY_NEGATIVE) {
        dy_ternary_t is_subtype_in = dy_is_subtype(ctx, dy_type_of(ctx, *value_map.e1), *supertype.type_map.arg.type);

        dy_ternary_t is_subtype_out = dy_is_subtype(ctx, *value_map.e2, *supertype.type_map.expr);

        return dy_ternary_conjunction(is_subtype_in, is_subtype_out);
    }

    return DY_NO;
}

dy_ternary_t negative_value_map_is_subtype(struct dy_check_ctx ctx, struct dy_core_value_map value_map, struct dy_core_expr supertype)
{
    if (supertype.tag == DY_CORE_EXPR_VALUE_MAP && supertype.value_map.polarity == DY_CORE_POLARITY_NEGATIVE) {
        dy_ternary_t are_equal = dy_are_equal(ctx, *value_map.e1, *supertype.value_map.e1);

        dy_ternary_t is_subtype = dy_is_subtype(ctx, *value_map.e2, *supertype.value_map.e2);

        return dy_ternary_conjunction(are_equal, is_subtype);
    }

    return DY_NO;
}

dy_ternary_t type_map_is_subtype(struct dy_check_ctx ctx, struct dy_core_type_map type_map, struct dy_core_expr supertype)
{
    switch (type_map.polarity) {
    case DY_CORE_POLARITY_POSITIVE:
        return positive_type_map_is_subtype(ctx, type_map, supertype);
    case DY_CORE_POLARITY_NEGATIVE:
        return negative_type_map_is_subtype(ctx, type_map, supertype);
    }

    DY_IMPOSSIBLE_ENUM();
}

dy_ternary_t positive_type_map_is_subtype(struct dy_check_ctx ctx, struct dy_core_type_map type_map, struct dy_core_expr supertype)
{
    if (supertype.tag == DY_CORE_EXPR_VALUE_MAP && supertype.value_map.polarity == DY_CORE_POLARITY_NEGATIVE) {
        dy_ternary_t is_subtype_in = dy_is_subtype(ctx, dy_type_of(ctx, *supertype.value_map.e1), *type_map.arg.type);

        dy_ternary_t is_subtype_out = dy_is_subtype(ctx, *type_map.expr, *supertype.value_map.e2);

        return dy_ternary_conjunction(is_subtype_in, is_subtype_out);
    }

    if (supertype.tag == DY_CORE_EXPR_TYPE_MAP && supertype.type_map.polarity == DY_CORE_POLARITY_POSITIVE) {
        dy_ternary_t is_subtype_in = dy_is_subtype(ctx, *supertype.type_map.arg.type, *type_map.arg.type);

        dy_ternary_t is_subtype_out = dy_is_subtype(ctx, *type_map.expr, *supertype.type_map.expr);

        return dy_ternary_conjunction(is_subtype_in, is_subtype_out);
    }

    return DY_NO;
}

dy_ternary_t negative_type_map_is_subtype(struct dy_check_ctx ctx, struct dy_core_type_map type_map, struct dy_core_expr supertype)
{
    if (supertype.tag == DY_CORE_EXPR_TYPE_MAP && supertype.type_map.polarity == DY_CORE_POLARITY_NEGATIVE) {
        dy_ternary_t is_subtype_in = dy_is_subtype(ctx, *supertype.type_map.arg.type, *type_map.arg.type);

        dy_ternary_t is_subtype_out = dy_is_subtype(ctx, *type_map.expr, *supertype.type_map.expr);

        return dy_ternary_conjunction(is_subtype_in, is_subtype_out);
    }

    return DY_NO;
}

dy_ternary_t both_is_subtype(struct dy_check_ctx ctx, struct dy_core_pair both, struct dy_core_expr expr)
{
    if (expr.tag == DY_CORE_EXPR_BOTH) {
        return pair_is_subtype_of_pair(ctx, both, expr.both);
    }

    dy_ternary_t first_res = dy_is_subtype(ctx, *both.first, expr);
    if (first_res == DY_YES) {
        return DY_YES;
    }

    dy_ternary_t second_res = dy_is_subtype(ctx, *both.second, expr);
    if (second_res == DY_YES) {
        return DY_YES;
    }

    if (first_res == DY_MAYBE || second_res == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_NO;
}

dy_ternary_t any_of_is_subtype(struct dy_check_ctx ctx, struct dy_core_pair any_of, struct dy_core_expr expr)
{
    if (expr.tag == DY_CORE_EXPR_ANY_OF) {
        return pair_is_subtype_of_pair(ctx, any_of, expr.any_of);
    }

    dy_ternary_t first_result = dy_is_subtype(ctx, *any_of.first, expr);

    dy_ternary_t second_result = dy_is_subtype(ctx, *any_of.second, expr);

    return dy_ternary_conjunction(first_result, second_result);
}

dy_ternary_t is_sutype_of_both(struct dy_check_ctx ctx, struct dy_core_expr expr, struct dy_core_pair both)
{
    if (expr.tag == DY_CORE_EXPR_BOTH) {
        return pair_is_subtype_of_pair(ctx, expr.both, both);
    }

    dy_ternary_t first_result = dy_is_subtype(ctx, expr, *both.first);

    dy_ternary_t second_result = dy_is_subtype(ctx, expr, *both.second);

    return dy_ternary_conjunction(first_result, second_result);
}

dy_ternary_t is_subtype_of_any_of(struct dy_check_ctx ctx, struct dy_core_expr expr, struct dy_core_pair any_of)
{
    if (expr.tag == DY_CORE_EXPR_ANY_OF) {
        return pair_is_subtype_of_pair(ctx, expr.any_of, any_of);
    }

    dy_ternary_t first_res = dy_is_subtype(ctx, expr, *any_of.first);
    if (first_res == DY_YES) {
        return DY_YES;
    }

    dy_ternary_t second_res = dy_is_subtype(ctx, expr, *any_of.second);
    if (second_res == DY_YES) {
        return DY_YES;
    }

    if (first_res == DY_MAYBE || second_res == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_NO;
}

dy_ternary_t pair_is_subtype_of_pair(struct dy_check_ctx ctx, struct dy_core_pair p1, struct dy_core_pair p2)
{
    dy_ternary_t first_result1 = dy_is_subtype(ctx, *p1.first, *p2.first);

    dy_ternary_t second_result1 = dy_is_subtype(ctx, *p1.second, *p2.second);

    dy_ternary_t result1 = dy_ternary_conjunction(first_result1, second_result1);
    if (result1 == DY_YES) {
        return DY_YES;
    }


    dy_ternary_t first_result2 = dy_is_subtype(ctx, *p1.first, *p2.second);

    dy_ternary_t second_result2 = dy_is_subtype(ctx, *p1.second, *p2.first);

    dy_ternary_t result2 = dy_ternary_conjunction(first_result2, second_result2);
    if (result2 == DY_YES) {
        return DY_YES;
    }

    if (result1 == DY_MAYBE || result2 == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_NO;
}
