/*
 * Copyright 2017-2021 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core.h"

/**
 * Determines the type of the expression 'expr'.
 */
static inline struct dy_core_expr dy_type_of(struct dy_core_ctx *ctx, struct dy_core_expr expr);

struct dy_core_expr dy_type_of(struct dy_core_ctx *ctx, struct dy_core_expr expr)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_PROBLEM:
        expr.problem.polarity = DY_POLARITY_POSITIVE;

        switch (expr.problem.tag) {
        case DY_CORE_FUNCTION:
            dy_core_expr_retain_ptr(ctx, expr.problem.function.type);

            dy_array_add(&ctx->free_variables, &(struct dy_free_var){
                .id = expr.problem.function.id,
                .type = *expr.problem.function.type
            });

            expr.problem.function.expr = dy_core_expr_new(dy_type_of(ctx, *expr.problem.function.expr));

            ctx->free_variables.num_elems--;

            return expr;
        case DY_CORE_PAIR:
            expr.problem.pair.left = dy_core_expr_new(dy_type_of(ctx, *expr.problem.pair.left));
            expr.problem.pair.right = dy_core_expr_new(dy_type_of(ctx, *expr.problem.pair.right));
            return expr;
        case DY_CORE_RECURSION:
            dy_array_add(&ctx->free_variables, &(struct dy_free_var){
                .id = expr.problem.recursion.id,
                .type = {
                    .tag = DY_CORE_EXPR_VARIABLE,
                    .variable_id = expr.problem.recursion.id
                }
            });

            expr.problem.recursion.expr = dy_core_expr_new(dy_type_of(ctx, *expr.problem.recursion.expr));

            ctx->free_variables.num_elems--;

            return expr;
        }

        dy_bail("impossible");
    case DY_CORE_EXPR_SOLUTION:
        if (expr.solution.tag == DY_CORE_FUNCTION) {
            dy_core_expr_retain_ptr(ctx, expr.solution.expr);
        }

        expr.solution.out = dy_core_expr_new(dy_type_of(ctx, *expr.solution.out));
        return expr;
    case DY_CORE_EXPR_APPLICATION:
        return dy_core_expr_retain(ctx, *expr.application.solution.out);
    case DY_CORE_EXPR_VARIABLE:
        for (size_t i = ctx->free_variables.num_elems; i-- > 0;) {
            const struct dy_free_var *free_var = dy_array_pos(&ctx->free_variables, i);
            if (free_var->id == expr.variable_id) {
                return dy_core_expr_retain(ctx, free_var->type);
            }
        }

        dy_bail("impossible");
    case DY_CORE_EXPR_ANY:
    case DY_CORE_EXPR_VOID:
        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_VOID
        };
    case DY_CORE_EXPR_INFERENCE_CTX:
        dy_bail("impossible");
    case DY_CORE_EXPR_INFERENCE_VAR:
        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_ANY
        };
    case DY_CORE_EXPR_CUSTOM: {
        const struct dy_core_custom_shared *s = dy_array_pos(&ctx->custom_shared, expr.custom.id);
        return s->type_of(ctx, expr.custom.data);
    }
    }

    dy_bail("Impossible object type.");
}
