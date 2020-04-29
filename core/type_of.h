/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_TYPE_OF_H
#define DY_TYPE_OF_H

#include "core.h"

/**
 * Determines the type of the expression 'expr'.
 */
static inline struct dy_core_expr dy_type_of(struct dy_core_ctx *ctx, struct dy_core_expr expr);

struct dy_core_expr dy_type_of(struct dy_core_ctx *ctx, struct dy_core_expr expr)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_EQUALITY_MAP:
        if (dy_core_expr_is_computation(*expr.equality_map.e1)) {
            return (struct dy_core_expr){
                .tag = DY_CORE_EXPR_TYPE_MAP,
                .type_map = {
                    .binding = {
                        .id = ctx->running_id++,
                        .type = dy_core_expr_new(dy_type_of(ctx, *expr.equality_map.e1)),
                    },
                    .expr = dy_core_expr_new(dy_type_of(ctx, *expr.equality_map.e2)),
                    .polarity = DY_CORE_POLARITY_POSITIVE,
                    .is_implicit = expr.equality_map.is_implicit,
                }
            };
        } else {
            dy_core_expr_retain_ptr(expr.equality_map.e1);
            expr.equality_map.e2 = dy_core_expr_new(dy_type_of(ctx, *expr.equality_map.e2));
            expr.equality_map.polarity = DY_CORE_POLARITY_POSITIVE;
            return expr;
        }
    case DY_CORE_EXPR_TYPE_MAP:
        dy_core_expr_retain_ptr(expr.type_map.binding.type);
        expr.type_map.expr = dy_core_expr_new(dy_type_of(ctx, *expr.type_map.expr));
        expr.type_map.polarity = DY_CORE_POLARITY_POSITIVE;
        return expr;
    case DY_CORE_EXPR_EQUALITY_MAP_ELIM:
        return dy_core_expr_retain(*expr.equality_map_elim.map.e2);
    case DY_CORE_EXPR_TYPE_MAP_ELIM:
        return dy_core_expr_retain(*expr.type_map_elim.map.expr);
    case DY_CORE_EXPR_JUNCTION:
        expr.junction.e1 = dy_core_expr_new(dy_type_of(ctx, *expr.junction.e1));
        expr.junction.e2 = dy_core_expr_new(dy_type_of(ctx, *expr.junction.e2));
        expr.junction.polarity = DY_CORE_POLARITY_POSITIVE;
        return expr;
    case DY_CORE_EXPR_ONE_OF:
        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_JUNCTION,
            .junction = {
                .e1 = dy_core_expr_new(dy_type_of(ctx, *expr.one_of.first)),
                .e2 = dy_core_expr_new(dy_type_of(ctx, *expr.one_of.second)),
                .polarity = DY_CORE_POLARITY_NEGATIVE,
            }
        };
    case DY_CORE_EXPR_VARIABLE:
        return dy_core_expr_retain(*expr.variable.type);
    case DY_CORE_EXPR_INFERENCE_VARIABLE:
        return dy_core_expr_retain(*expr.inference_variable.type);
    case DY_CORE_EXPR_CUSTOM:
        return expr.custom.type_of(expr.custom.data, ctx);
    case DY_CORE_EXPR_INFERENCE_TYPE_MAP:
        dy_bail("Should not happen\n");
    case DY_CORE_EXPR_RECURSION: {
        struct dy_core_expr type = dy_type_of(ctx, *expr.recursion.map.expr);

        if (dy_core_expr_is_bound(expr.recursion.map.binding.id, type)) {
            return (struct dy_core_expr){
                .tag = DY_CORE_EXPR_RECURSION,
                .recursion = {
                    .map = {
                        .binding = {
                            .id = expr.recursion.map.binding.id,
                            .type = dy_core_expr_retain_ptr(expr.recursion.map.binding.type),
                        },
                        .expr = dy_core_expr_new(type),
                        .polarity = DY_CORE_POLARITY_POSITIVE,
                    },
                    .check_result = expr.recursion.check_result,
                }
            };
        } else {
            return type;
        }
    }
    case DY_CORE_EXPR_END:
        // fallthrough
    case DY_CORE_EXPR_SYMBOL:
        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_SYMBOL,
        };
    }

    dy_bail("Impossible object type.");
}

#endif // DY_TYPE_OF_H
