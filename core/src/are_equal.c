/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <duality/core/are_equal.h>

#include <duality/support/assert.h>
#include "substitute.h"

static dy_ternary_t is_equal_to_both_positive(struct dy_check_ctx ctx, struct dy_core_expr expr, struct dy_core_both both);

static dy_ternary_t is_equal_to_both_negative(struct dy_check_ctx ctx, struct dy_core_expr expr, struct dy_core_both both);

static dy_ternary_t both_is_equal_to_both(struct dy_check_ctx ctx, struct dy_core_both p1, struct dy_core_both p2);

static dy_ternary_t expr_map_is_equal(struct dy_check_ctx ctx, struct dy_core_expr_map expr_map, struct dy_core_expr expr);

static dy_ternary_t expr_map_is_equal_to_expr_map(struct dy_check_ctx ctx, struct dy_core_expr_map expr_map1, struct dy_core_expr_map expr_map2);

static dy_ternary_t type_map_is_equal(struct dy_check_ctx ctx, struct dy_core_type_map type_map, struct dy_core_expr expr);

static dy_ternary_t type_map_is_equal_to_type_map(struct dy_check_ctx ctx, struct dy_core_type_map type_map1, struct dy_core_type_map type_map2);

static dy_ternary_t unknown_is_equal(struct dy_check_ctx ctx, struct dy_core_unknown unknown, struct dy_core_expr expr);

static dy_ternary_t expr_map_elim_is_equal(struct dy_check_ctx ctx, struct dy_core_expr_map_elim elim, struct dy_core_expr expr);

static dy_ternary_t type_map_elim_is_equal(struct dy_check_ctx ctx, struct dy_core_type_map_elim elim, struct dy_core_expr expr);

static dy_ternary_t one_of_is_equal(struct dy_check_ctx ctx, struct dy_core_one_of one_of, struct dy_core_expr expr);

