/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <duality/core/check.h>

#include <duality/core/is_subtype.h>
#include <duality/core/type_of.h>

#include <duality/support/array.h>
#include <duality/support/assert.h>
#include <duality/support/allocator.h>

#include "substitute.h"

#include <stdio.h>

static struct dy_core_expr *alloc_expr(struct dy_core_expr expr);
static struct dy_constraint *alloc_constraint(struct dy_constraint constraint);

static bool is_bound(size_t id, struct dy_core_expr expr);

static bool resolve_implicit(struct dy_check_ctx ctx, size_t id, struct dy_core_expr type, enum dy_core_polarity polarity, struct dy_constraint constraint, bool have_constraint, struct dy_constraint *new_constraint, bool *have_new_constraint, struct dy_core_expr expr, struct dy_core_expr *new_expr);
static void remove_id(dy_array_t *ids, size_t id);

static void print_core_expr(struct dy_core_expr expr);
static void print_constraint(struct dy_constraint c);

static bool is_mentioned_in_constraints(struct dy_check_ctx ctx, size_t id, struct dy_constraint constraint);

bool dy_check_expr(struct dy_check_ctx ctx, struct dy_core_expr expr, struct dy_core_expr *new_expr, struct dy_constraint *constraint, bool *did_generate_constraint)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_EXPR_MAP:
        if (!dy_check_expr_map(ctx, expr.expr_map, &expr.expr_map, constraint, did_generate_constraint)) {
            return false;
        }

        *new_expr = expr;

        return true;
    case DY_CORE_EXPR_TYPE_MAP:
        if (!dy_check_type_map(ctx, expr.type_map, &expr.type_map, constraint, did_generate_constraint)) {
            return false;
        }

        *new_expr = expr;

        return true;
    case DY_CORE_EXPR_EXPR_MAP_ELIM:
        if (!dy_check_expr_map_elim(ctx, expr.expr_map_elim, &expr.expr_map_elim, constraint, did_generate_constraint)) {
            return false;
        }

        *new_expr = expr;

        return true;
    case DY_CORE_EXPR_TYPE_MAP_ELIM:
        if (!dy_check_type_map_elim(ctx, expr.type_map_elim, &expr.type_map_elim, constraint, did_generate_constraint)) {
            return false;
        }

        *new_expr = expr;

        return true;
    case DY_CORE_EXPR_BOTH:
        if (!dy_check_both(ctx, expr.both, &expr.both, constraint, did_generate_constraint)) {
            return false;
        }

        *new_expr = expr;

        return true;
    case DY_CORE_EXPR_ONE_OF:
        return dy_check_one_of(ctx, expr.one_of, new_expr, constraint, did_generate_constraint);
    case DY_CORE_EXPR_INFERENCE_CTX:
        return dy_check_inference_ctx(ctx, expr.inference_ctx, new_expr, constraint, did_generate_constraint);
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

