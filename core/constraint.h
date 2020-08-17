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
struct dy_constraint_single {
    size_t id;
    struct dy_core_expr expr;
    enum dy_core_polarity polarity;
};

/**
 * Represents multiple constraints, either in a conjunctive (positive, "and")
 * or in a disjunctive (negative, "or") fashion.
 */
struct dy_constraint_multiple {
    const struct dy_constraint *c1;
    const struct dy_constraint *c2;
    enum dy_core_polarity polarity;
};

enum dy_constraint_tag {
    DY_CONSTRAINT_SINGLE,
    DY_CONSTRAINT_MULTIPLE
};

/**
 * A constraint. Can be just one or multiple.
 */
struct dy_constraint {
    union {
        struct dy_constraint_single single;
        struct dy_constraint_multiple multiple;
    };

    enum dy_constraint_tag tag;
};

/**
 * Returns both the subtype and supertype (if any) of 'id' build from 'constraint'.
 */
static inline bool dy_constraint_collect(struct dy_constraint constraint, size_t id, enum dy_core_polarity polarity, struct dy_core_expr *result);

bool dy_constraint_collect(struct dy_constraint constraint, size_t id, enum dy_core_polarity polarity, struct dy_core_expr *result)
{
    switch (constraint.tag) {
    case DY_CONSTRAINT_SINGLE:
        if (constraint.single.id != id) {
            return false;
        }

        *result = dy_core_expr_retain(constraint.single.expr);
        return true;
    case DY_CONSTRAINT_MULTIPLE: {
        struct dy_core_expr e1;
        bool r1 = dy_constraint_collect(*constraint.multiple.c1, id, polarity, &e1);

        struct dy_core_expr e2;
        bool r2 = dy_constraint_collect(*constraint.multiple.c2, id, polarity, &e2);

        if (!r1 && !r2) {
            return false;
        }

        /**
         * We never throw constraints away, even in disjunctive cases where only one
         * bound is present.
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

        if (r1 && !r2) {
            *result = e1;
            return true;
        }

        if (!r1 && r2) {
            *result = e2;
            return true;
        }

        if (polarity == constraint.multiple.polarity) {
            *result = (struct dy_core_expr){
                .tag = DY_CORE_EXPR_JUNCTION,
                .junction = {
                    .e1 = dy_core_expr_new(e1),
                    .e2 = dy_core_expr_new(e2),
                    .polarity = DY_CORE_POLARITY_NEGATIVE,
                }
            };
        } else {
            *result = (struct dy_core_expr){
                .tag = DY_CORE_EXPR_JUNCTION,
                .junction = {
                    .e1 = dy_core_expr_new(e1),
                    .e2 = dy_core_expr_new(e2),
                    .polarity = DY_CORE_POLARITY_POSITIVE,
                }
            };
        }

        return true;
    }
    }
}
