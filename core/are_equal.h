/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_ARE_EQUAL_H
#define DY_ARE_EQUAL_H

#include "substitute.h"
#include "../support/bail.h"

/**
 * The functions here define equality for all objects of Core.
 *
 * Equality is generally purely syntactical, except for the following objects:
 *   - Type maps are alpha-converted (might use De-Bruijn indices at some point).
 *   - Positive and negative 'junction' act like conjunction/disjunction on their constituents.
 *   - Recursion is treated like its infinite unfolding instead of its finite syntactical representation.
 */

static inline dy_ternary_t dy_are_equal(struct dy_core_expr e1, struct dy_core_expr e2);

static inline dy_ternary_t is_equal_to_junction_positive(struct dy_core_expr expr, struct dy_core_junction junction);

static inline dy_ternary_t is_equal_to_junction_negative(struct dy_core_expr expr, struct dy_core_junction junction);

static inline dy_ternary_t junction_is_equal_to_junction(struct dy_core_junction p1, struct dy_core_junction p2);

static inline dy_ternary_t equality_map_is_equal(struct dy_core_equality_map equality_map, struct dy_core_expr expr);

static inline dy_ternary_t equality_map_is_equal_to_equality_map(struct dy_core_equality_map equality_map1, struct dy_core_equality_map equality_map2);

static inline dy_ternary_t type_map_is_equal(struct dy_core_type_map type_map, struct dy_core_expr expr);

static inline dy_ternary_t type_map_is_equal_to_type_map(struct dy_core_type_map type_map1, struct dy_core_type_map type_map2);

static inline dy_ternary_t variable_is_equal(struct dy_core_variable variable, struct dy_core_expr expr);

static inline dy_ternary_t equality_map_elim_is_equal(struct dy_core_equality_map_elim elim, struct dy_core_expr expr);

static inline dy_ternary_t type_map_elim_is_equal(struct dy_core_type_map_elim elim, struct dy_core_expr expr);

static inline dy_ternary_t one_of_is_equal(struct dy_core_one_of one_of, struct dy_core_expr expr);

dy_ternary_t dy_are_equal(struct dy_core_expr e1, struct dy_core_expr e2)
{
    if (e1.tag == DY_CORE_EXPR_JUNCTION && e1.junction.polarity == DY_CORE_POLARITY_POSITIVE) {
        return is_equal_to_junction_positive(e2, e1.junction);
    }

    if (e2.tag == DY_CORE_EXPR_JUNCTION && e2.junction.polarity == DY_CORE_POLARITY_POSITIVE) {
        return is_equal_to_junction_positive(e1, e2.junction);
    }

    if (e1.tag == DY_CORE_EXPR_JUNCTION && e1.junction.polarity == DY_CORE_POLARITY_NEGATIVE) {
        return is_equal_to_junction_negative(e2, e1.junction);
    }

    if (e2.tag == DY_CORE_EXPR_JUNCTION && e2.junction.polarity == DY_CORE_POLARITY_NEGATIVE) {
        return is_equal_to_junction_negative(e1, e2.junction);
    }

    if (e2.tag == DY_CORE_EXPR_VARIABLE) {
        return variable_is_equal(e2.variable, e1);
    }

    if (e2.tag == DY_CORE_EXPR_INFERENCE_VARIABLE) {
        return variable_is_equal(e2.inference_variable, e1);
    }

    if (e2.tag == DY_CORE_EXPR_EQUALITY_MAP_ELIM) {
        return equality_map_elim_is_equal(e2.equality_map_elim, e1);
    }

    if (e2.tag == DY_CORE_EXPR_TYPE_MAP_ELIM) {
        return type_map_elim_is_equal(e2.type_map_elim, e1);
    }

    if (e2.tag == DY_CORE_EXPR_ONE_OF) {
        return one_of_is_equal(e2.one_of, e1);
    }

    if (e2.tag == DY_CORE_EXPR_CUSTOM) {
        return e2.custom.is_equal(e2.custom.data, e1);
    }

    switch (e1.tag) {
    case DY_CORE_EXPR_EQUALITY_MAP:
        return equality_map_is_equal(e1.equality_map, e2);
    case DY_CORE_EXPR_TYPE_MAP:
        return type_map_is_equal(e1.type_map, e2);
    case DY_CORE_EXPR_EQUALITY_MAP_ELIM:
        return equality_map_elim_is_equal(e1.equality_map_elim, e2);
    case DY_CORE_EXPR_TYPE_MAP_ELIM:
        return type_map_elim_is_equal(e1.type_map_elim, e2);
    case DY_CORE_EXPR_VARIABLE:
        return variable_is_equal(e1.variable, e2);
    case DY_CORE_EXPR_INFERENCE_VARIABLE:
        return variable_is_equal(e1.inference_variable, e2);
    case DY_CORE_EXPR_ONE_OF:
        return one_of_is_equal(e1.one_of, e2);
    case DY_CORE_EXPR_END:
        if (e2.tag == DY_CORE_EXPR_END && e1.end_polarity == e2.end_polarity) {
            return DY_YES;
        } else {
            return DY_NO;
        }
    case DY_CORE_EXPR_CUSTOM:
        return e1.custom.is_equal(e1.custom.data, e2);
    case DY_CORE_EXPR_SYMBOL:
        if (e2.tag == DY_CORE_EXPR_SYMBOL) {
            return DY_YES;
        } else {
            return DY_NO;
        }
    case DY_CORE_EXPR_RECURSION:
        dy_bail("Not yet implemented");
    case DY_CORE_EXPR_JUNCTION:
        // fallthrough
    case DY_CORE_EXPR_INFERENCE_TYPE_MAP:
        dy_bail("Should not be reachable!");
    }

    dy_bail("Impossible core object.");
}

