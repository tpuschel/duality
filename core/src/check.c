/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <duality/core/check.h>

#include <duality/core/is_subtype.h>
#include <duality/core/type_of.h>
#include <duality/core/constraint.h>

#include <duality/support/array.h>
#include <duality/support/assert.h>

#include "substitute.h"

#include <stdio.h>

static struct dy_core_expr *alloc_expr(struct dy_check_ctx ctx, struct dy_core_expr expr);
static struct dy_constraint *alloc_constraint(struct dy_check_ctx ctx, struct dy_constraint constraint);

bool dy_check_expr(struct dy_check_ctx ctx, struct dy_core_expr expr, struct dy_core_expr *new_expr, struct dy_constraint *constraint, bool *did_generate_constraint)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_VALUE_MAP: {
        if (!dy_check_value_map(ctx, expr.value_map, &expr.value_map, constraint, did_generate_constraint)) {
            return false;
        }

        *new_expr = expr;

        return true;
    }
    case DY_CORE_EXPR_TYPE_MAP:
        return dy_check_type_map(ctx, expr.type_map, new_expr, constraint, did_generate_constraint);
    case DY_CORE_EXPR_VALUE_MAP_ELIM: {
        if (!dy_check_value_map_elim(ctx, expr.value_map_elim, &expr.value_map_elim, constraint, did_generate_constraint)) {
            return false;
        }

        *new_expr = expr;

        return true;
    }
    case DY_CORE_EXPR_TYPE_MAP_ELIM: {
        if (!dy_check_type_map_elim(ctx, expr.type_map_elim, &expr.type_map_elim, constraint, did_generate_constraint)) {
            return false;
        }

        *new_expr = expr;

        return true;
    }
    case DY_CORE_EXPR_BOTH: {
        if (!dy_check_both(ctx, expr.both, &expr.both, constraint, did_generate_constraint)) {
            return false;
        }

        *new_expr = expr;

        return true;
    }
    case DY_CORE_EXPR_ONE_OF:
        return dy_check_one_of(ctx, expr.one_of, new_expr, constraint, did_generate_constraint);
    case DY_CORE_EXPR_UNKNOWN:
        *new_expr = expr;
        return true;
    case DY_CORE_EXPR_END:
        *new_expr = expr;
        return true;
    case DY_CORE_EXPR_STRING:
        *new_expr = expr;
        return true;
    case DY_CORE_EXPR_TYPE_OF_STRINGS:
        *new_expr = expr;
        return true;
    case DY_CORE_EXPR_PRINT:
        *new_expr = expr;
        return true;
    }

    DY_IMPOSSIBLE_ENUM();
}

bool dy_check_value_map(struct dy_check_ctx ctx, struct dy_core_value_map value_map, struct dy_core_value_map *new_value_map, struct dy_constraint *constraint, bool *did_generate_constraint)
{
    struct dy_core_expr e1;
    struct dy_constraint c1;
    bool have_c1 = false;
    bool first_succeeded = dy_check_expr(ctx, *value_map.e1, &e1, &c1, &have_c1);
    value_map.e1 = alloc_expr(ctx, e1);

    struct dy_core_expr e2;
    struct dy_constraint c2;
    bool have_c2 = false;
    bool second_succeeded = dy_check_expr(ctx, *value_map.e2, &e2, &c2, &have_c2);
    value_map.e2 = alloc_expr(ctx, e2);

    if (!first_succeeded || !second_succeeded) {
        return false;
    }

    if (have_c1 && have_c2) {
        *constraint = (struct dy_constraint){
            .tag = DY_CONSTRAINT_MULTIPLE,
            .multiple = {
                .c1 = alloc_constraint(ctx, c1),
                .c2 = alloc_constraint(ctx, c2),
                .polarity = DY_CORE_POLARITY_POSITIVE,
            }
        };
        *did_generate_constraint = true;
    } else if (have_c1) {
        *constraint = c1;
        *did_generate_constraint = true;
    } else if (have_c2) {
        *constraint = c2;
        *did_generate_constraint = true;
    }

    *new_value_map = value_map;

    return true;
}

