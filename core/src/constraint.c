/*
 * Copyright 2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <duality/core/constraint.h>
#include <duality/core/is_subtype.h>

#include <duality/support/assert.h>

static struct dy_core_expr *alloc_expr(struct dy_check_ctx ctx, struct dy_core_expr expr);

static struct dy_constraint_range constraint_conjunction(struct dy_check_ctx ctx, struct dy_constraint_range range1, struct dy_constraint_range range2);
static struct dy_constraint_range constraint_disjunction(struct dy_check_ctx ctx, struct dy_constraint_range range1, struct dy_constraint_range range2);

struct dy_constraint_range dy_constraint_collect(struct dy_check_ctx ctx, struct dy_constraint constraint, size_t id)
{
    switch (constraint.tag) {
    case DY_CONSTRAINT_SINGLE:
        if (constraint.single.id != id) {
            return (struct dy_constraint_range){
                .have_subtype = false,
                .have_supertype = false
            };
        }

        return constraint.single.range;
    case DY_CONSTRAINT_MULTIPLE: {
        struct dy_constraint_range range1 = dy_constraint_collect(ctx, *constraint.multiple.c1, id);

        struct dy_constraint_range range2 = dy_constraint_collect(ctx, *constraint.multiple.c2, id);

        switch (constraint.multiple.polarity) {
        case DY_CORE_POLARITY_POSITIVE:
            return constraint_conjunction(ctx, range1, range2);
        case DY_CORE_POLARITY_NEGATIVE:
            return constraint_disjunction(ctx, range1, range2);
        }

        DY_IMPOSSIBLE_ENUM();
    }
    }
}

struct dy_constraint_range constraint_conjunction(struct dy_check_ctx ctx, struct dy_constraint_range range1, struct dy_constraint_range range2)
{
    struct dy_core_expr supertype;
    bool have_supertype = false;
    if (range1.have_supertype && range2.have_supertype) {
        supertype = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_BOTH,
            .both = {
                .e1 = alloc_expr(ctx, range1.supertype),
                .e2 = alloc_expr(ctx, range2.supertype),
                .polarity = DY_CORE_POLARITY_POSITIVE,
            }
        };

        have_supertype = true;
    } else if (range1.have_supertype) {
        supertype = range1.supertype;
        have_supertype = true;
    } else if (range2.have_supertype) {
        supertype = range2.supertype;
        have_supertype = true;
    }

    struct dy_core_expr subtype;
    bool have_subtype = false;
    if (range1.have_subtype && range2.have_subtype) {
        subtype = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_BOTH,
            .both = {
                .e1 = alloc_expr(ctx, range1.subtype),
                .e2 = alloc_expr(ctx, range2.subtype),
                .polarity = DY_CORE_POLARITY_NEGATIVE,
            }
        };

        have_subtype = true;
    } else if (range1.have_subtype) {
        subtype = range1.subtype;
        have_subtype = true;
    } else if (range2.have_subtype) {
        subtype = range2.subtype;
        have_subtype = true;
    }

    return (struct dy_constraint_range){
        .subtype = subtype,
        .have_subtype = have_subtype,
        .supertype = supertype,
        .have_supertype = have_supertype
    };
}

struct dy_constraint_range constraint_disjunction(struct dy_check_ctx ctx, struct dy_constraint_range range1, struct dy_constraint_range range2)
{
    struct dy_core_expr supertype;
    bool have_supertype = false;
    if (range1.have_supertype && range2.have_supertype) {
        supertype = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_BOTH,
            .both = {
                .e1 = alloc_expr(ctx, range1.supertype),
                .e2 = alloc_expr(ctx, range2.supertype),
                .polarity = DY_CORE_POLARITY_NEGATIVE,
            }
        };

        have_supertype = true;
    }

    struct dy_core_expr subtype;
    bool have_subtype = false;
    if (range1.have_subtype && range2.have_subtype) {
        subtype = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_BOTH,
            .both = {
                .e1 = alloc_expr(ctx, range1.subtype),
                .e2 = alloc_expr(ctx, range2.subtype),
                .polarity = DY_CORE_POLARITY_POSITIVE,
            }
        };

        have_subtype = true;
    }

    return (struct dy_constraint_range){
        .subtype = subtype,
        .have_subtype = have_subtype,
        .supertype = supertype,
        .have_supertype = have_supertype
    };
}

struct dy_core_expr *alloc_expr(struct dy_check_ctx ctx, struct dy_core_expr expr)
{
    return dy_alloc(&expr, sizeof expr, ctx.allocator);
}