dy_ternary_t is_equal_to_junction_positive(struct dy_core_expr expr, struct dy_core_junction junction)
{
    if (expr.tag == DY_CORE_EXPR_JUNCTION && expr.junction.polarity == DY_CORE_POLARITY_POSITIVE) {
        return junction_is_equal_to_junction(expr.junction, junction);
    }

    dy_ternary_t first_res = dy_are_equal(expr, *junction.e1);
    if (first_res == DY_NO) {
        return DY_NO;
    }

    dy_ternary_t second_res = dy_are_equal(expr, *junction.e2);
    if (second_res == DY_NO) {
        return DY_NO;
    }

    if (first_res == DY_MAYBE || second_res == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

dy_ternary_t is_equal_to_junction_negative(struct dy_core_expr expr, struct dy_core_junction junction)
{
    if (expr.tag == DY_CORE_EXPR_JUNCTION && expr.junction.polarity == DY_CORE_POLARITY_NEGATIVE) {
        return junction_is_equal_to_junction(expr.junction, junction);
    }

    dy_ternary_t first_res = dy_are_equal(expr, *junction.e1);
    if (first_res == DY_YES) {
        return DY_YES;
    }

    dy_ternary_t second_res = dy_are_equal(expr, *junction.e2);
    if (second_res == DY_YES) {
        return DY_YES;
    }

    if (first_res == DY_MAYBE || second_res == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_NO;
}

dy_ternary_t junction_is_equal_to_junction(struct dy_core_junction p1, struct dy_core_junction p2)
{
    dy_ternary_t first_res = dy_are_equal(*p1.e1, *p2.e1);
    if (first_res == DY_NO) {
        return DY_NO;
    }

    dy_ternary_t second_res = dy_are_equal(*p1.e2, *p2.e2);
    if (second_res == DY_NO) {
        return DY_NO;
    }

    if (first_res == DY_MAYBE || second_res == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

dy_ternary_t equality_map_is_equal(struct dy_core_equality_map equality_map, struct dy_core_expr expr)
{
    if (expr.tag != DY_CORE_EXPR_EQUALITY_MAP) {
        return DY_NO;
    }

    return equality_map_is_equal_to_equality_map(equality_map, expr.equality_map);
}

dy_ternary_t equality_map_is_equal_to_equality_map(struct dy_core_equality_map equality_map1, struct dy_core_equality_map equality_map2)
{
    if (equality_map1.polarity != equality_map2.polarity || equality_map1.is_implicit != equality_map2.is_implicit) {
        return DY_NO;
    }

    dy_ternary_t first_res = dy_are_equal(*equality_map1.e1, *equality_map2.e1);
    if (first_res == DY_NO) {
        return DY_NO;
    }

    dy_ternary_t second_res = dy_are_equal(*equality_map1.e2, *equality_map2.e2);
    if (second_res == DY_NO) {
        return DY_NO;
    }

    if (first_res == DY_MAYBE || second_res == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

dy_ternary_t type_map_is_equal(struct dy_core_type_map type_map, struct dy_core_expr expr)
{
    if (expr.tag != DY_CORE_EXPR_TYPE_MAP) {
        return DY_NO;
    }

    return type_map_is_equal_to_type_map(type_map, expr.type_map);
}

dy_ternary_t type_map_is_equal_to_type_map(struct dy_core_type_map type_map1, struct dy_core_type_map type_map2)
{
    if (type_map1.polarity != type_map2.polarity || type_map1.is_implicit != type_map2.is_implicit) {
        return DY_NO;
    }

    struct dy_core_expr id_expr = {
        .tag = DY_CORE_EXPR_VARIABLE,
        .variable = type_map1.binding
    };

    struct dy_core_expr e = substitute(*type_map2.expr, type_map2.binding.id, id_expr);

    dy_ternary_t result = dy_are_equal(*type_map1.expr, e);

    dy_core_expr_release(e);

    return result;
}

dy_ternary_t one_of_is_equal(struct dy_core_one_of one_of, struct dy_core_expr expr)
{
    if (expr.tag != DY_CORE_EXPR_ONE_OF) {
        return DY_MAYBE;
    }

    dy_ternary_t first_res = dy_are_equal(*one_of.first, *expr.one_of.first);
    if (first_res == DY_NO) {
        return DY_NO;
    }

    dy_ternary_t second_res = dy_are_equal(*one_of.second, *expr.one_of.second);
    if (second_res == DY_NO) {
        return DY_NO;
    }

    if (first_res == DY_MAYBE || second_res == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

dy_ternary_t variable_is_equal(struct dy_core_variable variable, struct dy_core_expr expr)
{
    if (expr.tag != DY_CORE_EXPR_VARIABLE) {
        return DY_MAYBE;
    }

    if (variable.id == expr.variable.id) {
        return DY_YES;
    } else {
        return DY_MAYBE;
    }
}

dy_ternary_t equality_map_elim_is_equal(struct dy_core_equality_map_elim elim, struct dy_core_expr expr)
{
    if (expr.tag != DY_CORE_EXPR_EQUALITY_MAP_ELIM) {
        return DY_MAYBE;
    }

    dy_ternary_t first_res = dy_are_equal(*elim.expr, *expr.equality_map_elim.expr);
    if (first_res == DY_NO) {
        return DY_NO;
    }

    dy_ternary_t second_res = equality_map_is_equal_to_equality_map(elim.map, expr.equality_map_elim.map);
    if (second_res == DY_NO) {
        return DY_NO;
    }

    if (first_res == DY_MAYBE || second_res == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

dy_ternary_t type_map_elim_is_equal(struct dy_core_type_map_elim elim, struct dy_core_expr expr)
{
    if (expr.tag != DY_CORE_EXPR_TYPE_MAP_ELIM) {
        return DY_MAYBE;
    }

    dy_ternary_t first_res = dy_are_equal(*elim.expr, *expr.type_map_elim.expr);
    if (first_res == DY_NO) {
        return DY_NO;
    }

    dy_ternary_t second_res = type_map_is_equal_to_type_map(elim.map, expr.type_map_elim.map);
    if (second_res == DY_NO) {
        return DY_NO;
    }

    if (first_res == DY_MAYBE || second_res == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

#endif // DY_ARE_EQUAL_H
