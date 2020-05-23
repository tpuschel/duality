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
 *
 * Note that throughout this file, lower and upper bound
 * are synonymous with subtype and supertype, respectively.
 */

/**
 * Represents lower and upper bound of any inference variable.
 * Note that neither bound has to necessarily exist.
 */
struct dy_constraint_range {
    struct dy_core_expr subtype;
    bool have_subtype;
    struct dy_core_expr supertype;
    bool have_supertype;
};

/** An inference variable + its associated upper/lower bounds. */
struct dy_constraint_single {
    size_t id;
    struct dy_constraint_range range;
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
static inline struct dy_constraint_range dy_constraint_collect(struct dy_constraint constraint, size_t id);

/** Just ref-count stuff. */
static inline struct dy_constraint_range retain_range(struct dy_constraint_range range);
static inline void release_range(struct dy_constraint_range range);

/**
 * Merges two ranges. The type of junction determines the way they are merged.
 * Refer to their implementation for details.
 */
static inline struct dy_constraint_range constraint_conjunction(struct dy_constraint_range range1, struct dy_constraint_range range2);
static inline struct dy_constraint_range constraint_disjunction(struct dy_constraint_range range1, struct dy_constraint_range range2);

struct dy_constraint_range dy_constraint_collect(struct dy_constraint constraint, size_t id)
{
    switch (constraint.tag) {
    case DY_CONSTRAINT_SINGLE:
        if (constraint.single.id != id) {
            return (struct dy_constraint_range){
                .have_subtype = false,
                .have_supertype = false
            };
        }

        return retain_range(constraint.single.range);
    case DY_CONSTRAINT_MULTIPLE: {
        struct dy_constraint_range range1 = dy_constraint_collect(*constraint.multiple.c1, id);
        struct dy_constraint_range range2 = dy_constraint_collect(*constraint.multiple.c2, id);

        struct dy_constraint_range r;

        switch (constraint.multiple.polarity) {
        case DY_CORE_POLARITY_POSITIVE:
            r = constraint_conjunction(range1, range2);
            break;
        case DY_CORE_POLARITY_NEGATIVE:
            r = constraint_disjunction(range1, range2);
            break;
        }

        release_range(range1);
        release_range(range2);

        return r;
    }
    }
}

/**
 * Merges two ranges in a conjunctive way.
 *
 * The resulting range should be at least as 'strong' as both input ranges.
 * This is achived using the following rules ('_' = empty; all rules are commutative):
 *
 * (_  <: _ ) AND (_  <: _ )  =    _    <:    _
 * (E1 <: _ ) AND (_  <: E4)  =    E1   <:    E4
 * (_  <: _ ) AND (E3 <: E4)  =    E3   <:    E4
 * (E1 <: _ ) AND (E3 <: _ )  = E1 | E3 <:    _
 * (_  <: E2) AND (_  <: E4)  =    _    <: E2 & E4
 * (E1 <: E2) AND (E3 <: E4)  = E1 | E3 <: E2 & E4
 *
 * The equality checks are an optimization to avoid superfluous bounds.
 */
struct dy_constraint_range constraint_conjunction(struct dy_constraint_range range1, struct dy_constraint_range range2)
{
    struct dy_core_expr supertype;
    bool have_supertype = false;
    if (range1.have_supertype && range2.have_supertype && dy_are_equal(range1.supertype, range2.supertype) != DY_YES) {
        supertype = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_JUNCTION,
            .junction = {
                .e1 = dy_core_expr_new(dy_core_expr_retain(range1.supertype)),
                .e2 = dy_core_expr_new(dy_core_expr_retain(range2.supertype)),
                .polarity = DY_CORE_POLARITY_POSITIVE,
            }
        };

        have_supertype = true;
    } else if (range1.have_supertype) {
        supertype = dy_core_expr_retain(range1.supertype);
        have_supertype = true;
    } else if (range2.have_supertype) {
        supertype = dy_core_expr_retain(range2.supertype);
        have_supertype = true;
    }