bool dy_check_expr_map(struct dy_check_ctx ctx, struct dy_core_expr_map expr_map, struct dy_core_expr_map *new_expr_map, struct dy_constraint *constraint, bool *did_generate_constraint)
{
    struct dy_core_expr e1;
    struct dy_constraint c1;
    bool have_c1 = false;
    bool first_succeeded = dy_check_expr(ctx, *expr_map.e1, &e1, &c1, &have_c1);
    expr_map.e1 = alloc_expr(e1);

    struct dy_core_expr e2;
    struct dy_constraint c2;
    bool have_c2 = false;
    bool second_succeeded = dy_check_expr(ctx, *expr_map.e2, &e2, &c2, &have_c2);
    expr_map.e2 = alloc_expr(e2);

    if (!first_succeeded || !second_succeeded) {
        return false;
    }

    if (have_c1 && have_c2) {
        *constraint = (struct dy_constraint){
            .tag = DY_CONSTRAINT_MULTIPLE,
            .multiple = {
                .c1 = alloc_constraint(c1),
                .c2 = alloc_constraint(c2),
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

    *new_expr_map = expr_map;

    return true;
}

bool dy_check_type_map(struct dy_check_ctx ctx, struct dy_core_type_map type_map, struct dy_core_type_map *new_type_map, struct dy_constraint *constraint, bool *did_generate_constraint)
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
                .type = alloc_expr(type),
                .is_inference_var = false,
            },
        };

        expr = substitute(type_map.arg_id, new_unknown, *type_map.expr);
    } else {
        expr = *type_map.expr;
    }

    struct dy_constraint c2;
    bool have_c2 = false;
    bool expr_succeeded = dy_check_expr(ctx, expr, &expr, &c2, &have_c2);

    if (!arg_succeeded || !expr_succeeded) {
        return false;
    }

    if (have_c2) {
        // Check to see if we're bound in one of the constraints.
        // If that's the case, error out for now.
        if (is_mentioned_in_constraints(ctx, type_map.arg_id, c2)) {
            return false;
        }
    }

    if (have_c1 && have_c2) {
        *constraint = (struct dy_constraint){
            .tag = DY_CONSTRAINT_MULTIPLE,
            .multiple = {
                .c1 = alloc_constraint(c1),
                .c2 = alloc_constraint(c2),
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

    *new_type_map = (struct dy_core_type_map){
        .arg_id = type_map.arg_id,
        .arg_type = alloc_expr(type),
        .expr = alloc_expr(expr),
        .polarity = type_map.polarity,
        .is_implicit = type_map.is_implicit
    };

    return true;
}

bool dy_check_expr_map_elim(struct dy_check_ctx ctx, struct dy_core_expr_map_elim elim, struct dy_core_expr_map_elim *new_elim, struct dy_constraint *constraint, bool *did_generate_constraint)
{
    struct dy_core_expr expr;
    struct dy_constraint c1;
    bool have_c1 = false;
    bool expr_succeeded = dy_check_expr(ctx, *elim.expr, &expr, &c1, &have_c1);

    struct dy_core_expr_map expr_map;
    struct dy_constraint c2;
    bool have_c2 = false;
    bool expr_map_succeeded = dy_check_expr_map(ctx, elim.expr_map, &expr_map, &c2, &have_c2);

    if (!expr_succeeded || !expr_map_succeeded) {
        return false;
    }

    struct dy_core_expr expr_map_expr = {
        .tag = DY_CORE_EXPR_EXPR_MAP,
        .expr_map = expr_map
    };

    struct dy_constraint c3;
    bool have_c3 = false;
    dy_ternary_t subtype_res = dy_is_subtype(ctx, dy_type_of(ctx, expr), expr_map_expr, &c3, &have_c3, expr, &expr);

    if (!expr_succeeded || !expr_map_succeeded || subtype_res == DY_NO) {
        return false;
    }

    if (have_c1 && have_c2 && have_c3) {
        struct dy_constraint c = {
            .tag = DY_CONSTRAINT_MULTIPLE,
            .multiple = {
                .c1 = alloc_constraint(c1),
                .c2 = alloc_constraint(c2),
                .polarity = DY_CORE_POLARITY_POSITIVE,
            }
        };

        *constraint = (struct dy_constraint){
            .tag = DY_CONSTRAINT_MULTIPLE,
            .multiple = {
                .c1 = alloc_constraint(c),
                .c2 = alloc_constraint(c3),
                .polarity = DY_CORE_POLARITY_POSITIVE,
            }
        };

        *did_generate_constraint = true;
    } else if (have_c1 && have_c2) {
        *constraint = (struct dy_constraint){
            .tag = DY_CONSTRAINT_MULTIPLE,
            .multiple = {
                .c1 = alloc_constraint(c1),
                .c2 = alloc_constraint(c2),
                .polarity = DY_CORE_POLARITY_POSITIVE,
            }
        };
        *did_generate_constraint = true;
    } else if (have_c2 && have_c3) {
        *constraint = (struct dy_constraint){
            .tag = DY_CONSTRAINT_MULTIPLE,
            .multiple = {
                .c1 = alloc_constraint(c2),
                .c2 = alloc_constraint(c3),
                .polarity = DY_CORE_POLARITY_POSITIVE,
            }
        };
        *did_generate_constraint = true;
    } else if (have_c1 && have_c3) {
        *constraint = (struct dy_constraint){
            .tag = DY_CONSTRAINT_MULTIPLE,
            .multiple = {
                .c1 = alloc_constraint(c1),
                .c2 = alloc_constraint(c3),
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

    *new_elim = (struct dy_core_expr_map_elim){
        .expr = alloc_expr(expr),
        .expr_map = expr_map
    };

    return true;
}

bool dy_check_type_map_elim(struct dy_check_ctx ctx, struct dy_core_type_map_elim elim, struct dy_core_type_map_elim *new_elim, struct dy_constraint *constraint, bool *did_generate_constraint)
{
    struct dy_core_expr expr;
    struct dy_constraint c1;
    bool have_c1 = false;
    bool expr_succeeded = dy_check_expr(ctx, *elim.expr, &expr, &c1, &have_c1);

    struct dy_core_type_map type_map;
    struct dy_constraint c2;
    bool have_c2 = false;
    bool type_map_succeeded = dy_check_type_map(ctx, elim.type_map, &type_map, &c2, &have_c2);

    struct dy_core_expr type_map_expr = {
        .tag = DY_CORE_EXPR_TYPE_MAP,
        .type_map = type_map
    };

    struct dy_constraint c3;
    bool have_c3 = false;
    dy_ternary_t subtype_res = dy_is_subtype(ctx, dy_type_of(ctx, expr), type_map_expr, &c3, &have_c3, expr, &expr);

    if (!expr_succeeded || !type_map_succeeded || subtype_res == DY_NO) {
        return false;
    }

    if (have_c1 && have_c2 && have_c3) {
        struct dy_constraint c = {
            .tag = DY_CONSTRAINT_MULTIPLE,
            .multiple = {
                .c1 = alloc_constraint(c1),
                .c2 = alloc_constraint(c2),
                .polarity = DY_CORE_POLARITY_POSITIVE,
            }
        };

        *constraint = (struct dy_constraint){
            .tag = DY_CONSTRAINT_MULTIPLE,
            .multiple = {
                .c1 = alloc_constraint(c),
                .c2 = alloc_constraint(c3),
                .polarity = DY_CORE_POLARITY_POSITIVE,
            }
        };

        *did_generate_constraint = true;
    } else if (have_c1 && have_c2) {
        *constraint = (struct dy_constraint){
            .tag = DY_CONSTRAINT_MULTIPLE,
            .multiple = {
                .c1 = alloc_constraint(c1),
                .c2 = alloc_constraint(c2),
                .polarity = DY_CORE_POLARITY_POSITIVE,
            }
        };
        *did_generate_constraint = true;
    } else if (have_c2 && have_c3) {
        *constraint = (struct dy_constraint){
            .tag = DY_CONSTRAINT_MULTIPLE,
            .multiple = {
                .c1 = alloc_constraint(c2),
                .c2 = alloc_constraint(c3),
                .polarity = DY_CORE_POLARITY_POSITIVE,
            }
        };
        *did_generate_constraint = true;
    } else if (have_c1 && have_c3) {
        *constraint = (struct dy_constraint){
            .tag = DY_CONSTRAINT_MULTIPLE,
            .multiple = {
                .c1 = alloc_constraint(c1),
                .c2 = alloc_constraint(c3),
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
        .expr = alloc_expr(expr),
        .type_map = type_map
    };

    return true;
}

bool dy_check_both(struct dy_check_ctx ctx, struct dy_core_both both, struct dy_core_both *new_both, struct dy_constraint *constraint, bool *did_generate_constraint)
{
    struct dy_core_expr e1;
    struct dy_constraint c1;
    bool have_c1 = false;
    bool first_succeeded = dy_check_expr(ctx, *both.e1, &e1, &c1, &have_c1);
    both.e1 = alloc_expr(e1);

    struct dy_core_expr e2;
    struct dy_constraint c2;
    bool have_c2 = false;
    bool second_succeeded = dy_check_expr(ctx, *both.e2, &e2, &c2, &have_c2);
    both.e2 = alloc_expr(e2);

    if (!first_succeeded || !second_succeeded) {
        return false;
    }

    if (have_c1 && have_c2) {
        *constraint = (struct dy_constraint){
            .tag = DY_CONSTRAINT_MULTIPLE,
            .multiple = {
                .c1 = alloc_constraint(c1),
                .c2 = alloc_constraint(c2),
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
                    .c1 = alloc_constraint(c1),
                    .c2 = alloc_constraint(c2),
                    .polarity = DY_CORE_POLARITY_NEGATIVE,
                }
            };
            *did_generate_constraint = true;
        }

        *new_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_ONE_OF,
            .one_of = {
                .first = alloc_expr(first),
                .second = alloc_expr(second),
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

bool dy_check_inference_ctx(struct dy_check_ctx ctx, struct dy_core_inference_ctx inference_ctx, struct dy_core_expr *new_expr, struct dy_constraint *constraint, bool *did_generate_constraint)
{
    struct dy_core_expr type;
    struct dy_constraint c1;
    bool have_c1 = false;
    bool arg_succeeded = dy_check_expr(ctx, *inference_ctx.type, &type, &c1, &have_c1);

    struct dy_core_expr expr;
    if (arg_succeeded) {
        struct dy_core_expr new_unknown = {
            .tag = DY_CORE_EXPR_UNKNOWN,
            .unknown = {
                .id = inference_ctx.id,
                .type = alloc_expr(type),
                .is_inference_var = true,
            },
        };

        expr = substitute(inference_ctx.id, new_unknown, *inference_ctx.expr);
    } else {
        expr = *inference_ctx.expr;
    }

    struct dy_constraint c2;
    bool have_c2 = false;
    bool expr_succeeded = dy_check_expr(ctx, expr, &expr, &c2, &have_c2);

    if (!arg_succeeded || !expr_succeeded) {
        return false;
    }

    dy_array_t *ids = dy_array_create(sizeof(size_t), 4);
    dy_binding_contraints(ctx, inference_ctx.id, c2, have_c2, ids);

    if (dy_array_size(ids) == 0) {
        dy_array_destroy(ids);

        bool have_new_constaint = false;
        if (!resolve_implicit(ctx, inference_ctx.id, *inference_ctx.type, inference_ctx.polarity, c2, have_c2, &c2, &have_new_constaint, expr, &expr)) {
            return false;
        }

        have_c2 = have_new_constaint;
    } else {
        struct dy_bound_constraint bound_constraint = {
            .id = inference_ctx.id,
            .type = *inference_ctx.type,
            .binding_ids = ids
        };

        dy_array_add(ctx.bound_constraints, &bound_constraint);
    }

    if (have_c1 && have_c2) {
        *constraint = (struct dy_constraint){
            .tag = DY_CONSTRAINT_MULTIPLE,
            .multiple = {
                .c1 = alloc_constraint(c1),
                .c2 = alloc_constraint(c2),
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

    *new_expr = expr;

    return true;
}

bool is_bound(size_t id, struct dy_core_expr expr)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_EXPR_MAP:
        return is_bound(id, *expr.expr_map.e1) || is_bound(id, *expr.expr_map.e2);
    case DY_CORE_EXPR_TYPE_MAP:
        if (is_bound(id, *expr.type_map.arg_type)) {
            return true;
        }

        if (id == expr.type_map.arg_id) {
            return false;
        }

        return is_bound(id, *expr.type_map.expr);
    case DY_CORE_EXPR_EXPR_MAP_ELIM:
        return is_bound(id, *expr.expr_map_elim.expr) || is_bound(id, *expr.expr_map_elim.expr_map.e1) || is_bound(id, *expr.expr_map_elim.expr_map.e2);
    case DY_CORE_EXPR_TYPE_MAP_ELIM:
        if (is_bound(id, *expr.type_map_elim.expr) || is_bound(id, *expr.type_map_elim.type_map.arg_type)) {
            return true;
        }

        if (id == expr.type_map_elim.type_map.arg_id) {
            return false;
        }

        return is_bound(id, *expr.type_map_elim.type_map.expr);
    case DY_CORE_EXPR_BOTH:
        return is_bound(id, *expr.both.e1) || is_bound(id, *expr.both.e2);
    case DY_CORE_EXPR_ONE_OF:
        return is_bound(id, *expr.one_of.first) || is_bound(id, *expr.one_of.second);
    case DY_CORE_EXPR_UNKNOWN:
        return expr.unknown.id == id;
    case DY_CORE_EXPR_INFERENCE_CTX:
        if (is_bound(id, *expr.inference_ctx.type)) {
            return true;
        }

        if (id == expr.inference_ctx.id) {
            return false;
        }

        return is_bound(id, *expr.inference_ctx.expr);
    case DY_CORE_EXPR_STRING:
        // fallthrough
    case DY_CORE_EXPR_TYPE_OF_STRINGS:
        // fallthrough
    case DY_CORE_EXPR_END:
        // fallthrough
    case DY_CORE_EXPR_PRINT:
        return false;
    }
}

void dy_binding_contraints(struct dy_check_ctx ctx, size_t id, struct dy_constraint constraint, bool have_constraint, dy_array_t *ids)
{
    if (!have_constraint) {
        return;
    }

    switch (constraint.tag) {
    case DY_CONSTRAINT_SINGLE:
        if (constraint.single.id == id) {
            return;
        }

        // No duplicates.
        for (size_t i = 0; i < dy_array_size(ids); ++i) {
            size_t binding_id;
            dy_array_get(ids, i, &binding_id);

            if (constraint.single.id == binding_id) {
                return;
            }
        }

        // No mutually recursive dependencies.
        // TODO: Actually follow indirection.
        for (size_t i = dy_array_size(ctx.bound_constraints); i-- > 0;) {
            struct dy_bound_constraint bound_constraint;
            dy_array_get(ctx.bound_constraints, i, &bound_constraint);

            if (constraint.single.id != bound_constraint.id) {
                continue;
            }

            for (size_t k = 0, size = dy_array_size(bound_constraint.binding_ids); k < size; ++k) {
                size_t binding_id;
                dy_array_get(bound_constraint.binding_ids, k, &binding_id);
                if (binding_id == id) {
                    return;
                }
            }
        }

        if (constraint.single.range.have_subtype && is_bound(id, constraint.single.range.subtype)) {
            dy_array_add(ids, &constraint.single.id);
            return;
        }

        if (constraint.single.range.have_supertype && is_bound(id, constraint.single.range.supertype)) {
            dy_array_add(ids, &constraint.single.id);
            return;
        }

        return;
    case DY_CONSTRAINT_MULTIPLE:
        dy_binding_contraints(ctx, id, *constraint.multiple.c1, true, ids);
        dy_binding_contraints(ctx, id, *constraint.multiple.c2, true, ids);
        return;
    }

    DY_IMPOSSIBLE_ENUM();
}

bool resolve_implicit(struct dy_check_ctx ctx, size_t id, struct dy_core_expr type, enum dy_core_polarity polarity, struct dy_constraint constraint, bool have_constraint, struct dy_constraint *new_constraint, bool *have_new_constraint, struct dy_core_expr expr, struct dy_core_expr *new_expr)
{
    if (have_constraint) {
        struct dy_constraint_range solution = dy_constraint_collect(constraint, id);

        switch (polarity) {
        case DY_CORE_POLARITY_POSITIVE:
            if (solution.have_supertype) {
                dy_assert(!is_bound(id, solution.supertype));

                expr = substitute(id, solution.supertype, expr);
            } else {
                struct dy_core_expr new_unknown = {
                    .tag = DY_CORE_EXPR_UNKNOWN,
                    .unknown = {
                        .id = id,
                        .type = alloc_expr(type),
                        .is_inference_var = false,
                    }
                };

                expr = substitute(id, new_unknown, expr);

                expr = (struct dy_core_expr){
                    .tag = DY_CORE_EXPR_TYPE_MAP,
                    .type_map = {
                        .arg_id = id,
                        .arg_type = alloc_expr(type),
                        .polarity = DY_CORE_POLARITY_POSITIVE,
                        .expr = alloc_expr(expr),
                        .is_implicit = true,
                    }
                };
            }
            break;
        case DY_CORE_POLARITY_NEGATIVE:
            if (solution.have_subtype) {
                dy_assert(!is_bound(id, solution.subtype));

                expr = substitute(id, solution.subtype, expr);
            } else {
                struct dy_core_expr new_unknown = {
                    .tag = DY_CORE_EXPR_UNKNOWN,
                    .unknown = {
                        .id = id,
                        .type = alloc_expr(type),
                        .is_inference_var = false,
                    }
                };

                expr = substitute(id, new_unknown, expr);

                expr = (struct dy_core_expr){
                    .tag = DY_CORE_EXPR_TYPE_MAP,
                    .type_map = {
                        .arg_id = id,
                        .arg_type = alloc_expr(type),
                        .polarity = DY_CORE_POLARITY_POSITIVE,
                        .expr = alloc_expr(expr),
                        .is_implicit = true,
                    }
                };
            }
            break;
        }

        have_constraint = false;
        if (!dy_check_expr(ctx, expr, &expr, &constraint, &have_constraint)) {
            return false;
        }
    } else {
        struct dy_core_expr new_unknown = {
            .tag = DY_CORE_EXPR_UNKNOWN,
            .unknown = {
                .id = id,
                .type = alloc_expr(type),
                .is_inference_var = false,
            }
        };

        expr = substitute(id, new_unknown, expr);

        expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_TYPE_MAP,
            .type_map = {
                .arg_id = id,
                .arg_type = alloc_expr(type),
                .polarity = DY_CORE_POLARITY_POSITIVE,
                .expr = alloc_expr(expr),
                .is_implicit = true,
            }
        };
    }

    for (size_t i = dy_array_size(ctx.bound_constraints); i-- > 0;) {
        struct dy_bound_constraint bound_constraint;
        dy_array_get(ctx.bound_constraints, i, &bound_constraint);

        remove_id(bound_constraint.binding_ids, id);

        if (dy_array_size(bound_constraint.binding_ids) == 0) {
            dy_array_destroy(bound_constraint.binding_ids);

            dy_array_remove(ctx.bound_constraints, i);

            bool have_new_constraint2 = false;
            if (!resolve_implicit(ctx, bound_constraint.id, bound_constraint.type, polarity, constraint, have_constraint, &constraint, &have_new_constraint2, expr, &expr)) {
                return false;
            }

            have_constraint = have_new_constraint2;

            i = dy_array_size(ctx.bound_constraints);
        }
    }

    if (have_constraint) {
        *new_constraint = constraint;
        *have_new_constraint = true;
    }

    *new_expr = expr;

    return true;
}

void remove_id(dy_array_t *ids, size_t id_to_remove)
{
    for (size_t i = 0, size = dy_array_size(ids); i < size; ++i) {
        size_t id;
        dy_array_get(ids, i, &id);

        if (id == id_to_remove) {
            dy_array_remove(ids, i);
            return;
        }
    }
}

bool is_mentioned_in_constraints(struct dy_check_ctx ctx, size_t id, struct dy_constraint constraint)
{
    switch (constraint.tag) {
    case DY_CONSTRAINT_SINGLE:
        if (constraint.single.range.have_subtype && is_bound(id, constraint.single.range.subtype)) {
            return true;
        }

        if (constraint.single.range.have_supertype && is_bound(id, constraint.single.range.supertype)) {
            return true;
        }

        return false;
    case DY_CONSTRAINT_MULTIPLE:
        return is_mentioned_in_constraints(ctx, id, *constraint.multiple.c1)
               || is_mentioned_in_constraints(ctx, id, *constraint.multiple.c2);
    }
}

struct dy_core_expr *alloc_expr(struct dy_core_expr expr)
{
    return dy_alloc_and_copy(&expr, sizeof expr);
}

struct dy_constraint *alloc_constraint(struct dy_constraint constraint)
{
    return dy_alloc_and_copy(&constraint, sizeof constraint);
}

void print_core_expr(struct dy_core_expr expr)
{
    dy_array_t *s = dy_array_create(1, 64);
    dy_core_expr_to_string(expr, s);

    for (size_t i = 0; i < dy_array_size(s); ++i) {
        char c;
        dy_array_get(s, i, &c);
        fprintf(stderr, "%c", c);
    }

    dy_array_destroy(s);
}

void print_constraint(struct dy_constraint c)
{
    switch (c.tag) {
    case DY_CONSTRAINT_SINGLE:
        if (c.single.range.have_subtype) {
            print_core_expr(c.single.range.subtype);
            fprintf(stderr, " <: ");
        }
        fprintf(stderr, "%zu", c.single.id);
        if (c.single.range.have_supertype) {
            fprintf(stderr, " <: ");
            print_core_expr(c.single.range.supertype);
        }
        break;
    case DY_CONSTRAINT_MULTIPLE:
        fprintf(stderr, "(");
        print_constraint(*c.multiple.c1);
        switch (c.multiple.polarity) {
        case DY_CORE_POLARITY_POSITIVE:
            fprintf(stderr, " & ");
            break;
        case DY_CORE_POLARITY_NEGATIVE:
            fprintf(stderr, " | ");
            break;
        }
        print_constraint(*c.multiple.c2);
        fprintf(stderr, ")");
    }
}
