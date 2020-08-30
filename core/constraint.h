/*
 * Copyright 2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core.h"

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

static inline const struct dy_constraint *dy_constraint_new(struct dy_constraint constraint);

static inline struct dy_constraint dy_constraint_retain(struct dy_constraint constraint);

static inline const struct dy_constraint *dy_constraint_retain_ptr(const struct dy_constraint *constraint);

static inline void dy_constraint_release(struct dy_constraint constraint);

static inline void dy_constraint_release_ptr(const struct dy_constraint *constraint);

static inline void dy_join_constraints(struct dy_constraint c1, bool have_c1, enum dy_core_polarity polarity, struct dy_constraint c2, bool have_c2, struct dy_constraint *c3, bool *have_c3);

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

    dy_bail("Impossible constraint type");
}

static const size_t dy_constraint_pre_padding = DY_RC_PRE_PADDING(struct dy_constraint);
static const size_t dy_constraint_post_padding = DY_RC_POST_PADDING(struct dy_constraint);

const struct dy_constraint *dy_constraint_new(struct dy_constraint constraint)
{
    return dy_rc_new(&constraint, sizeof(struct dy_constraint), dy_constraint_pre_padding, dy_constraint_post_padding);
}

struct dy_constraint dy_constraint_retain(struct dy_constraint constraint)
{
    switch (constraint.tag) {
    case DY_CONSTRAINT_SINGLE:
        dy_core_expr_retain(constraint.single.expr);
        return constraint;
    case DY_CONSTRAINT_MULTIPLE:
        dy_constraint_retain_ptr(constraint.multiple.c1);
        dy_constraint_retain_ptr(constraint.multiple.c2);
        return constraint;
    }
    
    dy_bail("Impossible constraint type.");
}

const struct dy_constraint *dy_constraint_retain_ptr(const struct dy_constraint *constraint)
{
    return dy_rc_retain(constraint, dy_constraint_pre_padding, dy_constraint_post_padding);
}

void dy_constraint_release(struct dy_constraint constraint)
{
    switch (constraint.tag) {
    case DY_CONSTRAINT_SINGLE:
        dy_core_expr_release(constraint.single.expr);
        return;
    case DY_CONSTRAINT_MULTIPLE:
        dy_constraint_release_ptr(constraint.multiple.c1);
        dy_constraint_release_ptr(constraint.multiple.c2);
        return;
    }
    
    dy_bail("Impossible constraint type.");
}

void dy_constraint_release_ptr(const struct dy_constraint *constraint)
{
    struct dy_constraint shallow_copy = *constraint;
    if (dy_rc_release(constraint, dy_constraint_pre_padding, dy_constraint_post_padding) == 0) {
        dy_constraint_release(shallow_copy);
    }
}

void dy_join_constraints(struct dy_constraint c1, bool have_c1, enum dy_core_polarity polarity, struct dy_constraint c2, bool have_c2, struct dy_constraint *c3, bool *have_c3)
{
    if (have_c1 && have_c2) {
        *c3 = (struct dy_constraint){
            .tag = DY_CONSTRAINT_MULTIPLE,
            .multiple = {
                .c1 = dy_constraint_new(c1),
                .c2 = dy_constraint_new(c2),
                .polarity = polarity,
            }
        };
        *have_c3 = true;
    } else if (have_c1) {
        *c3 = c1;
        *have_c3 = true;
    } else if (have_c2) {
        *c3 = c2;
        *have_c3 = true;
    }
}

