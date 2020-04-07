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

static struct dy_constraint *alloc_constraint(struct dy_constraint constraint);

static void dy_binding_contraints(struct dy_core_ctx ctx, size_t id, struct dy_constraint constraint, bool have_constraint, dy_array_t *ids);

static struct dy_core_expr resolve_implicit(struct dy_core_ctx ctx, size_t id, struct dy_core_expr type, enum dy_core_polarity polarity, struct dy_constraint constraint, bool have_constraint, struct dy_constraint *new_constraint, bool *have_new_constraint, struct dy_core_expr expr);
static void remove_id(dy_array_t *ids, size_t id);

static bool is_mentioned_in_constraints(struct dy_core_ctx ctx, size_t id, struct dy_constraint constraint);

struct dy_core_expr dy_check_expr(struct dy_core_ctx ctx, struct dy_core_expr expr, struct dy_constraint *constraint, bool *did_generate_constraint)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_EXPR_MAP:
        expr.expr_map = dy_check_expr_map(ctx, expr.expr_map, constraint, did_generate_constraint);
        return expr;
    case DY_CORE_EXPR_TYPE_MAP:
        expr.type_map = dy_check_type_map(ctx, expr.type_map, constraint, did_generate_constraint);
        return expr;
    case DY_CORE_EXPR_EXPR_MAP_ELIM:
        expr.expr_map_elim = dy_check_expr_map_elim(ctx, expr.expr_map_elim, constraint, did_generate_constraint);
        return expr;
    case DY_CORE_EXPR_TYPE_MAP_ELIM:
        expr.type_map_elim = dy_check_type_map_elim(ctx, expr.type_map_elim, constraint, did_generate_constraint);
        return expr;
    case DY_CORE_EXPR_BOTH:
        expr.both = dy_check_both(ctx, expr.both, constraint, did_generate_constraint);
        return expr;
    case DY_CORE_EXPR_RECURSION:
        expr.recursion = dy_check_recursion(ctx, expr.recursion, constraint, did_generate_constraint);
        return expr;
    case DY_CORE_EXPR_ONE_OF:
        expr.one_of = dy_check_one_of(ctx, expr.one_of, constraint, did_generate_constraint);
        return expr;
    case DY_CORE_EXPR_INFERENCE_CTX:
        return dy_check_inference_ctx(ctx, expr.inference_ctx, constraint, did_generate_constraint);
    case DY_CORE_EXPR_UNKNOWN:
        return dy_core_expr_retain(ctx.expr_pool, expr);
    case DY_CORE_EXPR_END:
        return dy_core_expr_retain(ctx.expr_pool, expr);
    case DY_CORE_EXPR_STRING:
        return dy_core_expr_retain(ctx.expr_pool, expr);
    case DY_CORE_EXPR_TYPE_OF_STRINGS:
        return dy_core_expr_retain(ctx.expr_pool, expr);
    case DY_CORE_EXPR_PRINT:
        return dy_core_expr_retain(ctx.expr_pool, expr);
    }

    DY_IMPOSSIBLE_ENUM();
}

struct dy_core_expr_map dy_check_expr_map(struct dy_core_ctx ctx, struct dy_core_expr_map expr_map, struct dy_constraint *constraint, bool *did_generate_constraint)
{
    struct dy_constraint c1;
    bool have_c1 = false;
    struct dy_core_expr e1 = dy_check_expr(ctx, *expr_map.e1, &c1, &have_c1);
    expr_map.e1 = dy_core_expr_new(ctx.expr_pool, e1);

    struct dy_constraint c2;
    bool have_c2 = false;
    struct dy_core_expr e2 = dy_check_expr(ctx, *expr_map.e2, &c2, &have_c2);
    expr_map.e2 = dy_core_expr_new(ctx.expr_pool, e2);

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

    return expr_map;
}

struct dy_core_type_map dy_check_type_map(struct dy_core_ctx ctx, struct dy_core_type_map type_map, struct dy_constraint *constraint, bool *did_generate_constraint)
{
    struct dy_constraint c1;
    bool have_c1 = false;
    struct dy_core_expr type = dy_check_expr(ctx, *type_map.arg_type, &c1, &have_c1);
    type_map.arg_type = dy_core_expr_new(ctx.expr_pool, type);

    struct dy_core_expr new_unknown = {
        .tag = DY_CORE_EXPR_UNKNOWN,
        .unknown = {
            .id = type_map.arg_id,
            .type = dy_core_expr_new(ctx.expr_pool, dy_core_expr_retain(ctx.expr_pool, type)),
            .is_inference_var = false,
        },
    };