bool dy_check_type_map(struct dy_check_ctx ctx, struct dy_core_type_map type_map, struct dy_core_expr *new_expr, struct dy_constraint *constraint, bool *did_generate_constraint)
{
    struct dy_core_expr type;
    struct dy_constraint c1;
    bool have_c1 = false;
    bool arg_succeeded = dy_check_expr(ctx, *type_map.arg_type, &type, &c1, &have_c1);

    struct dy_core_expr expr;
    if (arg_succeeded) {
        struct dy_core_expr new_unknown = {
            .tag = DY_CORE_EXPR_UNKNOWN,
            .unknown = {
                .id = type_map.arg_id,
                .type = alloc_expr(ctx, type),
                .is_inference_var = type_map.is_implicit,
            },
        };

        expr = substitute(ctx, type_map.arg_id, new_unknown, *type_map.expr);
    } else {
        if (type_map.is_implicit) {
            struct dy_core_expr new_unknown = {
                .tag = DY_CORE_EXPR_UNKNOWN,
                .unknown = {
                    .id = type_map.arg_id,
                    .type = type_map.arg_type,
                    .is_inference_var = true,
                },
            };

            expr = substitute(ctx, type_map.arg_id, new_unknown, *type_map.expr);
        } else {
            expr = *type_map.expr;
        }
    }

    struct dy_constraint c2;
    bool have_c2 = false;
    bool expr_succeeded = dy_check_expr(ctx, expr, &expr, &c2, &have_c2);

    if (!arg_succeeded || !expr_succeeded) {
        return false;
    }

    if (type_map.is_implicit && have_c2) {
        struct dy_core_expr subtype;
        bool have_subtype = false;
        struct dy_core_expr supertype;
        bool have_supertype = false;
        have_c2 = false;
        if (!dy_constraint_solve(ctx, c2, type_map.arg_id, &subtype, &have_subtype, &supertype, &have_supertype, &c2, &have_c2)) {
            return false;
        }

        if (have_c1 && have_c2) {
            *constraint = (struct dy_constraint){
                .tag = DY_CONSTRAINT_MULTIPLE,
                .multiple = {
                    .c1 = alloc_constraint(ctx, c1),
                    .c2 = alloc_constraint(ctx, c2),
                    .polarity = DY_CORE_POLARITY_POSITIVE,
                }
            };
            *did_generate_constraint = true;
        } else if (have_c1) {
            *constraint = c1;
            *did_generate_constraint = true;
        } else if (have_c2) {
            *constraint = c2;
            *did_generate_constraint = true;
        }

        switch (type_map.polarity) {
        case DY_CORE_POLARITY_POSITIVE:
            if (have_supertype) {
                *new_expr = substitute(ctx, type_map.arg_id, supertype, expr);
                return true;
            }

            fprintf(stderr, "Parametric implicit arg! Not allowed yet..\n");

            return false;
        case DY_CORE_POLARITY_NEGATIVE:
            if (have_subtype) {
                *new_expr = substitute(ctx, type_map.arg_id, subtype, expr);
                return true;
            }

            fprintf(stderr, "Parametric implicit arg! Not allowed yet..\n");

            return false;
        }

        DY_IMPOSSIBLE_ENUM();
    } else {
        if (have_c1 && have_c2) {
            *constraint = (struct dy_constraint){
                .tag = DY_CONSTRAINT_MULTIPLE,
                .multiple = {
                    .c1 = alloc_constraint(ctx, c1),
                    .c2 = alloc_constraint(ctx, c2),
                    .polarity = DY_CORE_POLARITY_POSITIVE,
                }
            };
            *did_generate_constraint = true;
        } else if (have_c1) {
            *constraint = c1;
            *did_generate_constraint = true;
        } else if (have_c2) {
            *constraint = c2;
            *did_generate_constraint = true;
        }

        if (type_map.is_implicit) {
            struct dy_core_expr new_unknown = {
                .tag = DY_CORE_EXPR_UNKNOWN,
                .unknown = {
                    .id = type_map.arg_id,
                    .type = alloc_expr(ctx, type),
                    .is_inference_var = false,
                },
            };

            expr = substitute(ctx, type_map.arg_id, new_unknown, expr);
        }

        *new_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_TYPE_MAP,
            .type_map = {
                .arg_id = type_map.arg_id,
                .arg_type = alloc_expr(ctx, type),
                .expr = alloc_expr(ctx, expr),
                .polarity = type_map.polarity,
                .is_implicit = type_map.is_implicit,
            }
        };

        return true;
    }
}

