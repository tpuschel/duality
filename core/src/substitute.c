/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "substitute.h"

#include <duality/support/assert.h>

static struct dy_core_expr *alloc_expr(struct dy_check_ctx ctx, struct dy_core_expr expr);

static struct dy_core_value_map substitute_value_map(struct dy_check_ctx ctx, size_t id, struct dy_core_expr sub, struct dy_core_value_map value_map);

static struct dy_core_type_map substitute_type_map(struct dy_check_ctx ctx, size_t id, struct dy_core_expr sub, struct dy_core_type_map type_map);

struct dy_core_expr substitute(struct dy_check_ctx ctx, size_t id, struct dy_core_expr sub, struct dy_core_expr expr)
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
    case DY_CORE_EXPR_VALUE_MAP:
        expr.value_map = substitute_value_map(ctx, id, sub, expr.value_map);
        return expr;
    case DY_CORE_EXPR_TYPE_MAP:
        expr.type_map = substitute_type_map(ctx, id, sub, expr.type_map);
        return expr;
    case DY_CORE_EXPR_UNKNOWN:
        if (expr.unknown.id == id) {
            return sub;
        } else {
            expr.unknown.type = alloc_expr(ctx, substitute(ctx, id, sub, *expr.unknown.type));
            return expr;
        }
    case DY_CORE_EXPR_BOTH:
        expr.both.e1 = alloc_expr(ctx, substitute(ctx, id, sub, *expr.both.e1));
        expr.both.e2 = alloc_expr(ctx, substitute(ctx, id, sub, *expr.both.e2));
        return expr;
    case DY_CORE_EXPR_ONE_OF:
        expr.one_of.first = alloc_expr(ctx, substitute(ctx, id, sub, *expr.one_of.first));
        expr.one_of.second = alloc_expr(ctx, substitute(ctx, id, sub, *expr.one_of.second));
        return expr;
    case DY_CORE_EXPR_VALUE_MAP_ELIM:
        expr.value_map_elim.expr = alloc_expr(ctx, substitute(ctx, id, sub, *expr.value_map_elim.expr));
        expr.value_map_elim.value_map = substitute_value_map(ctx, id, sub, expr.value_map_elim.value_map);
        return expr;
    case DY_CORE_EXPR_TYPE_MAP_ELIM:
        expr.type_map_elim.expr = alloc_expr(ctx, substitute(ctx, id, sub, *expr.type_map_elim.expr));
        expr.type_map_elim.type_map = substitute_type_map(ctx, id, sub, expr.type_map_elim.type_map);
        return expr;
    }

    DY_IMPOSSIBLE_ENUM();
}

static struct dy_core_value_map substitute_value_map(struct dy_check_ctx ctx, size_t id, struct dy_core_expr sub, struct dy_core_value_map value_map)
{
    value_map.e1 = alloc_expr(ctx, substitute(ctx, id, sub, *value_map.e1));
    value_map.e2 = alloc_expr(ctx, substitute(ctx, id, sub, *value_map.e2));
    return value_map;
}

static struct dy_core_type_map substitute_type_map(struct dy_check_ctx ctx, size_t id, struct dy_core_expr sub, struct dy_core_type_map type_map)
{
    type_map.arg_type = alloc_expr(ctx, substitute(ctx, id, sub, *type_map.arg_type));
    type_map.expr = alloc_expr(ctx, substitute(ctx, id, sub, *type_map.expr));
    return type_map;
}

struct dy_core_expr *alloc_expr(struct dy_check_ctx ctx, struct dy_core_expr expr)
{
    return dy_alloc(&expr, sizeof expr, ctx.allocator);
}
