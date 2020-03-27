/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <duality/core/type_of.h>

#include <duality/support/assert.h>

static struct dy_core_expr *alloc_expr(struct dy_check_ctx ctx, struct dy_core_expr expr);

struct dy_core_expr dy_type_of(struct dy_check_ctx ctx, struct dy_core_expr expr)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_EXPR_MAP:
        expr.expr_map.e2 = alloc_expr(ctx, dy_type_of(ctx, *expr.expr_map.e2));
        expr.expr_map.polarity = DY_CORE_POLARITY_POSITIVE;
        return expr;
    case DY_CORE_EXPR_TYPE_MAP:
        expr.type_map.expr = alloc_expr(ctx, dy_type_of(ctx, *expr.type_map.expr));
        expr.type_map.polarity = DY_CORE_POLARITY_POSITIVE;
        return expr;
    case DY_CORE_EXPR_EXPR_MAP_ELIM:
        return *expr.expr_map_elim.expr_map.e2;
    case DY_CORE_EXPR_TYPE_MAP_ELIM:
        return *expr.type_map_elim.type_map.expr;
    case DY_CORE_EXPR_BOTH:
        expr.both.e1 = alloc_expr(ctx, dy_type_of(ctx, *expr.both.e1));
        expr.both.e2 = alloc_expr(ctx, dy_type_of(ctx, *expr.both.e2));
        expr.both.polarity = DY_CORE_POLARITY_POSITIVE;
        return expr;
    case DY_CORE_EXPR_ONE_OF:
        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_BOTH,
            .both = {
                .e1 = alloc_expr(ctx, dy_type_of(ctx, *expr.one_of.first)),
                .e2 = alloc_expr(ctx, dy_type_of(ctx, *expr.one_of.second)),
                .polarity = DY_CORE_POLARITY_NEGATIVE,
            }
        };
    case DY_CORE_EXPR_UNKNOWN:
        return *expr.unknown.type;
    case DY_CORE_EXPR_INFERENCE_CTX:
        return dy_type_of(ctx, *expr.inference_ctx.expr);
    case DY_CORE_EXPR_STRING:
        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_TYPE_OF_STRINGS
        };
    case DY_CORE_EXPR_END:
        // fallthrough
    case DY_CORE_EXPR_TYPE_OF_STRINGS:
        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_END,
            .end_polarity = DY_CORE_POLARITY_POSITIVE
        };
    case DY_CORE_EXPR_PRINT:
        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_TYPE_MAP,
            .type_map = {
                .arg_id = (*ctx.running_id)++,
                .arg_type = alloc_expr(ctx, (struct dy_core_expr){ .tag = DY_CORE_EXPR_TYPE_OF_STRINGS }),
                .expr = alloc_expr(ctx, (struct dy_core_expr){ .tag = DY_CORE_EXPR_TYPE_OF_STRINGS }),
                .polarity = DY_CORE_POLARITY_POSITIVE,
                .is_implicit = false,
            }
        };
    }

    DY_IMPOSSIBLE_ENUM();
}

struct dy_core_expr *alloc_expr(struct dy_check_ctx ctx, struct dy_core_expr expr)
{
    return dy_alloc(&expr, sizeof expr, ctx.allocator);
}