bool dy_check_value_map_elim(struct dy_check_ctx ctx, struct dy_core_value_map_elim elim, struct dy_core_value_map_elim *new_elim, struct dy_constraint *constraint, bool *did_generate_constraint)
{
    struct dy_core_expr expr;
    struct dy_constraint c1;
    bool have_c1 = false;
    bool expr_succeeded = dy_check_expr(ctx, *elim.expr, &expr, &c1, &have_c1);

    struct dy_core_value_map value_map;
    struct dy_constraint c2;
    bool have_c2 = false;
    bool value_map_succeeded = dy_check_value_map(ctx, elim.value_map, &value_map, &c2, &have_c2);

    struct dy_core_expr value_map_expr = {
        .tag = DY_CORE_EXPR_VALUE_MAP,
        .value_map = value_map
    };

    bool check_elim = true;
    for (size_t i = 0, size = dy_array_size(ctx.successful_elims); i < size; ++i) {
        size_t id;
        dy_array_get(ctx.successful_elims, i, &id);

        if (id == elim.id) {
            check_elim = false;
            break;
        }
    }

    dy_ternary_t subtype_res;
    struct dy_constraint c3;
    bool have_c3 = false;
    if (check_elim) {
        if (!expr_succeeded || !value_map_succeeded) {
            return false;
        }

        subtype_res = dy_is_subtype(ctx, dy_type_of(ctx, expr), value_map_expr, &c3, &have_c3, expr, &expr);
    } else {
        subtype_res = DY_YES;
    }

    if (!expr_succeeded || !value_map_succeeded || subtype_res == DY_NO) {
        return false;
    }

    if (check_elim && subtype_res == DY_YES) {
        dy_array_add(ctx.successful_elims, &elim.id);
    }

    if (have_c1 && have_c2 && have_c3) {
        struct dy_constraint c = {
            .tag = DY_CONSTRAINT_MULTIPLE,
            .multiple = {
                .c1 = alloc_constraint(ctx, c1),
                .c2 = alloc_constraint(ctx, c2),
                .polarity = DY_CORE_POLARITY_POSITIVE,
            }
        };

        *constraint = (struct dy_constraint){
            .tag = DY_CONSTRAINT_MULTIPLE,
            .multiple = {
                .c1 = alloc_constraint(ctx, c),
                .c2 = alloc_constraint(ctx, c3),
                .polarity = DY_CORE_POLARITY_POSITIVE,
            }
        };

        *did_generate_constraint = true;
    } else if (have_c1 && have_c2) {
        *constraint = (struct dy_constraint){
            .tag = DY_CONSTRAINT_MULTIPLE,
            .multiple = {
                .c1 = alloc_constraint(ctx, c1),
                .c2 = alloc_constraint(ctx, c2),
                .polarity = DY_CORE_POLARITY_POSITIVE,
            }
        };
        *did_generate_constraint = true;
    } else if (have_c2 && have_c3) {
        *constraint = (struct dy_constraint){
            .tag = DY_CONSTRAINT_MULTIPLE,
            .multiple = {
                .c1 = alloc_constraint(ctx, c2),
                .c2 = alloc_constraint(ctx, c3),
                .polarity = DY_CORE_POLARITY_POSITIVE,
            }
        };
        *did_generate_constraint = true;
    } else if (have_c1 && have_c3) {
        *constraint = (struct dy_constraint){
            .tag = DY_CONSTRAINT_MULTIPLE,
            .multiple = {
                .c1 = alloc_constraint(ctx, c1),
                .c2 = alloc_constraint(ctx, c3),
                .polarity = DY_CORE_POLARITY_POSITIVE,
            }
        };
        *did_generate_constraint = true;
    } else if (have_c1) {
        *constraint = c1;
        *did_generate_constraint = true;
    } else if (have_c2) {
        *constraint = c2;
        *did_generate_constraint = true;
    } else if (have_c3) {
        *constraint = c3;
        *did_generate_constraint = true;
    }

    *new_elim = (struct dy_core_value_map_elim){
        .id = elim.id,
        .expr = alloc_expr(ctx, expr),
        .value_map = value_map
    };

    return true;
}

