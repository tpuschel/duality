/*
 * Copyright 2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core.h"
#include "are_equal.h"

/**
 * This file deals with collecting/resolving constraints.
 */

/** An inference variable + its associated upper or lower bound. */
struct dy_constraint {
    size_t id;
    struct dy_core_expr expr;
    enum dy_core_polarity polarity;
};

/**
 * Returns both the subtype and supertype (if any) of 'id' build from 'constraint'.
 */
static inline bool dy_constraint_get(struct dy_core_ctx *ctx, size_t id, size_t start, struct dy_core_expr *result);

static inline void dy_join_constraints(struct dy_core_ctx *ctx, size_t start1, size_t start2, enum dy_core_polarity polarity);

static inline void dy_free_first_constraints(struct dy_core_ctx *ctx, size_t start1, size_t start2);

bool dy_constraint_get(struct dy_core_ctx *ctx, size_t id, size_t start, struct dy_core_expr *result)
{
    for (size_t i = start, size = ctx->constraints.num_elems; i < size; ++i) {
        const struct dy_constraint *c = dy_array_pos(ctx->constraints, i);
        if (c->id == id) {
            *result = dy_core_expr_retain(ctx, c->expr);
            return true;
        }
    }

    return false;
}

void dy_join_constraints(struct dy_core_ctx *ctx, size_t start1, size_t start2, enum dy_core_polarity polarity)
{
    for (size_t i = start2; i < ctx->constraints.num_elems; ++i) {
        const struct dy_constraint *c = dy_array_pos(ctx->constraints, i);

        bool merged = false;
        for (size_t k = start1; k < start2; ++k) {
            struct dy_constraint *c2 = dy_array_pos(ctx->constraints, k);

            if (c->id == c2->id) {
                if (dy_are_equal(ctx, c->expr, c2->expr) == DY_YES) {
                    dy_core_expr_release(ctx, c->expr);
                } else if (polarity == c->polarity) {
                    c2->expr = (struct dy_core_expr){
                        .tag = DY_CORE_EXPR_JUNCTION,
                        .junction = {
                            .e1 = dy_core_expr_new(c2->expr),
                            .e2 = dy_core_expr_new(c->expr),
                            .polarity = DY_CORE_POLARITY_NEGATIVE
                        }
                    };
                } else {
                    c2->expr = (struct dy_core_expr){
                        .tag = DY_CORE_EXPR_JUNCTION,
                        .junction = {
                            .e1 = dy_core_expr_new(c2->expr),
                            .e2 = dy_core_expr_new(c->expr),
                            .polarity = DY_CORE_POLARITY_POSITIVE
                        }
                    };
                }

                merged = true;
            }
        }

        if (merged) {
            dy_array_remove(&ctx->constraints, i);
            --i;
        }

        /*
         * We never throw constraints away, even in disjunctive (polarity = negative)
         * cases where only one bound is present.
         *
         * Image the following expression: '[a] -> [b] -> try { a "a"; b "b" }'
         * There are essentially two ways the types of a and b can be inferred,
         * too permissive: '[a Any] -> [b Any] -> try { a "a"; b "b" }'
         * or too restrictive: '[a "a" ~> ...] -> [b "b" ~> ...] -> try { a "a"; b "b" }'.
         *
         * Dropping the bounds leads to the permissive variant, while keeping them
         * leads to the restrictive variant.
         *
         * In each of the two cases, the expression can be rewritten to avoid those problems:
         * '[x] -> try { x "a"; x "b" }' where the type of 'x' is inferred to be
         * 'choice { "a" ~> ..., "b" ~> ... }', which is neither too permissive nor restrictive.
         *
         * Since the expression is suboptimal, the question is how to best nudge
         * the user to reformulate their expression. For these purposes, we have chosen
         * the always keep the bounds.
         */
    }
}

void dy_free_first_constraints(struct dy_core_ctx *ctx, size_t start1, size_t start2)
{
    for (size_t i = start2; i-- > start1;) {
        const struct dy_constraint *c = dy_array_pos(ctx->constraints, i);
        dy_core_expr_release(ctx, c->expr);
        dy_array_remove(&ctx->constraints, i);
    }
}