    struct dy_core_expr subtype;
    bool have_subtype = false;
    if (range1.have_subtype && range2.have_subtype && dy_are_equal(range1.subtype, range2.subtype) != DY_YES) {
        subtype = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_JUNCTION,
            .junction = {
                .e1 = dy_core_expr_new(dy_core_expr_retain(range1.subtype)),
                .e2 = dy_core_expr_new(dy_core_expr_retain(range2.subtype)),
                .polarity = DY_CORE_POLARITY_NEGATIVE,
            }
        };

        have_subtype = true;
    } else if (range1.have_subtype) {
        subtype = dy_core_expr_retain(range1.subtype);
        have_subtype = true;
    } else if (range2.have_subtype) {
        subtype = dy_core_expr_retain(range2.subtype);
        have_subtype = true;
    }

    return (struct dy_constraint_range){
        .subtype = subtype,
        .have_subtype = have_subtype,
        .supertype = supertype,
        .have_supertype = have_supertype
    };
}

/**
 * Merges two ranges in a disjunctive way.
 *
 * The resulting range should be at most as 'strong' as both input ranges.
 * This is achived using the following rules ('_' = empty; all rules are commutative):
 *
 * (_  <: _ ) OR (_  <: _ ) =    _    <:    _
 * (E1 <: _ ) OR (_  <: E4) =    _    <:    _
 * (_  <: _ ) OR (E3 <: E4) =    _    <:    _
 * (E1 <: _ ) OR (E3 <: _ ) = E1 & E3 <:    _
 * (_  <: E2) OR (_  <: E4) =    _    <: E2 | E4
 * (E1 <: E2) OR (E3 <: E4) = E1 & E3 <: E2 | E4
 *
 * The equality checks are an optimization to avoid superfluous bounds.
 */
struct dy_constraint_range constraint_disjunction(struct dy_constraint_range range1, struct dy_constraint_range range2)
{
    struct dy_core_expr supertype;
    bool have_supertype = false;
    if (range1.have_supertype && range2.have_supertype) {
        if (dy_are_equal(range1.supertype, range2.supertype) == DY_YES) {
            supertype = dy_core_expr_retain(range1.supertype);
        } else {
            supertype = (struct dy_core_expr){
                .tag = DY_CORE_EXPR_JUNCTION,
                .junction = {
                    .e1 = dy_core_expr_new(dy_core_expr_retain(range1.supertype)),
                    .e2 = dy_core_expr_new(dy_core_expr_retain(range2.supertype)),
                    .polarity = DY_CORE_POLARITY_NEGATIVE,
                }
            };
        }

        have_supertype = true;
    }

    struct dy_core_expr subtype;
    bool have_subtype = false;
    if (range1.have_subtype && range2.have_subtype) {
        if (dy_are_equal(range1.subtype, range2.subtype) == DY_YES) {
            subtype = dy_core_expr_retain(range1.subtype);
        } else {
            subtype = (struct dy_core_expr){
                .tag = DY_CORE_EXPR_JUNCTION,
                .junction = {
                    .e1 = dy_core_expr_new(dy_core_expr_retain(range1.subtype)),
                    .e2 = dy_core_expr_new(dy_core_expr_retain(range2.subtype)),
                    .polarity = DY_CORE_POLARITY_POSITIVE,
                }
            };
        }

        have_subtype = true;
    }

    return (struct dy_constraint_range){
        .subtype = subtype,
        .have_subtype = have_subtype,
        .supertype = supertype,
        .have_supertype = have_supertype
    };
}

struct dy_constraint_range retain_range(struct dy_constraint_range range)
{
    if (range.have_subtype) {
        dy_core_expr_retain(range.subtype);
    }

    if (range.have_supertype) {
        dy_core_expr_retain(range.supertype);
    }

    return range;
}

void release_range(struct dy_constraint_range range)
{
    if (range.have_subtype) {
        dy_core_expr_release(range.subtype);
    }

    if (range.have_supertype) {
        dy_core_expr_release(range.supertype);
    }
}