bool dy_check_type_map_elim(struct dy_check_ctx ctx, struct dy_core_type_map_elim elim, struct dy_core_type_map_elim *new_elim, struct dy_constraint *constraint, bool *did_generate_constraint)
{
    struct dy_core_expr expr;
    struct dy_constraint c1;
    bool have_c1 = false;
    bool expr_succeeded = dy_check_expr(ctx, *elim.expr, &expr, &c1, &have_c1);

    struct dy_core_expr type_map;
    struct dy_constraint c2;
    bool have_c2 = false;
    bool type_map_succeeded = dy_check_type_map(ctx, elim.type_map, &type_map, &c2, &have_c2);

    dy_assert(type_map.tag == DY_CORE_EXPR_TYPE_MAP);

    struct dy_core_expr type_map_expr = {
        .tag = DY_CORE_EXPR_TYPE_MAP,
        .type_map = type_map.type_map
    };

    bool check_elim = true;
    for (size_t i = 0, size = dy_array_size(ctx.successful_elims); i < size; ++i) {
        size_t id;
        dy_array_get(ctx.successful_elims, i, &id);

        if (id == elim.id) {
            check_elim = false;
            break;
        }
    }

    dy_ternary_t subtype_res;
    struct dy_constraint c3;
    bool have_c3 = false;
    if (check_elim) {
        subtype_res = dy_is_subtype(ctx, dy_type_of(ctx, expr), type_map_expr, &c3, &have_c3, expr, &expr);
    } else {
        subtype_res = DY_YES;
    }

    if (!expr_succeeded || !type_map_succeeded || subtype_res == DY_NO) {
        return false;
    }

    if (check_elim && subtype_res == DY_YES) {
        dy_array_add(ctx.successful_elims, &elim.id);
    }

    if (have_c1 && have_c2 && have_c3) {
        struct dy_constraint c = {
            .tag = DY_CONSTRAINT_MULTIPLE,
            .multiple = {
                .c1 = alloc_constraint(ctx, c1),
                .c2 = alloc_constraint(ctx, c2),
                .polarity = DY_CORE_POLARITY_POSITIVE,
            }
        };

        *constraint = (struct dy_constraint){
            .tag = DY_CONSTRAINT_MULTIPLE,
            .multiple = {
                .c1 = alloc_constraint(ctx, c),
                .c2 = alloc_constraint(ctx, c3),
                .polarity = DY_CORE_POLARITY_POSITIVE,
            }
        };

        *did_generate_constraint = true;
    } else if (have_c1 && have_c2) {
        *constraint = (struct dy_constraint){
            .tag = DY_CONSTRAINT_MULTIPLE,
            .multiple = {
                .c1 = alloc_constraint(ctx, c1),
                .c2 = alloc_constraint(ctx, c2),
                .polarity = DY_CORE_POLARITY_POSITIVE,
            }
        };
        *did_generate_constraint = true;
    } else if (have_c2 && have_c3) {
        *constraint = (struct dy_constraint){
            .tag = DY_CONSTRAINT_MULTIPLE,
            .multiple = {
                .c1 = alloc_constraint(ctx, c2),
                .c2 = alloc_constraint(ctx, c3),
                .polarity = DY_CORE_POLARITY_POSITIVE,
            }
        };
        *did_generate_constraint = true;
    } else if (have_c1 && have_c3) {
        *constraint = (struct dy_constraint){
            .tag = DY_CONSTRAINT_MULTIPLE,
            .multiple = {
                .c1 = alloc_constraint(ctx, c1),
                .c2 = alloc_constraint(ctx, c3),
                .polarity = DY_CORE_POLARITY_POSITIVE,
            }
        };
        *did_generate_constraint = true;
    } else if (have_c1) {
        *constraint = c1;
        *did_generate_constraint = true;
    } else if (have_c2) {
        *constraint = c2;
        *did_generate_constraint = true;
    } else if (have_c3) {
        *constraint = c3;
        *did_generate_constraint = true;
    }

    *new_elim = (struct dy_core_type_map_elim){
        .id = elim.id,
        .expr = alloc_expr(ctx, expr),
        .type_map = type_map.type_map
    };

    return true;
}

