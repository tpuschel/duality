/*
 * Copyright 2020-2021 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core.h"
#include "are_equal.h"
#include "substitute.h"

/**
 * This file deals with collecting/resolving constraints.
 */

struct dy_constraint {
    size_t id;
    struct dy_core_expr lower;
    bool have_lower;
    struct dy_core_expr upper;
    bool have_upper;
};

static inline bool dy_constraint_get(struct dy_core_ctx *ctx, size_t id, enum dy_polarity polarity, size_t start, struct dy_core_expr *result);

static inline void dy_join_constraints(struct dy_core_ctx *ctx, size_t start1, size_t start2, enum dy_polarity polarity);

static inline void dy_free_constraints_starting_at(struct dy_core_ctx *ctx, size_t start);

bool dy_constraint_get(struct dy_core_ctx *ctx, size_t id, enum dy_polarity polarity, size_t start, struct dy_core_expr *result)
{
    for (size_t i = start, size = ctx->constraints.num_elems; i < size; ++i) {
        const struct dy_constraint *c = dy_array_pos(&ctx->constraints, i);
        if (c->id != id) {
            continue;
        }

        if (!c->have_lower && !c->have_upper) {
            dy_bail("impossible");
        }

        struct dy_core_expr var_expr = {
            .tag = DY_CORE_EXPR_VARIABLE,
            .variable_id = id
        };

        if (polarity == DY_POLARITY_POSITIVE) {
            if (!c->have_lower) {
                return false;
            }

            struct dy_core_expr e;
            if (dy_substitute(ctx, c->lower, id, var_expr, &e)) {
                *result = (struct dy_core_expr){
                    .tag = DY_CORE_EXPR_PROBLEM,
                    .problem = {
                        .is_implicit = true,
                        .polarity = DY_POLARITY_POSITIVE,
                        .tag = DY_CORE_RECURSION,
                        .recursion = {
                            .id = id,
                            .expr = dy_core_expr_new(e)
                        }
                    }
                };
            } else {
                *result = dy_core_expr_retain(ctx, c->lower);
            }
        } else {
            if (!c->have_upper) {
                return false;
            }

            struct dy_core_expr e;
            if (dy_substitute(ctx, c->upper, id, var_expr, &e)) {
                *result = (struct dy_core_expr){
                    .tag = DY_CORE_EXPR_PROBLEM,
                    .problem = {
                        .is_implicit = true,
                        .polarity = DY_POLARITY_NEGATIVE,
                        .tag = DY_CORE_RECURSION,
                        .recursion = {
                            .id = id,
                            .expr = dy_core_expr_new(e)
                        }
                    }
                };
            } else {
                *result = dy_core_expr_retain(ctx, c->upper);
            }
        }

        return true;
    }

    return false;
}

void dy_join_constraints(struct dy_core_ctx *ctx, size_t start1, size_t start2, enum dy_polarity polarity)
{
    for (size_t i = start2; i < ctx->constraints.num_elems; ++i) {
        const struct dy_constraint *c = dy_array_pos(&ctx->constraints, i);

        bool merged = false;
        for (size_t k = start1; k < start2; ++k) {
            struct dy_constraint *c2 = dy_array_pos(&ctx->constraints, k);
            if (c->id != c2->id) {
                continue;
            }

            if (c->have_lower && c2->have_lower) {
                if (dy_are_equal(ctx, c->lower, c2->lower) == DY_YES) {
                    dy_core_expr_release(ctx, c->lower);
                } else {
                    c2->lower = (struct dy_core_expr){
                        .tag = DY_CORE_EXPR_PROBLEM,
                        .problem = {
                            .polarity = dy_flip_polarity(polarity),
                            .is_implicit = true,
                            .tag = DY_CORE_PAIR,
                            .pair = {
                                .left = dy_core_expr_new(c2->lower),
                                .right = dy_core_expr_new(c->lower)
                            }
                        }
                    };
                }
            } else if (c->have_lower) {
                c2->lower = c->lower;
                c2->have_lower = true;
            }

            if (c->have_upper && c2->have_upper) {
                if (dy_are_equal(ctx, c->upper, c2->upper) == DY_YES) {
                    dy_core_expr_release(ctx, c->upper);
                } else {
                    c2->upper = (struct dy_core_expr){
                        .tag = DY_CORE_EXPR_PROBLEM,
                        .problem = {
                            .polarity = polarity,
                            .is_implicit = true,
                            .tag = DY_CORE_PAIR,
                            .pair = {
                                .left = dy_core_expr_new(c2->upper),
                                .right = dy_core_expr_new(c->upper)
                            }
                        }
                    };
                }
            } else if (c->have_upper) {
                c2->upper = c->upper;
                c2->have_upper = true;
            }

            merged = true;
            break;
        }

        if (merged) {
            dy_array_remove(&ctx->constraints, i);
            --i;
        }
    }
}

void dy_free_constraints_starting_at(struct dy_core_ctx *ctx, size_t start)
{
    for (size_t i = ctx->constraints.num_elems; i-- > start;) {
        const struct dy_constraint *c = dy_array_pos(&ctx->constraints, i);
        if (c->have_lower) {
             dy_core_expr_release(ctx, c->lower);
        }
        if (c->have_upper) {
             dy_core_expr_release(ctx, c->upper);
        }
        dy_array_remove(&ctx->constraints, i);
    }
}
