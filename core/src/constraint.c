/*
 * Copyright 2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <duality/core/constraint.h>
#include <duality/core/is_subtype.h>

#include <duality/support/assert.h>

static struct dy_core_expr *alloc_expr(struct dy_check_ctx ctx, struct dy_core_expr expr);
static struct dy_constraint *alloc_constraint(struct dy_check_ctx ctx, struct dy_constraint constraint);

bool dy_constraint_solve(struct dy_check_ctx ctx, struct dy_constraint constraint, size_t id, struct dy_core_expr *subtype, bool *have_subtype, struct dy_core_expr *supertype, bool *have_supertype, struct dy_constraint *new_constraint, bool *have_new_constraint)
{
    switch (constraint.tag) {
    case DY_CONSTRAINT_SINGLE:
        if (constraint.single.id != id) {
            *new_constraint = constraint;
            *have_new_constraint = true;
            return true;
        }

        switch (constraint.single.polarity_position) {
        case DY_CORE_POLARITY_POSITIVE:
            *supertype = constraint.single.expr;
            *have_supertype = true;
            return true;
        case DY_CORE_POLARITY_NEGATIVE:
            *subtype = constraint.single.expr;
            *have_subtype = true;
            return true;
        }

        DY_IMPOSSIBLE_ENUM();
    case DY_CONSTRAINT_MULTIPLE: {
        struct dy_core_expr subtype1;
        bool have_subtype1 = false;
        struct dy_core_expr supertype1;
        bool have_supertype1 = false;
        struct dy_constraint c1;
        bool have_c1 = false;
        bool result1 = dy_constraint_solve(ctx, *constraint.multiple.c1, id, &subtype1, &have_subtype1, &supertype1, &have_supertype1, &c1, &have_c1);

        struct dy_core_expr subtype2;
        bool have_subtype2 = false;
        struct dy_core_expr supertype2;
        bool have_supertype2 = false;
        struct dy_constraint c2;
        bool have_c2 = false;
        bool result2 = dy_constraint_solve(ctx, *constraint.multiple.c2, id, &subtype2, &have_subtype2, &supertype2, &have_supertype2, &c2, &have_c2);

        if (!result1 || !result2) {
            return false;
        }

        switch (constraint.multiple.polarity) {
        case DY_CORE_POLARITY_POSITIVE: {
            struct dy_constraint c3;
            bool have_c3 = false;
            if (have_subtype1 && have_supertype2) {
                if (dy_is_subtype_no_transformation(ctx, subtype1, supertype2, &c3, &have_c3) == DY_NO) {
                    return false;
                }
            }

            struct dy_constraint c4;
            bool have_c4 = false;
            if (have_subtype2 && have_supertype1) {
                if (dy_is_subtype_no_transformation(ctx, subtype2, supertype1, &c4, &have_c4) == DY_NO) {
                    return false;
                }
            }

            if (have_subtype1 && have_subtype2) {
                *subtype = (struct dy_core_expr){
                    .tag = DY_CORE_EXPR_BOTH,
                    .both = {
                        .e1 = alloc_expr(ctx, subtype1),
                        .e2 = alloc_expr(ctx, subtype2),
                        .polarity = DY_CORE_POLARITY_NEGATIVE,
                    }
                };
                *have_subtype = true;
            } else if (have_subtype1) {
                *subtype = subtype1;
                *have_subtype = true;
            } else if (have_subtype2) {
                *subtype = subtype2;
                *have_subtype = true;
            }

            if (have_supertype1 && have_supertype2) {
                *supertype = (struct dy_core_expr){
                    .tag = DY_CORE_EXPR_BOTH,
                    .both = {
                        .e1 = alloc_expr(ctx, supertype1),
                        .e2 = alloc_expr(ctx, supertype2),
                        .polarity = DY_CORE_POLARITY_POSITIVE,
                    }
                };
                *have_supertype = true;
            } else if (have_supertype1) {
                *supertype = supertype1;
                *have_supertype = true;
            } else if (have_supertype2) {
                *supertype = supertype2;
                *have_supertype = true;
            }

            if (have_c1 && have_c2 && have_c3 && have_c4) {
                struct dy_constraint cLeft = {
                    .tag = DY_CONSTRAINT_MULTIPLE,
                    .multiple = {
                        .c1 = alloc_constraint(ctx, c1),
                        .c2 = alloc_constraint(ctx, c2),
                        .polarity = DY_CORE_POLARITY_POSITIVE,
                    }
                };

                struct dy_constraint cRight = {
                    .tag = DY_CONSTRAINT_MULTIPLE,
                    .multiple = {
                        .c1 = alloc_constraint(ctx, c3),
                        .c2 = alloc_constraint(ctx, c4),
                        .polarity = DY_CORE_POLARITY_POSITIVE,
                    }
                };

                *new_constraint = (struct dy_constraint){
                    .tag = DY_CONSTRAINT_MULTIPLE,
                    .multiple = {
                        .c1 = alloc_constraint(ctx, cLeft),
                        .c2 = alloc_constraint(ctx, cRight),
                        .polarity = DY_CORE_POLARITY_POSITIVE,
                    }
                };

                *have_new_constraint = true;
            } else if (have_c1 && have_c2 && have_c3) {
                struct dy_constraint c = {
                    .tag = DY_CONSTRAINT_MULTIPLE,
                    .multiple = {
                        .c1 = alloc_constraint(ctx, c1),
                        .c2 = alloc_constraint(ctx, c2),
                        .polarity = DY_CORE_POLARITY_POSITIVE,
                    }
                };

                *new_constraint = (struct dy_constraint){
                    .tag = DY_CONSTRAINT_MULTIPLE,
                    .multiple = {
                        .c1 = alloc_constraint(ctx, c),
                        .c2 = alloc_constraint(ctx, c3),
                        .polarity = DY_CORE_POLARITY_POSITIVE,
                    }
                };

                *have_new_constraint = true;
            } else if (have_c1 && have_c2 && have_c4) {
                struct dy_constraint c = {
                    .tag = DY_CONSTRAINT_MULTIPLE,
                    .multiple = {
                        .c1 = alloc_constraint(ctx, c1),
                        .c2 = alloc_constraint(ctx, c2),
                        .polarity = DY_CORE_POLARITY_POSITIVE,
                    }
                };

                *new_constraint = (struct dy_constraint){
                    .tag = DY_CONSTRAINT_MULTIPLE,
                    .multiple = {
                        .c1 = alloc_constraint(ctx, c),
                        .c2 = alloc_constraint(ctx, c4),
                        .polarity = DY_CORE_POLARITY_POSITIVE,
                    }
                };

                *have_new_constraint = true;
            } else if (have_c1 && have_c3 && have_c4) {
                struct dy_constraint c = {
                    .tag = DY_CONSTRAINT_MULTIPLE,
                    .multiple = {
                        .c1 = alloc_constraint(ctx, c1),
                        .c2 = alloc_constraint(ctx, c3),
                        .polarity = DY_CORE_POLARITY_POSITIVE,
                    }
                };

                *new_constraint = (struct dy_constraint){
                    .tag = DY_CONSTRAINT_MULTIPLE,
                    .multiple = {
                        .c1 = alloc_constraint(ctx, c),
                        .c2 = alloc_constraint(ctx, c4),
                        .polarity = DY_CORE_POLARITY_POSITIVE,
                    }
                };

                *have_new_constraint = true;
            } else if (have_c2 && have_c3 && have_c4) {
                struct dy_constraint c = {
                    .tag = DY_CONSTRAINT_MULTIPLE,
                    .multiple = {
                        .c1 = alloc_constraint(ctx, c2),
                        .c2 = alloc_constraint(ctx, c3),
                        .polarity = DY_CORE_POLARITY_POSITIVE,
                    }
                };

                *new_constraint = (struct dy_constraint){
                    .tag = DY_CONSTRAINT_MULTIPLE,
                    .multiple = {
                        .c1 = alloc_constraint(ctx, c),
                        .c2 = alloc_constraint(ctx, c4),
                        .polarity = DY_CORE_POLARITY_POSITIVE,
                    }
                };

                *have_new_constraint = true;
            } else if (have_c1 && have_c2) {
                *new_constraint = (struct dy_constraint){
                    .tag = DY_CONSTRAINT_MULTIPLE,
                    .multiple = {
                        .c1 = alloc_constraint(ctx, c1),
                        .c2 = alloc_constraint(ctx, c2),
                        .polarity = DY_CORE_POLARITY_POSITIVE,
                    }
                };
                *have_new_constraint = true;
            } else if (have_c1 && have_c3) {
                *new_constraint = (struct dy_constraint){
                    .tag = DY_CONSTRAINT_MULTIPLE,
                    .multiple = {
                        .c1 = alloc_constraint(ctx, c1),
                        .c2 = alloc_constraint(ctx, c3),
                        .polarity = DY_CORE_POLARITY_POSITIVE,
                    }
                };
                *have_new_constraint = true;
            } else if (have_c1 && have_c4) {
                *new_constraint = (struct dy_constraint){
                    .tag = DY_CONSTRAINT_MULTIPLE,
                    .multiple = {
                        .c1 = alloc_constraint(ctx, c1),
                        .c2 = alloc_constraint(ctx, c4),
                        .polarity = DY_CORE_POLARITY_POSITIVE,
                    }
                };
                *have_new_constraint = true;
            } else if (have_c2 && have_c3) {
                *new_constraint = (struct dy_constraint){
                    .tag = DY_CONSTRAINT_MULTIPLE,
                    .multiple = {
                        .c1 = alloc_constraint(ctx, c2),
                        .c2 = alloc_constraint(ctx, c3),
                        .polarity = DY_CORE_POLARITY_POSITIVE,
                    }
                };
                *have_new_constraint = true;
            } else if (have_c2 && have_c4) {
                *new_constraint = (struct dy_constraint){
                    .tag = DY_CONSTRAINT_MULTIPLE,
                    .multiple = {
                        .c1 = alloc_constraint(ctx, c2),
                        .c2 = alloc_constraint(ctx, c4),
                        .polarity = DY_CORE_POLARITY_POSITIVE,
                    }
                };
                *have_new_constraint = true;
            } else if (have_c3 && have_c4) {
                *new_constraint = (struct dy_constraint){
                    .tag = DY_CONSTRAINT_MULTIPLE,
                    .multiple = {
                        .c1 = alloc_constraint(ctx, c3),
                        .c2 = alloc_constraint(ctx, c4),
                        .polarity = DY_CORE_POLARITY_POSITIVE,
                    }
                };
                *have_new_constraint = true;
            } else if (have_c1) {
                *new_constraint = c1;
                *have_new_constraint = true;
            } else if (have_c2) {
                *new_constraint = c2;
                *have_new_constraint = true;
            } else if (have_c3) {
                *new_constraint = c3;
                *have_new_constraint = true;
            } else if (have_c4) {
                *new_constraint = c4;
                *have_new_constraint = true;
            }

            return true;
        }
        case DY_CORE_POLARITY_NEGATIVE:
            if (have_subtype1 && have_subtype2) {
                *subtype = (struct dy_core_expr){
                    .tag = DY_CORE_EXPR_BOTH,
                    .both = {
                        .e1 = alloc_expr(ctx, subtype1),
                        .e2 = alloc_expr(ctx, subtype2),
                        .polarity = DY_CORE_POLARITY_POSITIVE,
                    }
                };
                *have_subtype = true;
            } else if (have_subtype1) {
                *subtype = subtype1;
                *have_subtype = true;
            } else if (have_subtype2) {
                *subtype = subtype2;
                *have_subtype = true;
            }

            if (have_supertype1 && have_supertype2) {
                *supertype = (struct dy_core_expr){
                    .tag = DY_CORE_EXPR_BOTH,
                    .both = {
                        .e1 = alloc_expr(ctx, supertype1),
                        .e2 = alloc_expr(ctx, supertype2),
                        .polarity = DY_CORE_POLARITY_NEGATIVE,
                    }
                };
                *have_supertype = true;
            } else if (have_supertype1) {
                *supertype = supertype1;
                *have_supertype = true;
            } else if (have_supertype2) {
                *subtype = supertype2;
                *have_supertype = true;
            }

            if (have_c1 && have_c2) {
                *new_constraint = (struct dy_constraint){
                    .tag = DY_CONSTRAINT_MULTIPLE,
                    .multiple = {
                        .c1 = alloc_constraint(ctx, c1),
                        .c2 = alloc_constraint(ctx, c2),
                        .polarity = DY_CORE_POLARITY_NEGATIVE,
                    }
                };
                *have_new_constraint = true;
            } else if (have_c1) {
                *new_constraint = c1;
                *have_new_constraint = true;
            } else if (have_c2) {
                *new_constraint = c2;
                *have_new_constraint = true;
            }

            return true;
        }

        DY_IMPOSSIBLE_ENUM();
    }
    }
}

struct dy_core_expr *alloc_expr(struct dy_check_ctx ctx, struct dy_core_expr expr)
{
    return dy_alloc(&expr, sizeof expr, ctx.allocator);
}

struct dy_constraint *alloc_constraint(struct dy_check_ctx ctx, struct dy_constraint constraint)
{
    return dy_alloc(&constraint, sizeof constraint, ctx.allocator);
}
