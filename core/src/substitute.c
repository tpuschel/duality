/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "substitute.h"

#include <duality/support/assert.h>
#include <duality/support/allocator.h>

static struct dy_core_expr *alloc_expr(struct dy_core_expr expr);

static struct dy_core_expr_map substitute_expr_map(size_t id, struct dy_core_expr sub, struct dy_core_expr_map expr_map);

static struct dy_core_type_map substitute_type_map(size_t id, struct dy_core_expr sub, struct dy_core_type_map type_map);

struct dy_core_expr substitute(size_t id, struct dy_core_expr sub, struct dy_core_expr expr)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_END:
        // fallthrough
    case DY_CORE_EXPR_TYPE_OF_STRINGS:
        // fallthrough
    case DY_CORE_EXPR_PRINT:
        // fallthrough
    case DY_CORE_EXPR_STRING:
        return expr;
    case DY_CORE_EXPR_EXPR_MAP:
        expr.expr_map = substitute_expr_map(id, sub, expr.expr_map);
        return expr;
    case DY_CORE_EXPR_TYPE_MAP:
        expr.type_map = substitute_type_map(id, sub, expr.type_map);
        return expr;
    case DY_CORE_EXPR_UNKNOWN:
        if (expr.unknown.id == id) {
            return sub;
        } else {
            expr.unknown.type = alloc_expr(substitute(id, sub, *expr.unknown.type));
            return expr;
        }
    case DY_CORE_EXPR_BOTH:
        expr.both.e1 = alloc_expr(substitute(id, sub, *expr.both.e1));
        expr.both.e2 = alloc_expr(substitute(id, sub, *expr.both.e2));
        return expr;
    case DY_CORE_EXPR_ONE_OF:
        expr.one_of.first = alloc_expr(substitute(id, sub, *expr.one_of.first));
        expr.one_of.second = alloc_expr(substitute(id, sub, *expr.one_of.second));
        return expr;
    case DY_CORE_EXPR_EXPR_MAP_ELIM:
        expr.expr_map_elim.expr = alloc_expr(substitute(id, sub, *expr.expr_map_elim.expr));
        expr.expr_map_elim.expr_map = substitute_expr_map(id, sub, expr.expr_map_elim.expr_map);
        return expr;
    case DY_CORE_EXPR_TYPE_MAP_ELIM:
        expr.type_map_elim.expr = alloc_expr(substitute(id, sub, *expr.type_map_elim.expr));
        expr.type_map_elim.type_map = substitute_type_map(id, sub, expr.type_map_elim.type_map);
        return expr;
    case DY_CORE_EXPR_INFERENCE_CTX:
        expr.inference_ctx.type = alloc_expr(substitute(id, sub, *expr.inference_ctx.type));
        expr.inference_ctx.expr = alloc_expr(substitute(id, sub, *expr.inference_ctx.expr));
        return expr;
    }

    DY_IMPOSSIBLE_ENUM();
}

static struct dy_core_expr_map substitute_expr_map(size_t id, struct dy_core_expr sub, struct dy_core_expr_map expr_map)
{
    expr_map.e1 = alloc_expr(substitute(id, sub, *expr_map.e1));
    expr_map.e2 = alloc_expr(substitute(id, sub, *expr_map.e2));
    return expr_map;
}

static struct dy_core_type_map substitute_type_map(size_t id, struct dy_core_expr sub, struct dy_core_type_map type_map)
{
    type_map.arg_type = alloc_expr(substitute(id, sub, *type_map.arg_type));
    if (id != type_map.arg_id) {
        type_map.expr = alloc_expr(substitute(id, sub, *type_map.expr));
    }
    return type_map;
}

struct dy_core_expr *alloc_expr(struct dy_core_expr expr)
{
    return dy_alloc_and_copy(&expr, sizeof expr);
}
