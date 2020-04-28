/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_SUBSTITUTE_H
#define DY_SUBSTITUTE_H

#include "core.h"

static inline struct dy_core_expr substitute(struct dy_core_expr expr, size_t id, struct dy_core_expr sub);

static inline struct dy_core_equality_map substitute_equality_map(struct dy_core_equality_map equality_map, size_t id, struct dy_core_expr sub);

static inline struct dy_core_type_map substitute_type_map(struct dy_core_type_map type_map, size_t id, struct dy_core_expr sub);

struct dy_core_expr substitute(struct dy_core_expr expr, size_t id, struct dy_core_expr sub)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_CUSTOM:
        return expr.custom.substitute(expr.custom.data, id, sub);
    case DY_CORE_EXPR_END:
        // fallthrough
    case DY_CORE_EXPR_SYMBOL:
        return dy_core_expr_retain(expr);
    case DY_CORE_EXPR_EQUALITY_MAP:
        expr.equality_map = substitute_equality_map(expr.equality_map, id, sub);
        return expr;
    case DY_CORE_EXPR_TYPE_MAP:
        expr.type_map = substitute_type_map(expr.type_map, id, sub);
        return expr;
    case DY_CORE_EXPR_VARIABLE:
        if (expr.variable.id == id) {
            return dy_core_expr_retain(sub);
        } else {
            expr.variable.type = dy_core_expr_new(substitute(*expr.variable.type, id, sub));
            return expr;
        }
    case DY_CORE_EXPR_INFERENCE_VARIABLE:
        if (expr.variable.id == id) {
            return dy_core_expr_retain(sub);
        } else {
            expr.inference_variable.type = dy_core_expr_new(substitute(*expr.inference_variable.type, id, sub));
            return expr;
        }
    case DY_CORE_EXPR_BOTH:
        expr.both.e1 = dy_core_expr_new(substitute(*expr.both.e1, id, sub));
        expr.both.e2 = dy_core_expr_new(substitute(*expr.both.e2, id, sub));
        return expr;
    case DY_CORE_EXPR_ONE_OF:
        expr.one_of.first = dy_core_expr_new(substitute(*expr.one_of.first, id, sub));
        expr.one_of.second = dy_core_expr_new(substitute(*expr.one_of.second, id, sub));
        return expr;
    case DY_CORE_EXPR_EQUALITY_MAP_ELIM:
        expr.equality_map_elim.expr = dy_core_expr_new(substitute(*expr.equality_map_elim.expr, id, sub));
        expr.equality_map_elim.map = substitute_equality_map(expr.equality_map_elim.map, id, sub);
        return expr;
    case DY_CORE_EXPR_TYPE_MAP_ELIM:
        expr.type_map_elim.expr = dy_core_expr_new(substitute(*expr.type_map_elim.expr, id, sub));
        expr.type_map_elim.map = substitute_type_map(expr.type_map_elim.map, id, sub);
        return expr;
    case DY_CORE_EXPR_INFERENCE_TYPE_MAP:
        expr.inference_type_map = substitute_type_map(expr.inference_type_map, id, sub);
        return expr;
    case DY_CORE_EXPR_RECURSION:
        expr.recursion.map = substitute_type_map(expr.recursion.map, id, sub);
        return expr;
    }

    DY_IMPOSSIBLE_ENUM();
}

static struct dy_core_equality_map substitute_equality_map(struct dy_core_equality_map equality_map, size_t id, struct dy_core_expr sub)
{
    equality_map.e1 = dy_core_expr_new(substitute(*equality_map.e1, id, sub));
    equality_map.e2 = dy_core_expr_new(substitute(*equality_map.e2, id, sub));
    return equality_map;
}

static struct dy_core_type_map substitute_type_map(struct dy_core_type_map type_map, size_t id, struct dy_core_expr sub)
{
    type_map.binding.type = dy_core_expr_new(substitute(*type_map.binding.type, id, sub));
    if (id != type_map.binding.id) {
        type_map.expr = dy_core_expr_new(substitute(*type_map.expr, id, sub));
    } else {
        type_map.expr = dy_core_expr_retain_ptr(type_map.expr);
    }
    return type_map;
}

#endif // DY_SUBSTITUTE_H
