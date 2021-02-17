/*
 * Copyright 2017-2021 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core.h"

/**
 * The functions here define equality for all objects of Core.
 *
 * Equality is generally purely syntactical, except for the following objects:
 *   - Functions/Recursions are alpha-converted (might use De-Bruijn indices at some point).
 */

static inline dy_ternary_t dy_are_equal(struct dy_core_ctx *ctx, struct dy_core_expr e1, struct dy_core_expr e2);

static inline dy_ternary_t dy_functions_are_equal(struct dy_core_ctx *ctx, struct dy_core_function fun1, struct dy_core_function fun2);

static inline dy_ternary_t dy_pairs_are_equal(struct dy_core_ctx *ctx, struct dy_core_pair p1, struct dy_core_pair p2);

static inline dy_ternary_t dy_recursions_are_equal(struct dy_core_ctx *ctx, struct dy_core_recursion rec1, struct dy_core_recursion rec2);

static inline dy_ternary_t dy_solutions_are_equal(struct dy_core_ctx *ctx, struct dy_core_solution sol1, struct dy_core_solution sol2);

static inline dy_ternary_t dy_applications_are_equal(struct dy_core_ctx *ctx, struct dy_core_application app1, struct dy_core_application app2);

static inline dy_ternary_t dy_variables_are_equal(struct dy_core_ctx *ctx, size_t id1, size_t id2);

dy_ternary_t dy_are_equal(struct dy_core_ctx *ctx, struct dy_core_expr e1, struct dy_core_expr e2)
{
    if (e1.tag == DY_CORE_EXPR_PROBLEM && e2.tag == DY_CORE_EXPR_PROBLEM) {
        if (e1.problem.tag != e2.problem.tag || e1.problem.is_implicit != e2.problem.is_implicit || e1.problem.polarity != e2.problem.polarity) {
            return DY_NO;
        }

        switch (e1.problem.tag) {
        case DY_CORE_FUNCTION:
            return dy_functions_are_equal(ctx, e1.problem.function, e2.problem.function);
        case DY_CORE_PAIR:
            return dy_pairs_are_equal(ctx, e1.problem.pair, e2.problem.pair);
        case DY_CORE_RECURSION:
            return dy_recursions_are_equal(ctx, e1.problem.recursion, e2.problem.recursion);
        }

        dy_bail("impossible");
    }

    if (e1.tag == DY_CORE_EXPR_SOLUTION && e2.tag == DY_CORE_EXPR_SOLUTION) {
        return dy_solutions_are_equal(ctx, e1.solution, e2.solution);
    }

    if (e1.tag == DY_CORE_EXPR_APPLICATION && e2.tag == DY_CORE_EXPR_APPLICATION) {
        return dy_applications_are_equal(ctx, e1.application, e2.application);
    }

    if (e1.tag == DY_CORE_EXPR_VARIABLE && e2.tag == DY_CORE_EXPR_VARIABLE) {
        return dy_variables_are_equal(ctx, e1.variable_id, e2.variable_id);
    }

    if (e1.tag == DY_CORE_EXPR_INFERENCE_VAR && e2.tag == DY_CORE_EXPR_INFERENCE_VAR) {
        return dy_variables_are_equal(ctx, e1.inference_var_id, e2.inference_var_id);
    }

    if (e1.tag == DY_CORE_EXPR_ANY && e2.tag == DY_CORE_EXPR_ANY) {
        return DY_YES;
    }

    if (e1.tag == DY_CORE_EXPR_VOID && e2.tag == DY_CORE_EXPR_VOID) {
        return DY_YES;
    }

    if (e1.tag == DY_CORE_EXPR_CUSTOM && e2.tag == DY_CORE_EXPR_CUSTOM && e1.custom.id == e2.custom.id) {
        const struct dy_core_custom_shared *vtab = dy_array_pos(&ctx->custom_shared, e1.custom.id);
        return vtab->is_equal(ctx, e1.custom.data, e2.custom.data);
    }

    if (e1.tag == DY_CORE_EXPR_APPLICATION || e2.tag == DY_CORE_EXPR_APPLICATION || e1.tag == DY_CORE_EXPR_VARIABLE || e2.tag == DY_CORE_EXPR_VARIABLE || e1.tag == DY_CORE_EXPR_INFERENCE_VAR || e2.tag == DY_CORE_EXPR_INFERENCE_VAR) {
        return DY_MAYBE;
    } else {
        return DY_NO;
    }
}

dy_ternary_t dy_functions_are_equal(struct dy_core_ctx *ctx, struct dy_core_function fun1, struct dy_core_function fun2)
{
    dy_array_add(&ctx->equal_variables, &(struct dy_equal_variables){
        .id1 = fun1.id,
        .id2 = fun2.id
    });

    dy_ternary_t result = dy_are_equal(ctx, *fun1.expr, *fun2.expr);

    --ctx->equal_variables.num_elems;

    return result;
}

dy_ternary_t dy_pairs_are_equal(struct dy_core_ctx *ctx, struct dy_core_pair p1, struct dy_core_pair p2)
{
    dy_ternary_t ret = dy_are_equal(ctx, *p1.left, *p2.left);
    if (ret != DY_YES) {
        return ret;
    }

    return dy_are_equal(ctx, *p1.right, *p2.right);
}

dy_ternary_t dy_recursions_are_equal(struct dy_core_ctx *ctx, struct dy_core_recursion rec1, struct dy_core_recursion rec2)
{
    dy_array_add(&ctx->equal_variables, &(struct dy_equal_variables){
        .id1 = rec1.id,
        .id2 = rec2.id
    });

    dy_ternary_t result = dy_are_equal(ctx, *rec1.expr, *rec2.expr);

    --ctx->equal_variables.num_elems;

    return result;
}

dy_ternary_t dy_solutions_are_equal(struct dy_core_ctx *ctx, struct dy_core_solution sol1, struct dy_core_solution sol2)
{
    if (sol1.tag != sol2.tag || sol1.is_implicit != sol2.is_implicit) {
        return DY_NO;
    }

    dy_ternary_t ret = DY_YES;
    if (sol1.tag == DY_CORE_FUNCTION) {
        ret = dy_are_equal(ctx, *sol1.expr, *sol2.expr);
    }

    dy_ternary_t ret2 = dy_are_equal(ctx, *sol1.out, *sol2.out);

    if (ret == DY_NO || ret2 == DY_NO) {
        return DY_NO;
    }

    if (ret == DY_MAYBE || ret2 == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

dy_ternary_t dy_applications_are_equal(struct dy_core_ctx *ctx, struct dy_core_application app1, struct dy_core_application app2)
{
    if (dy_are_equal(ctx, *app1.expr, *app2.expr) != DY_YES) {
        return DY_MAYBE;
    }

    if (dy_solutions_are_equal(ctx, app1.solution, app2.solution) != DY_YES) {
        return DY_MAYBE;
    }

    return DY_YES;
}

dy_ternary_t dy_variables_are_equal(struct dy_core_ctx *ctx, size_t id1, size_t id2)
{
    if (id1 == id2) {
        return DY_YES;
    }

    for (size_t i = 0, size = ctx->equal_variables.num_elems; i < size; ++i) {
        const struct dy_equal_variables *v = dy_array_pos(&ctx->equal_variables, i);

        if (v->id1 == id1 && v->id2 == id2) {
            return true;
        }

        if (v->id1 == id2 && v->id2 == id1) {
            return true;
        }
    }

    return DY_MAYBE;
}
