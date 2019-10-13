/*
 * Copyright 2017-2019 Thorben Hasenpusch <t.hasenpusch@icloud.com>
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
    case DY_CORE_EXPR_TYPE_OF_TYPES:
        // fallthrough
    case DY_CORE_EXPR_TYPE_OF_STRINGS:
        // fallthrough
    case DY_CORE_EXPR_STRING:
        return expr;
    case DY_CORE_EXPR_VALUE_MAP:
        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_VALUE_MAP,
            .value_map = substitute_value_map(ctx, id, sub, expr.value_map)
        };
    case DY_CORE_EXPR_TYPE_MAP:
        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_TYPE_MAP,
            .type_map = substitute_type_map(ctx, id, sub, expr.type_map)
        };
    case DY_CORE_EXPR_UNKNOWN:
        if (expr.unknown.id == id) {
            return sub;
        } else {
            return expr;
        }
    case DY_CORE_EXPR_BOTH: {
        struct dy_core_expr first = substitute(ctx, id, sub, *expr.both.first);
        struct dy_core_expr second = substitute(ctx, id, sub, *expr.both.second);

        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_BOTH,
            .both = {
                .first = alloc_expr(ctx, first),
                .second = alloc_expr(ctx, second),
            }
        };
    }
    case DY_CORE_EXPR_ONE_OF: {
        struct dy_core_expr first = substitute(ctx, id, sub, *expr.one_of.first);
        struct dy_core_expr second = substitute(ctx, id, sub, *expr.one_of.second);

        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_ONE_OF,
            .one_of = {
                .first = alloc_expr(ctx, first),
                .second = alloc_expr(ctx, second),
            }
        };
    }
    case DY_CORE_EXPR_ANY_OF: {
        struct dy_core_expr first = substitute(ctx, id, sub, *expr.any_of.first);
        struct dy_core_expr second = substitute(ctx, id, sub, *expr.any_of.second);

        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_ANY_OF,
            .any_of = {
                .first = alloc_expr(ctx, first),
                .second = alloc_expr(ctx, second),
            }
        };
    }
    case DY_CORE_EXPR_VALUE_MAP_ELIM: {
        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_VALUE_MAP_ELIM,
            .value_map_elim = {
                .expr = alloc_expr(ctx, substitute(ctx, id, sub, *expr.value_map_elim.expr)),
                .value_map = substitute_value_map(ctx, id, sub, expr.value_map_elim.value_map),
            }
        };
    }
    case DY_CORE_EXPR_TYPE_MAP_ELIM:
        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_TYPE_MAP_ELIM,
            .type_map_elim = {
                .expr = alloc_expr(ctx, substitute(ctx, id, sub, *expr.type_map_elim.expr)),
                .type_map = substitute_type_map(ctx, id, sub, expr.type_map_elim.type_map),
            }
        };
    }

    DY_IMPOSSIBLE_ENUM();
}

static struct dy_core_value_map substitute_value_map(struct dy_check_ctx ctx, size_t id, struct dy_core_expr sub, struct dy_core_value_map value_map)
{
    struct dy_core_expr e1 = substitute(ctx, id, sub, *value_map.e1);
    struct dy_core_expr e2 = substitute(ctx, id, sub, *value_map.e2);

    return (struct dy_core_value_map){
        .e1 = alloc_expr(ctx, e1),
        .e2 = alloc_expr(ctx, e2),
        .polarity = value_map.polarity
    };
}

static struct dy_core_type_map substitute_type_map(struct dy_check_ctx ctx, size_t id, struct dy_core_expr sub, struct dy_core_type_map type_map)
{
    struct dy_core_expr type = substitute(ctx, id, sub, *type_map.arg.type);
    struct dy_core_expr expr = substitute(ctx, id, sub, *type_map.expr);

    return (struct dy_core_type_map){
        .arg = {
            .id = type_map.arg.id,
            .type = alloc_expr(ctx, type),
        },
        .expr = alloc_expr(ctx, expr),
        .polarity = type_map.polarity
    };
}

struct dy_core_expr *alloc_expr(struct dy_check_ctx ctx, struct dy_core_expr expr)
{
    return dy_alloc(&expr, sizeof expr, ctx.allocator);
}
