/*
 * Copyright 2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <duality/core/constraint.h>

#include <duality/core/is_subtype.h>

#include <duality/support/assert.h>

static struct dy_constraint_range retain_range(struct dy_core_ctx ctx, struct dy_constraint_range range);
static void release_range(struct dy_core_ctx ctx, struct dy_constraint_range range);

static struct dy_constraint_range constraint_conjunction(struct dy_core_ctx ctx, struct dy_constraint_range range1, struct dy_constraint_range range2);
static struct dy_constraint_range constraint_disjunction(struct dy_core_ctx ctx, struct dy_constraint_range range1, struct dy_constraint_range range2);

struct dy_constraint_range dy_constraint_collect(struct dy_core_ctx ctx, struct dy_constraint constraint, size_t id)
{
    switch (constraint.tag) {
    case DY_CONSTRAINT_SINGLE:
        if (constraint.single.id != id) {
            return (struct dy_constraint_range){
                .have_subtype = false,
                .have_supertype = false
            };
        }

        return retain_range(ctx, constraint.single.range);
    case DY_CONSTRAINT_MULTIPLE: {
        struct dy_constraint_range range1 = dy_constraint_collect(ctx, *constraint.multiple.c1, id);
        struct dy_constraint_range range2 = dy_constraint_collect(ctx, *constraint.multiple.c2, id);

        struct dy_constraint_range r;

        switch (constraint.multiple.polarity) {
        case DY_CORE_POLARITY_POSITIVE:
            r = constraint_conjunction(ctx, range1, range2);
            break;
        case DY_CORE_POLARITY_NEGATIVE:
            r = constraint_disjunction(ctx, range1, range2);
            break;
        }

        release_range(ctx, range1);
        release_range(ctx, range2);

        return r;
    }
    }
}

struct dy_constraint_range constraint_conjunction(struct dy_core_ctx ctx, struct dy_constraint_range range1, struct dy_constraint_range range2)
{
    struct dy_core_expr supertype;
    bool have_supertype = false;
    if (range1.have_supertype && range2.have_supertype) {
        supertype = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_BOTH,
            .both = {
                .e1 = dy_core_expr_new(ctx.expr_pool, dy_core_expr_retain(ctx.expr_pool, range1.supertype)),
                .e2 = dy_core_expr_new(ctx.expr_pool, dy_core_expr_retain(ctx.expr_pool, range2.supertype)),
                .polarity = DY_CORE_POLARITY_POSITIVE,
            }
        };

        have_supertype = true;
    } else if (range1.have_supertype) {
        supertype = dy_core_expr_retain(ctx.expr_pool, range1.supertype);
        have_supertype = true;
    } else if (range2.have_supertype) {
        supertype = dy_core_expr_retain(ctx.expr_pool, range2.supertype);
        have_supertype = true;
    }

    struct dy_core_expr subtype;
    bool have_subtype = false;
    if (range1.have_subtype && range2.have_subtype) {
        subtype = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_BOTH,
            .both = {
                .e1 = dy_core_expr_new(ctx.expr_pool, dy_core_expr_retain(ctx.expr_pool, range1.subtype)),
                .e2 = dy_core_expr_new(ctx.expr_pool, dy_core_expr_retain(ctx.expr_pool, range2.subtype)),
                .polarity = DY_CORE_POLARITY_NEGATIVE,
            }
        };

        have_subtype = true;
    } else if (range1.have_subtype) {
        subtype = dy_core_expr_retain(ctx.expr_pool, range1.subtype);
        have_subtype = true;
    } else if (range2.have_subtype) {
        subtype = dy_core_expr_retain(ctx.expr_pool, range2.subtype);
        have_subtype = true;
    }

    return (struct dy_constraint_range){
        .subtype = subtype,
        .have_subtype = have_subtype,
        .supertype = supertype,
        .have_supertype = have_supertype
    };
}

struct dy_constraint_range constraint_disjunction(struct dy_core_ctx ctx, struct dy_constraint_range range1, struct dy_constraint_range range2)
{
    struct dy_core_expr supertype;
    bool have_supertype = false;
    if (range1.have_supertype && range2.have_supertype) {
        supertype = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_BOTH,
            .both = {
                .e1 = dy_core_expr_new(ctx.expr_pool, dy_core_expr_retain(ctx.expr_pool, range1.supertype)),
                .e2 = dy_core_expr_new(ctx.expr_pool, dy_core_expr_retain(ctx.expr_pool, range2.supertype)),
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
                .e1 = dy_core_expr_new(ctx.expr_pool, dy_core_expr_retain(ctx.expr_pool, range1.subtype)),
                .e2 = dy_core_expr_new(ctx.expr_pool, dy_core_expr_retain(ctx.expr_pool, range2.subtype)),
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

struct dy_constraint_range retain_range(struct dy_core_ctx ctx, struct dy_constraint_range range)
{
    if (range.have_subtype) {
        dy_core_expr_retain(ctx.expr_pool, range.subtype);
    }

    if (range.have_supertype) {
        dy_core_expr_retain(ctx.expr_pool, range.supertype);
    }

    return range;
}

void release_range(struct dy_core_ctx ctx, struct dy_constraint_range range)
{
    if (range.have_subtype) {
        dy_core_expr_release(ctx.expr_pool, range.subtype);
    }

    if (range.have_supertype) {
        dy_core_expr_release(ctx.expr_pool, range.supertype);
    }
}