    struct dy_core_expr expr = substitute(ctx, type_map.arg_id, new_unknown, *type_map.expr);

    dy_core_expr_release(ctx.expr_pool, new_unknown);

    struct dy_constraint c2;
    bool have_c2 = false;
    struct dy_core_expr new_expr = dy_check_expr(ctx, expr, &c2, &have_c2);

    dy_core_expr_release(ctx.expr_pool, expr);
    type_map.expr = dy_core_expr_new(ctx.expr_pool, new_expr);

    if (have_c2) {
        // Check to see if we're bound in one of the constraints.
        // If that's the case, error out for now.
        if (is_mentioned_in_constraints(ctx, type_map.arg_id, c2)) {
            dy_bail("shit");
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

    return type_map;
}

struct dy_core_expr_map_elim dy_check_expr_map_elim(struct dy_core_ctx ctx, struct dy_core_expr_map_elim elim, struct dy_constraint *constraint, bool *did_generate_constraint)
{
    struct dy_constraint c1;
    bool have_c1 = false;
    struct dy_core_expr expr = dy_check_expr(ctx, *elim.expr, &c1, &have_c1);

    struct dy_constraint c2;
    bool have_c2 = false;
    elim.expr_map = dy_check_expr_map(ctx, elim.expr_map, &c2, &have_c2);

    struct dy_constraint c3;
    bool have_c3 = false;
    if (elim.check_result == DY_MAYBE) {
        struct dy_core_expr type_of_expr = dy_type_of(ctx, expr);

        struct dy_core_expr new_expr;
        if (dy_core_expr_is_computation(*elim.expr_map.e1)) {
            struct dy_core_expr type_of_expr_map_e1 = dy_type_of(ctx, *elim.expr_map.e1);

            struct dy_core_expr type_map_expr = {
                .tag = DY_CORE_EXPR_TYPE_MAP,
                .type_map = {
                    .arg_id = (*ctx.running_id)++,
                    .arg_type = dy_core_expr_new(ctx.expr_pool, type_of_expr_map_e1),
                    .expr = dy_core_expr_retain_ptr(ctx.expr_pool, elim.expr_map.e2),
                    .polarity = DY_CORE_POLARITY_POSITIVE,
                    .is_implicit = elim.expr_map.is_implicit,
                }
            };

            elim.check_result = dy_is_subtype(ctx, type_of_expr, type_map_expr, &c3, &have_c3, expr, &new_expr);

            dy_core_expr_release(ctx.expr_pool, type_of_expr_map_e1);
        } else {
            struct dy_core_expr expr_map_expr = {
                .tag = DY_CORE_EXPR_EXPR_MAP,
                .expr_map = elim.expr_map
            };

            elim.check_result = dy_is_subtype(ctx, type_of_expr, expr_map_expr, &c3, &have_c3, expr, &new_expr);
        }

        dy_core_expr_release(ctx.expr_pool, type_of_expr);
        dy_core_expr_release(ctx.expr_pool, expr);

        expr = new_expr;
    }

    elim.expr = dy_core_expr_new(ctx.expr_pool, expr);

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

    return elim;
}

struct dy_core_type_map_elim dy_check_type_map_elim(struct dy_core_ctx ctx, struct dy_core_type_map_elim elim, struct dy_constraint *constraint, bool *did_generate_constraint)
{
    struct dy_constraint c1;
    bool have_c1 = false;
    struct dy_core_expr expr = dy_check_expr(ctx, *elim.expr, &c1, &have_c1);

    struct dy_constraint c2;
    bool have_c2 = false;
    elim.type_map = dy_check_type_map(ctx, elim.type_map, &c2, &have_c2);

    struct dy_constraint c3;
    bool have_c3 = false;
    if (elim.check_result == DY_MAYBE) {
        struct dy_core_expr type_map_expr = {
            .tag = DY_CORE_EXPR_TYPE_MAP,
            .type_map = elim.type_map
        };

        struct dy_core_expr type_of_expr = dy_type_of(ctx, expr);

        struct dy_core_expr new_expr;
        elim.check_result = dy_is_subtype(ctx, type_of_expr, type_map_expr, &c3, &have_c3, expr, &new_expr);

        dy_core_expr_release(ctx.expr_pool, type_of_expr);
        dy_core_expr_release(ctx.expr_pool, expr);

        expr = new_expr;
    }

    elim.expr = dy_core_expr_new(ctx.expr_pool, expr);

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

    return elim;
}

struct dy_core_both dy_check_both(struct dy_core_ctx ctx, struct dy_core_both both, struct dy_constraint *constraint, bool *did_generate_constraint)
{
    struct dy_constraint c1;
    bool have_c1 = false;
    struct dy_core_expr e1 = dy_check_expr(ctx, *both.e1, &c1, &have_c1);
    both.e1 = dy_core_expr_new(ctx.expr_pool, e1);

    struct dy_constraint c2;
    bool have_c2 = false;
    struct dy_core_expr e2 = dy_check_expr(ctx, *both.e2, &c2, &have_c2);
    both.e2 = dy_core_expr_new(ctx.expr_pool, e2);

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

    return both;
}

struct dy_core_one_of dy_check_one_of(struct dy_core_ctx ctx, struct dy_core_one_of one_of, struct dy_constraint *constraint, bool *did_generate_constraint)
{
    struct dy_constraint c1;
    bool have_c1 = false;
    struct dy_core_expr first = dy_check_expr(ctx, *one_of.first, &c1, &have_c1);
    one_of.first = dy_core_expr_new(ctx.expr_pool, first);

    struct dy_constraint c2;
    bool have_c2 = false;
    struct dy_core_expr second = dy_check_expr(ctx, *one_of.second, &c2, &have_c2);
    one_of.second = dy_core_expr_new(ctx.expr_pool, second);

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

    return one_of;
}

struct dy_core_recursion dy_check_recursion(struct dy_core_ctx ctx, struct dy_core_recursion recursion, struct dy_constraint *constraint, bool *did_generate_constraint)
{
    struct dy_constraint c1;
    bool have_c1 = false;
    struct dy_core_expr type = dy_check_expr(ctx, *recursion.type, &c1, &have_c1);
    recursion.type = dy_core_expr_new(ctx.expr_pool, type);

    struct dy_core_expr unknown = {
        .tag = DY_CORE_EXPR_UNKNOWN,
        .unknown = {
            .id = recursion.id,
            .type = dy_core_expr_new(ctx.expr_pool, dy_core_expr_retain(ctx.expr_pool, type)),
            .is_inference_var = false,
        }
    };

    struct dy_core_expr body = substitute(ctx, recursion.id, unknown, *recursion.expr);

    dy_core_expr_release(ctx.expr_pool, unknown);

    struct dy_constraint c2;
    bool have_c2 = false;
    struct dy_core_expr new_body = dy_check_expr(ctx, body, &c2, &have_c2);

    dy_core_expr_release(ctx.expr_pool, body);
    body = new_body;

    struct dy_constraint c3;
    bool have_c3 = false;
    if (recursion.check_result == DY_MAYBE) {
        struct dy_core_expr type_of_body = dy_type_of(ctx, body);

        recursion.check_result = dy_is_subtype(ctx, type_of_body, type, &c3, &have_c3, body, &new_body);

        dy_core_expr_release(ctx.expr_pool, body);
        body = new_body;
    }

    recursion.expr = dy_core_expr_new(ctx.expr_pool, body);

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

    return recursion;
}

struct dy_core_expr dy_check_inference_ctx(struct dy_core_ctx ctx, struct dy_core_inference_ctx inference_ctx, struct dy_constraint *constraint, bool *did_generate_constraint)
{
    struct dy_constraint c1;
    bool have_c1 = false;
    struct dy_core_expr type = dy_check_expr(ctx, *inference_ctx.type, &c1, &have_c1);

    struct dy_core_expr new_unknown = {
        .tag = DY_CORE_EXPR_UNKNOWN,
        .unknown = {
            .id = inference_ctx.id,
            .type = dy_core_expr_new(ctx.expr_pool, dy_core_expr_retain(ctx.expr_pool, type)),
            .is_inference_var = true,
        },
    };

    struct dy_core_expr inner_expr = substitute(ctx, inference_ctx.id, new_unknown, *inference_ctx.expr);

    dy_core_expr_release(ctx.expr_pool, new_unknown);

    struct dy_constraint c2;
    bool have_c2 = false;
    struct dy_core_expr new_inner_expr = dy_check_expr(ctx, inner_expr, &c2, &have_c2);

    dy_core_expr_release(ctx.expr_pool, inner_expr);

    dy_array_t *ids = dy_array_create(sizeof(size_t), 4);
    dy_binding_contraints(ctx, inference_ctx.id, c2, have_c2, ids);

    if (dy_array_size(ids) == 0) {
        dy_array_destroy(ids);

        dy_core_expr_release(ctx.expr_pool, type);

        struct dy_constraint c3;
        bool have_new_constaint = false;
        struct dy_core_expr resolved_expr = resolve_implicit(ctx, inference_ctx.id, *inference_ctx.type, inference_ctx.polarity, c2, have_c2, &c3, &have_new_constaint, new_inner_expr);

        dy_core_expr_release(ctx.expr_pool, new_inner_expr);

        new_inner_expr = resolved_expr;

        have_c2 = have_new_constaint;
        c2 = c3;
    } else {
        struct dy_bound_constraint bound_constraint = {
            .id = inference_ctx.id,
            .type = type,
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

    return new_inner_expr;
}

void dy_binding_contraints(struct dy_core_ctx ctx, size_t id, struct dy_constraint constraint, bool have_constraint, dy_array_t *ids)
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

        if (constraint.single.range.have_subtype && dy_core_expr_is_bound(id, constraint.single.range.subtype)) {
            dy_array_add(ids, &constraint.single.id);
            return;
        }

        if (constraint.single.range.have_supertype && dy_core_expr_is_bound(id, constraint.single.range.supertype)) {
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

struct dy_core_expr resolve_implicit(struct dy_core_ctx ctx, size_t id, struct dy_core_expr type, enum dy_core_polarity polarity, struct dy_constraint constraint, bool have_constraint, struct dy_constraint *new_constraint, bool *have_new_constraint, struct dy_core_expr expr)
{
    if (have_constraint) {
        struct dy_constraint_range solution = dy_constraint_collect(ctx, constraint, id);

        switch (polarity) {
        case DY_CORE_POLARITY_POSITIVE:
            if (solution.have_subtype) {
                dy_core_expr_release(ctx.expr_pool, solution.subtype);
            }

            if (solution.have_supertype) {
                dy_assert(!dy_core_expr_is_bound(id, solution.supertype));

                expr = substitute(ctx, id, solution.supertype, expr);
            } else {
                struct dy_core_expr type_of_expr = dy_type_of(ctx, expr);

                if (dy_core_expr_num_ocurrences(id, type_of_expr) > 1) {
                    struct dy_core_expr new_unknown = {
                        .tag = DY_CORE_EXPR_UNKNOWN,
                        .unknown = {
                            .id = id,
                            .type = dy_core_expr_new(ctx.expr_pool, dy_core_expr_retain(ctx.expr_pool, type)),
                            .is_inference_var = false,
                        }
                    };

                    expr = substitute(ctx, id, new_unknown, expr);

                    dy_core_expr_release(ctx.expr_pool, new_unknown);

                    expr = (struct dy_core_expr){
                        .tag = DY_CORE_EXPR_TYPE_MAP,
                        .type_map = {
                            .arg_id = id,
                            .arg_type = dy_core_expr_new(ctx.expr_pool, dy_core_expr_retain(ctx.expr_pool, type)),
                            .polarity = DY_CORE_POLARITY_POSITIVE,
                            .expr = dy_core_expr_new(ctx.expr_pool, expr),
                            .is_implicit = true,
                        }
                    };
                } else {
                    struct dy_core_expr all = {
                        .tag = DY_CORE_EXPR_END,
                        .end_polarity = DY_CORE_POLARITY_POSITIVE
                    };

                    expr = substitute(ctx, id, all, expr);
                }

                dy_core_expr_release(ctx.expr_pool, type_of_expr);
            }
            break;
        case DY_CORE_POLARITY_NEGATIVE:
            if (solution.have_supertype) {
                dy_core_expr_release(ctx.expr_pool, solution.supertype);
            }

            if (solution.have_subtype) {
                dy_assert(!dy_core_expr_is_bound(id, solution.subtype));

                expr = substitute(ctx, id, solution.subtype, expr);
            } else {
                struct dy_core_expr type_of_expr = dy_type_of(ctx, expr);

                if (dy_core_expr_num_ocurrences(id, type_of_expr) > 1) {
                    struct dy_core_expr new_unknown = {
                        .tag = DY_CORE_EXPR_UNKNOWN,
                        .unknown = {
                            .id = id,
                            .type = dy_core_expr_new(ctx.expr_pool, dy_core_expr_retain(ctx.expr_pool, type)),
                            .is_inference_var = false,
                        }
                    };

                    expr = substitute(ctx, id, new_unknown, expr);

                    dy_core_expr_release(ctx.expr_pool, new_unknown);

                    expr = (struct dy_core_expr){
                        .tag = DY_CORE_EXPR_TYPE_MAP,
                        .type_map = {
                            .arg_id = id,
                            .arg_type = dy_core_expr_new(ctx.expr_pool, dy_core_expr_retain(ctx.expr_pool, type)),
                            .polarity = DY_CORE_POLARITY_POSITIVE,
                            .expr = dy_core_expr_new(ctx.expr_pool, expr),
                            .is_implicit = true,
                        }
                    };
                } else {
                    struct dy_core_expr nothing = {
                        .tag = DY_CORE_EXPR_END,
                        .end_polarity = DY_CORE_POLARITY_NEGATIVE
                    };

                    expr = substitute(ctx, id, nothing, expr);
                }

                dy_core_expr_release(ctx.expr_pool, type_of_expr);
            }
            break;
        }

        have_constraint = false;
        struct dy_core_expr checked_expr = dy_check_expr(ctx, expr, &constraint, &have_constraint);

        dy_core_expr_release(ctx.expr_pool, expr);

        expr = checked_expr;
    } else {
        struct dy_core_expr type_of_expr = dy_type_of(ctx, expr);

        if (dy_core_expr_num_ocurrences(id, type_of_expr) > 1) {
            struct dy_core_expr new_unknown = {
                .tag = DY_CORE_EXPR_UNKNOWN,
                .unknown = {
                    .id = id,
                    .type = dy_core_expr_new(ctx.expr_pool, dy_core_expr_retain(ctx.expr_pool, type)),
                    .is_inference_var = false,
                }
            };

            expr = substitute(ctx, id, new_unknown, expr);

            dy_core_expr_release(ctx.expr_pool, new_unknown);

            expr = (struct dy_core_expr){
                .tag = DY_CORE_EXPR_TYPE_MAP,
                .type_map = {
                    .arg_id = id,
                    .arg_type = dy_core_expr_new(ctx.expr_pool, dy_core_expr_retain(ctx.expr_pool, type)),
                    .polarity = DY_CORE_POLARITY_POSITIVE,
                    .expr = dy_core_expr_new(ctx.expr_pool, expr),
                    .is_implicit = true,
                }
            };
        } else {
            struct dy_core_expr e;
            switch (polarity) {
            case DY_CORE_POLARITY_POSITIVE:
                e = (struct dy_core_expr){
                    .tag = DY_CORE_EXPR_END,
                    .end_polarity = DY_CORE_POLARITY_POSITIVE
                };

                break;
            case DY_CORE_POLARITY_NEGATIVE:
                e = (struct dy_core_expr){
                    .tag = DY_CORE_EXPR_END,
                    .end_polarity = DY_CORE_POLARITY_NEGATIVE
                };
                break;
            }

            expr = substitute(ctx, id, e, expr);
        }

        dy_core_expr_release(ctx.expr_pool, type_of_expr);
    }

    for (size_t i = dy_array_size(ctx.bound_constraints); i-- > 0;) {
        struct dy_bound_constraint bound_constraint;
        dy_array_get(ctx.bound_constraints, i, &bound_constraint);

        remove_id(bound_constraint.binding_ids, id);

        dy_binding_contraints(ctx, bound_constraint.id, constraint, have_constraint, bound_constraint.binding_ids);

        if (dy_array_size(bound_constraint.binding_ids) == 0) {
            dy_array_destroy(bound_constraint.binding_ids);

            dy_array_remove(ctx.bound_constraints, i);

            struct dy_constraint resolved_constraint;
            bool have_new_constraint2 = false;
            struct dy_core_expr resolved_expr = resolve_implicit(ctx, bound_constraint.id, bound_constraint.type, polarity, constraint, have_constraint, &resolved_constraint, &have_new_constraint2, expr);

            dy_core_expr_release(ctx.expr_pool, expr);

            if (have_constraint) {
                // release constraint
            }

            expr = resolved_expr;
            have_constraint = have_new_constraint2;
            constraint = resolved_constraint;

            i = dy_array_size(ctx.bound_constraints);
        }
    }

    if (have_constraint) {
        *new_constraint = constraint;
        *have_new_constraint = true;
    }

    return expr;
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

bool is_mentioned_in_constraints(struct dy_core_ctx ctx, size_t id, struct dy_constraint constraint)
{
    switch (constraint.tag) {
    case DY_CONSTRAINT_SINGLE:
        if (constraint.single.range.have_subtype && dy_core_expr_is_bound(id, constraint.single.range.subtype)) {
            return true;
        }

        if (constraint.single.range.have_supertype && dy_core_expr_is_bound(id, constraint.single.range.supertype)) {
            return true;
        }

        return false;
    case DY_CONSTRAINT_MULTIPLE:
        return is_mentioned_in_constraints(ctx, id, *constraint.multiple.c1)
               || is_mentioned_in_constraints(ctx, id, *constraint.multiple.c2);
    }
}

struct dy_constraint *alloc_constraint(struct dy_constraint constraint)
{
    return dy_alloc_and_copy(&constraint, sizeof constraint);
}