bool dy_check_both(struct dy_check_ctx ctx, struct dy_core_both both, struct dy_core_both *new_both, struct dy_constraint *constraint, bool *did_generate_constraint)
{
    struct dy_core_expr e1;
    struct dy_constraint c1;
    bool have_c1 = false;
    bool first_succeeded = dy_check_expr(ctx, *both.e1, &e1, &c1, &have_c1);
    both.e1 = alloc_expr(ctx, e1);

    struct dy_core_expr e2;
    struct dy_constraint c2;
    bool have_c2 = false;
    bool second_succeeded = dy_check_expr(ctx, *both.e2, &e2, &c2, &have_c2);
    both.e2 = alloc_expr(ctx, e2);

    if (!first_succeeded || !second_succeeded) {
        return false;
    }

    if (have_c1 && have_c2) {
        *constraint = (struct dy_constraint){
            .tag = DY_CONSTRAINT_MULTIPLE,
            .multiple = {
                .c1 = alloc_constraint(ctx, c1),
                .c2 = alloc_constraint(ctx, c2),
                .polarity = DY_CORE_POLARITY_POSITIVE,
            }
        };
        *did_generate_constraint = true;
    } else if (have_c1) {
        *constraint = c1;
        *did_generate_constraint = true;
    } else if (have_c2) {
        *constraint = c2;
        *did_generate_constraint = true;
    }

    *new_both = both;

    return true;
}

bool dy_check_one_of(struct dy_check_ctx ctx, struct dy_core_one_of one_of, struct dy_core_expr *new_expr, struct dy_constraint *constraint, bool *did_generate_constraint)
{
    struct dy_core_expr first;
    struct dy_constraint c1;
    bool have_c1 = false;
    bool first_succeeded = dy_check_expr(ctx, *one_of.first, &first, &c1, &have_c1);

    struct dy_core_expr second;
    struct dy_constraint c2;
    bool have_c2 = false;
    bool second_succeeded = dy_check_expr(ctx, *one_of.second, &second, &c2, &have_c2);

    if (!first_succeeded && !second_succeeded) {
        return false;
    }

    if (first_succeeded && second_succeeded) {
        if (have_c1 && have_c2) {
            *constraint = (struct dy_constraint){
                .tag = DY_CONSTRAINT_MULTIPLE,
                .multiple = {
                    .c1 = alloc_constraint(ctx, c1),
                    .c2 = alloc_constraint(ctx, c2),
                    .polarity = DY_CORE_POLARITY_NEGATIVE,
                }
            };
            *did_generate_constraint = true;
        } else if (have_c1) {
            *constraint = c1;
            *did_generate_constraint = true;
        } else if (have_c2) {
            *constraint = c2;
            *did_generate_constraint = true;
        }

        *new_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_ONE_OF,
            .one_of = {
                .first = alloc_expr(ctx, first),
                .second = alloc_expr(ctx, second),
            }
        };

        return true;
    }

    if (first_succeeded) {
        *constraint = c1;
        *did_generate_constraint = have_c1;
        *new_expr = first;
        return true;
    }

    dy_assert(second_succeeded);

    *constraint = c2;
    *did_generate_constraint = have_c2;

    *new_expr = second;

    return true;
}

struct dy_core_expr *alloc_expr(struct dy_check_ctx ctx, struct dy_core_expr expr)
{
    return dy_alloc(&expr, sizeof expr, ctx.allocator);
}

struct dy_constraint *alloc_constraint(struct dy_check_ctx ctx, struct dy_constraint constraint)
{
    return dy_alloc(&constraint, sizeof constraint, ctx.allocator);
}