dy_ternary_t dy_are_equal(struct dy_check_ctx ctx, struct dy_core_expr e1, struct dy_core_expr e2)
{
    if (e1.tag == DY_CORE_EXPR_BOTH && e1.both.polarity == DY_CORE_POLARITY_POSITIVE) {
        return is_equal_to_both_positive(ctx, e2, e1.both);
    }

    if (e2.tag == DY_CORE_EXPR_BOTH && e2.both.polarity == DY_CORE_POLARITY_POSITIVE) {
        return is_equal_to_both_positive(ctx, e1, e2.both);
    }

    if (e1.tag == DY_CORE_EXPR_BOTH && e1.both.polarity == DY_CORE_POLARITY_NEGATIVE) {
        return is_equal_to_both_negative(ctx, e2, e1.both);
    }

    if (e2.tag == DY_CORE_EXPR_BOTH && e2.both.polarity == DY_CORE_POLARITY_NEGATIVE) {
        return is_equal_to_both_negative(ctx, e1, e2.both);
    }

    if (e2.tag == DY_CORE_EXPR_UNKNOWN) {
        return unknown_is_equal(ctx, e2.unknown, e1);
    }

    if (e2.tag == DY_CORE_EXPR_EXPR_MAP_ELIM) {
        return expr_map_elim_is_equal(ctx, e2.expr_map_elim, e1);
    }

    if (e2.tag == DY_CORE_EXPR_TYPE_MAP_ELIM) {
        return type_map_elim_is_equal(ctx, e2.type_map_elim, e1);
    }

    if (e2.tag == DY_CORE_EXPR_ONE_OF) {
        return one_of_is_equal(ctx, e2.one_of, e1);
    }

    switch (e1.tag) {
    case DY_CORE_EXPR_EXPR_MAP:
        return expr_map_is_equal(ctx, e1.expr_map, e2);
    case DY_CORE_EXPR_TYPE_MAP:
        return type_map_is_equal(ctx, e1.type_map, e2);
    case DY_CORE_EXPR_EXPR_MAP_ELIM:
        return expr_map_elim_is_equal(ctx, e1.expr_map_elim, e2);
    case DY_CORE_EXPR_TYPE_MAP_ELIM:
        return type_map_elim_is_equal(ctx, e1.type_map_elim, e2);
    case DY_CORE_EXPR_UNKNOWN:
        return unknown_is_equal(ctx, e1.unknown, e2);
    case DY_CORE_EXPR_ONE_OF:
        return one_of_is_equal(ctx, e1.one_of, e2);
    case DY_CORE_EXPR_END:
        if (e2.tag == DY_CORE_EXPR_END && e1.end_polarity == e2.end_polarity) {
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
    case DY_CORE_EXPR_PRINT:
        if (e2.tag == DY_CORE_EXPR_PRINT) {
            return DY_YES;
        } else {
            return DY_NO;
        }
    case DY_CORE_EXPR_BOTH:
        dy_bail("should not be reachable!");
    }

    DY_IMPOSSIBLE_ENUM();
}

dy_ternary_t is_equal_to_both_positive(struct dy_check_ctx ctx, struct dy_core_expr expr, struct dy_core_both both)
{
    if (expr.tag == DY_CORE_EXPR_BOTH && expr.both.polarity == DY_CORE_POLARITY_POSITIVE) {
        return both_is_equal_to_both(ctx, expr.both, both);
    }

    dy_ternary_t first_res = dy_are_equal(ctx, expr, *both.e1);
    if (first_res == DY_NO) {
        return DY_NO;
    }

    dy_ternary_t second_res = dy_are_equal(ctx, expr, *both.e2);
    if (second_res == DY_NO) {
        return DY_NO;
    }

    if (first_res == DY_MAYBE || second_res == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

dy_ternary_t is_equal_to_both_negative(struct dy_check_ctx ctx, struct dy_core_expr expr, struct dy_core_both both)
{
    if (expr.tag == DY_CORE_EXPR_BOTH && expr.both.polarity == DY_CORE_POLARITY_NEGATIVE) {
        return both_is_equal_to_both(ctx, expr.both, both);
    }

    dy_ternary_t first_res = dy_are_equal(ctx, expr, *both.e1);
    if (first_res == DY_YES) {
        return DY_YES;
    }

    dy_ternary_t second_res = dy_are_equal(ctx, expr, *both.e2);
    if (second_res == DY_YES) {
        return DY_YES;
    }

    if (first_res == DY_MAYBE || second_res == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_NO;
}

dy_ternary_t both_is_equal_to_both(struct dy_check_ctx ctx, struct dy_core_both p1, struct dy_core_both p2)
{
    dy_ternary_t first_res = dy_are_equal(ctx, *p1.e1, *p2.e1);
    if (first_res == DY_NO) {
        return DY_NO;
    }

    dy_ternary_t second_res = dy_are_equal(ctx, *p1.e2, *p2.e2);
    if (second_res == DY_NO) {
        return DY_NO;
    }

    if (first_res == DY_MAYBE || second_res == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

dy_ternary_t expr_map_is_equal(struct dy_check_ctx ctx, struct dy_core_expr_map expr_map, struct dy_core_expr expr)
{
    if (expr.tag != DY_CORE_EXPR_EXPR_MAP) {
        return DY_NO;
    }

    return expr_map_is_equal_to_expr_map(ctx, expr_map, expr.expr_map);
}

dy_ternary_t expr_map_is_equal_to_expr_map(struct dy_check_ctx ctx, struct dy_core_expr_map expr_map1, struct dy_core_expr_map expr_map2)
{
    if (expr_map1.polarity != expr_map2.polarity || expr_map1.is_implicit != expr_map2.is_implicit) {
        return DY_NO;
    }

    dy_ternary_t first_res = dy_are_equal(ctx, *expr_map1.e1, *expr_map2.e1);
    if (first_res == DY_NO) {
        return DY_NO;
    }

    dy_ternary_t second_res = dy_are_equal(ctx, *expr_map1.e2, *expr_map2.e2);
    if (second_res == DY_NO) {
        return DY_NO;
    }

    if (first_res == DY_MAYBE || second_res == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
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
    if (type_map1.polarity != type_map2.polarity || type_map1.is_implicit != type_map2.is_implicit) {
        return DY_NO;
    }

    struct dy_core_expr id_expr = {
        .tag = DY_CORE_EXPR_UNKNOWN,
        .unknown = {
            .id = type_map1.arg_id,
            .type = type_map1.arg_type,
            .is_inference_var = false,
        }
    };

    return dy_are_equal(ctx, *type_map1.expr, substitute(type_map2.arg_id, id_expr, *type_map2.expr));
}

dy_ternary_t one_of_is_equal(struct dy_check_ctx ctx, struct dy_core_one_of one_of, struct dy_core_expr expr)
{
    if (expr.tag != DY_CORE_EXPR_ONE_OF) {
        return DY_MAYBE;
    }

    dy_ternary_t first_res = dy_are_equal(ctx, *one_of.first, *expr.one_of.first);
    if (first_res == DY_NO) {
        return DY_NO;
    }

    dy_ternary_t second_res = dy_are_equal(ctx, *one_of.second, *expr.one_of.second);
    if (second_res == DY_NO) {
        return DY_NO;
    }

    if (first_res == DY_MAYBE || second_res == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

dy_ternary_t unknown_is_equal(struct dy_check_ctx ctx, struct dy_core_unknown unknown, struct dy_core_expr expr)
{
    (void)ctx;

    if (expr.tag != DY_CORE_EXPR_UNKNOWN) {
        return DY_MAYBE;
    }

    if (unknown.id == expr.unknown.id) {
        return DY_YES;
    } else {
        return DY_MAYBE;
    }
}

dy_ternary_t expr_map_elim_is_equal(struct dy_check_ctx ctx, struct dy_core_expr_map_elim elim, struct dy_core_expr expr)
{
    if (expr.tag != DY_CORE_EXPR_EXPR_MAP_ELIM) {
        return DY_MAYBE;
    }

    dy_ternary_t first_res = dy_are_equal(ctx, *elim.expr, *expr.expr_map_elim.expr);
    if (first_res == DY_NO) {
        return DY_NO;
    }

    dy_ternary_t second_res = expr_map_is_equal_to_expr_map(ctx, elim.expr_map, expr.expr_map_elim.expr_map);
    if (second_res == DY_NO) {
        return DY_NO;
    }

    if (first_res == DY_MAYBE || second_res == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

dy_ternary_t type_map_elim_is_equal(struct dy_check_ctx ctx, struct dy_core_type_map_elim elim, struct dy_core_expr expr)
{
    if (expr.tag != DY_CORE_EXPR_TYPE_MAP_ELIM) {
        return DY_MAYBE;
    }

    dy_ternary_t first_res = dy_are_equal(ctx, *elim.expr, *expr.type_map_elim.expr);
    if (first_res == DY_NO) {
        return DY_NO;
    }

    dy_ternary_t second_res = type_map_is_equal_to_type_map(ctx, elim.type_map, expr.type_map_elim.type_map);
    if (second_res == DY_NO) {
        return DY_NO;
    }

    if (first_res == DY_MAYBE || second_res == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}
