/*
 * Copyright 2017-2019 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <duality/core/type_of.h>

#include <duality/support/assert.h>

static struct dy_core_expr *alloc_expr(struct dy_check_ctx ctx, struct dy_core_expr expr);

struct dy_core_expr dy_type_of(struct dy_check_ctx ctx, struct dy_core_expr expr)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_VALUE_MAP:
        switch (expr.value_map.polarity) {
        case DY_CORE_POLARITY_POSITIVE:
            return (struct dy_core_expr){
                .tag = DY_CORE_EXPR_VALUE_MAP,
                .value_map = {
                    .e1 = expr.value_map.e1,
                    .e2 = alloc_expr(ctx, dy_type_of(ctx, *expr.value_map.e2)),
                    .polarity = DY_CORE_POLARITY_POSITIVE,
                }
            };
        case DY_CORE_POLARITY_NEGATIVE:
            return (struct dy_core_expr){
                .tag = DY_CORE_EXPR_TYPE_OF_TYPES
            };
        }
    case DY_CORE_EXPR_TYPE_MAP:
        switch (expr.type_map.polarity) {
        case DY_CORE_POLARITY_POSITIVE:
            return (struct dy_core_expr){
                .tag = DY_CORE_EXPR_TYPE_MAP,
                .type_map = {
                    .arg = expr.type_map.arg,
                    .expr = alloc_expr(ctx, dy_type_of(ctx, *expr.type_map.expr)),
                }
            };
        case DY_CORE_POLARITY_NEGATIVE:
            return (struct dy_core_expr){
                .tag = DY_CORE_EXPR_TYPE_OF_TYPES
            };
        }
    case DY_CORE_EXPR_VALUE_MAP_ELIM:
        return *expr.value_map_elim.value_map.e2;
    case DY_CORE_EXPR_TYPE_MAP_ELIM:
        return *expr.type_map_elim.type_map.expr;
    case DY_CORE_EXPR_BOTH:
        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_BOTH,
            .both = {
                .first = alloc_expr(ctx, dy_type_of(ctx, *expr.both.first)),
                .second = alloc_expr(ctx, dy_type_of(ctx, *expr.both.second)),
            }
        };
    case DY_CORE_EXPR_ONE_OF:
        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_ANY_OF,
            .any_of = {
                .first = alloc_expr(ctx, dy_type_of(ctx, *expr.one_of.first)),
                .second = alloc_expr(ctx, dy_type_of(ctx, *expr.one_of.second)),
            }
        };
    case DY_CORE_EXPR_ANY_OF:
        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_TYPE_OF_TYPES
        };
    case DY_CORE_EXPR_UNKNOWN:
        return *expr.unknown.type;
    case DY_CORE_EXPR_STRING:
        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_TYPE_OF_STRINGS
        };
    case DY_CORE_EXPR_TYPE_OF_TYPES:
        // fallthrough
    case DY_CORE_EXPR_TYPE_OF_STRINGS:
        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_TYPE_OF_TYPES
        };
    }

    DY_IMPOSSIBLE_ENUM();
}

struct dy_core_expr *alloc_expr(struct dy_check_ctx ctx, struct dy_core_expr expr)
{
    return dy_alloc(&expr, sizeof expr, ctx.allocator);
}
