/*
 * Copyright 2017-2021 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core.h"
#include "constraint.h"
#include "type_of.h"
#include "substitute.h"
#include "are_equal.h"

/**
 * Implementation of the subtype check.
 *
 * Each subtype check has the following inputs:
 *    - The supposed subtype.
 *    - The supposed supertype.
 *    - The expression that is of type 'subtype'.
 *
 * And the following outputs:
 *    - Whether the subtype actually is a subtype of supertype; this is a ternary result.
 *    - Constraints that arose during subtype-checking inference variables.
 *    - A transformed 'subtype expr' as a result of subtype-checking implicit problems.
 *    - Inference variables created by transforming implicit functions.
 *      They are supposed to be resolved by the caller of 'is_subtype'.
 */

static inline dy_ternary_t dy_is_subtype(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_expr supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);


static inline dy_ternary_t dy_problem_is_subtype_of_problem(struct dy_core_ctx *ctx, struct dy_core_problem subtype, struct dy_core_problem supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);


static inline dy_ternary_t dy_positive_functions_are_subtypes(struct dy_core_ctx *ctx, struct dy_core_function subtype, struct dy_core_function supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t dy_negative_functions_are_subtypes(struct dy_core_ctx *ctx, struct dy_core_function subtype, struct dy_core_function supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t dy_positive_function_is_subtype_of_negative_function(struct dy_core_ctx *ctx, struct dy_core_function subtype, struct dy_core_function supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);


static inline dy_ternary_t dy_positive_pairs_are_subtypes(struct dy_core_ctx *ctx, struct dy_core_pair subtype, struct dy_core_pair supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t dy_negative_pairs_are_subtypes(struct dy_core_ctx *ctx, struct dy_core_pair subtype, struct dy_core_pair supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t dy_positive_pair_is_subtype_of_negative_pair(struct dy_core_ctx *ctx, struct dy_core_pair subtype, struct dy_core_pair supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);


static inline dy_ternary_t dy_positive_recursions_are_subtypes(struct dy_core_ctx *ctx, struct dy_core_recursion subtype, struct dy_core_recursion supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t dy_negative_recursions_are_subtypes(struct dy_core_ctx *ctx, struct dy_core_recursion subtype, struct dy_core_recursion supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t dy_positive_recursion_is_subtype_of_negative_recursion(struct dy_core_ctx *ctx, struct dy_core_recursion subtype, struct dy_core_recursion supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);


static inline dy_ternary_t dy_function_is_subtype_of_solution(struct dy_core_ctx *ctx, struct dy_core_function subtype, struct dy_core_solution supertype, bool is_implicit, enum dy_polarity polarity, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t dy_pair_is_subtype_of_solution(struct dy_core_ctx *ctx, struct dy_core_pair subtype, struct dy_core_solution supertype, bool is_implicit, enum dy_polarity polarity, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t dy_recursion_is_subtype_of_solution(struct dy_core_ctx *ctx, struct dy_core_recursion subtype, struct dy_core_solution supertype, bool is_implicit, enum dy_polarity polarity, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);


static inline dy_ternary_t dy_solution_is_subtype_of_function(struct dy_core_ctx *ctx, struct dy_core_solution subtype, struct dy_core_function supertype, struct dy_core_expr subtype_expr, bool is_implicit, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t dy_solution_is_subtype_of_pair(struct dy_core_ctx *ctx, struct dy_core_solution subtype, struct dy_core_pair supertype, struct dy_core_expr subtype_expr, bool is_implicit, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t dy_solution_is_subtype_of_recursion(struct dy_core_ctx *ctx, struct dy_core_solution subtype, struct dy_core_recursion supertype, struct dy_core_expr subtype_expr, bool is_implicit, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);


static inline dy_ternary_t dy_function_solution_is_subtype_of_solution(struct dy_core_ctx *ctx, struct dy_core_solution subtype, struct dy_core_solution supertype, struct dy_core_expr subtype_expr, bool is_implicit, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t dy_pair_solution_is_subtype_of_solution(struct dy_core_ctx *ctx, struct dy_core_solution subtype, struct dy_core_solution supertype, struct dy_core_expr subtype_expr, bool is_implicit, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t dy_recursion_solution_is_subtype_of_solution(struct dy_core_ctx *ctx, struct dy_core_solution subtype, struct dy_core_solution supertype, struct dy_core_expr subtype_expr, bool is_implicit, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);


static inline dy_ternary_t dy_positive_implicit_function_is_subtype(struct dy_core_ctx *ctx, struct dy_core_function subtype, struct dy_core_expr supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t dy_positive_implicit_pair_is_subtype(struct dy_core_ctx *ctx, struct dy_core_pair subtype, struct dy_core_expr supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_tranform_subtype_expr);

static inline dy_ternary_t dy_positive_implicit_recursion_is_subtype(struct dy_core_ctx *ctx, struct dy_core_recursion subtype, struct dy_core_expr supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);


static inline dy_ternary_t dy_negative_implicit_function_is_subtype(struct dy_core_ctx *ctx, struct dy_core_function subtype, struct dy_core_expr supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t dy_negative_implicit_pair_is_subtype(struct dy_core_ctx *ctx, struct dy_core_pair subtype, struct dy_core_expr supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_tranform_subtype_expr);

static inline dy_ternary_t dy_negative_implicit_recursion_is_subtype(struct dy_core_ctx *ctx, struct dy_core_recursion subtype, struct dy_core_expr supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);


static inline dy_ternary_t dy_is_subtype_of_positive_implicit_function(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_function supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t dy_is_subtype_of_positive_implicit_pair(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_pair supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t dy_is_subtype_of_positive_implicit_recursion(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_recursion supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);


static inline dy_ternary_t dy_is_subtype_of_negative_implicit_function(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_function supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t dy_is_subtype_of_negative_implicit_pair(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_pair supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t dy_is_subtype_of_negative_implicit_recursion(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_recursion supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

dy_ternary_t dy_is_subtype(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_expr supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    if (subtype.tag == DY_CORE_EXPR_PROBLEM && supertype.tag == DY_CORE_EXPR_PROBLEM) {
        dy_ternary_t res = dy_problem_is_subtype_of_problem(ctx, subtype.problem, supertype.problem, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
        if (res == DY_NO && (subtype.problem.is_implicit || supertype.problem.is_implicit)) {
            goto implicit_check;
        } else {
            return res;
        }
    }

    if (subtype.tag == DY_CORE_EXPR_PROBLEM && supertype.tag == DY_CORE_EXPR_SOLUTION && subtype.problem.is_implicit == supertype.solution.is_implicit) {
        if (subtype.problem.tag != supertype.solution.tag) {
            return DY_NO;
        }

        switch (subtype.problem.tag) {
        case DY_CORE_FUNCTION:
            return dy_function_is_subtype_of_solution(ctx, subtype.problem.function, supertype.solution, subtype.problem.is_implicit, subtype.problem.polarity, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
        case DY_CORE_PAIR:
            return dy_pair_is_subtype_of_solution(ctx, subtype.problem.pair, supertype.solution, subtype.problem.is_implicit, subtype.problem.polarity, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
        case DY_CORE_RECURSION:
            return dy_recursion_is_subtype_of_solution(ctx, subtype.problem.recursion, supertype.solution, subtype.problem.is_implicit, subtype.problem.polarity, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
        }
    }

    if (subtype.tag == DY_CORE_EXPR_SOLUTION && supertype.tag == DY_CORE_EXPR_PROBLEM && subtype.solution.is_implicit == supertype.problem.is_implicit) {
        if (supertype.problem.polarity == DY_POLARITY_POSITIVE || subtype.solution.tag != supertype.problem.tag) {
            return DY_NO;
        }

        dy_ternary_t ret;
        switch (subtype.problem.tag) {
        case DY_CORE_FUNCTION:
            ret = dy_solution_is_subtype_of_function(ctx, subtype.solution, supertype.problem.function, subtype_expr, subtype.solution.is_implicit, new_subtype_expr, did_transform_subtype_expr);
            break;
        case DY_CORE_PAIR:
            ret = dy_solution_is_subtype_of_pair(ctx, subtype.solution, supertype.problem.pair, subtype_expr, subtype.solution.is_implicit, new_subtype_expr, did_transform_subtype_expr);
            break;
        case DY_CORE_RECURSION:
            ret = dy_solution_is_subtype_of_recursion(ctx, subtype.solution, supertype.problem.recursion, subtype_expr, subtype.solution.is_implicit, new_subtype_expr, did_transform_subtype_expr);
            break;
        }

        if (ret == DY_YES) {
            return DY_MAYBE;
        } else {
            return ret;
        }
    }

    if (subtype.tag == DY_CORE_EXPR_SOLUTION && supertype.tag == DY_CORE_EXPR_SOLUTION) {
        if (subtype.solution.tag != supertype.solution.tag || subtype.solution.is_implicit != supertype.solution.is_implicit) {
            return DY_NO;
        }

        switch (subtype.solution.tag) {
        case DY_CORE_FUNCTION:
            return dy_function_solution_is_subtype_of_solution(ctx, subtype.solution, supertype.solution, subtype_expr, subtype.solution.is_implicit, new_subtype_expr, did_transform_subtype_expr);
        case DY_CORE_PAIR:
            return dy_pair_solution_is_subtype_of_solution(ctx, subtype.solution, supertype.solution, subtype_expr, subtype.solution.is_implicit, new_subtype_expr, did_transform_subtype_expr);
        case DY_CORE_RECURSION:
            return dy_recursion_solution_is_subtype_of_solution(ctx, subtype.solution, supertype.solution, subtype_expr, subtype.solution.is_implicit, new_subtype_expr, did_transform_subtype_expr);
        }

        dy_bail("Impossible!");
    }

    if (subtype.tag == DY_CORE_EXPR_APPLICATION && supertype.tag == DY_CORE_EXPR_APPLICATION) {
        return dy_applications_are_equal(ctx, subtype.application, supertype.application);
    }

    if (subtype.tag == DY_CORE_EXPR_VARIABLE && supertype.tag == DY_CORE_EXPR_VARIABLE) {
        return dy_variables_are_equal(ctx, subtype.variable_id, supertype.variable_id);
    }

    if (subtype.tag == DY_CORE_EXPR_INFERENCE_VAR && supertype.tag == DY_CORE_EXPR_INFERENCE_VAR) {
        if (dy_variables_are_equal(ctx, subtype.inference_var_id, supertype.inference_var_id) == DY_YES) {
            return DY_YES;
        }

        size_t constraint_start1 = ctx->constraints.num_elems;

        dy_array_add(&ctx->constraints, &(struct dy_constraint){
            .id = subtype.inference_var_id,
            .upper = dy_core_expr_retain(ctx, supertype),
            .have_lower = false,
            .have_upper = true
        });

        dy_array_add(&ctx->constraints, &(struct dy_constraint){
            .id = supertype.inference_var_id,
            .lower = dy_core_expr_retain(ctx, subtype),
            .have_lower = true,
            .have_upper = false
        });

        dy_join_constraints(ctx, constraint_start1, constraint_start1 + 1, DY_POLARITY_POSITIVE);

        return DY_MAYBE;
    }

    if (subtype.tag == DY_CORE_EXPR_ANY && supertype.tag == DY_CORE_EXPR_ANY) {
        return DY_YES;
    }

    if (subtype.tag == DY_CORE_EXPR_VOID && supertype.tag == DY_CORE_EXPR_VOID) {
        return DY_YES;
    }

    if (subtype.tag == DY_CORE_EXPR_CUSTOM && supertype.tag == DY_CORE_EXPR_CUSTOM && subtype.custom.id == supertype.custom.id) {
        const struct dy_core_custom_shared *vtab = dy_array_pos(&ctx->custom_shared, subtype.custom.id);
        return vtab->is_subtype(ctx, subtype.custom.data, supertype.custom.data, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
    }

    if (subtype.tag == DY_CORE_EXPR_ANY) {
        return DY_MAYBE;
    }

    if (supertype.tag == DY_CORE_EXPR_ANY) {
        return DY_YES;
    }

    if (subtype.tag == DY_CORE_EXPR_INFERENCE_VAR) {
        dy_array_add(&ctx->constraints, &(struct dy_constraint){
            .id = subtype.inference_var_id,
            .upper = dy_core_expr_retain(ctx, supertype),
            .have_upper = true,
            .have_lower = false
        });

        return DY_MAYBE;
    }

    if (supertype.tag == DY_CORE_EXPR_INFERENCE_VAR) {
        dy_array_add(&ctx->constraints, &(struct dy_constraint){
            .id = supertype.inference_var_id,
            .lower = dy_core_expr_retain(ctx, subtype),
            .have_lower = true,
            .have_upper = false
        });

        return DY_MAYBE;
    }

    if (subtype.tag == DY_CORE_EXPR_APPLICATION || subtype.tag == DY_CORE_EXPR_VARIABLE || supertype.tag == DY_CORE_EXPR_APPLICATION || supertype.tag == DY_CORE_EXPR_VARIABLE) {
        return DY_MAYBE;
    }

implicit_check:
    if (subtype.tag == DY_CORE_EXPR_PROBLEM && subtype.problem.is_implicit) {
        if (subtype.problem.polarity == DY_POLARITY_POSITIVE) {
            switch (subtype.problem.tag) {
            case DY_CORE_FUNCTION:
                return dy_positive_implicit_function_is_subtype(ctx, subtype.problem.function, supertype, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
            case DY_CORE_PAIR:
                return dy_positive_implicit_pair_is_subtype(ctx, subtype.problem.pair, supertype, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
            case DY_CORE_RECURSION:
                return dy_positive_implicit_recursion_is_subtype(ctx, subtype.problem.recursion, supertype, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
            }

            dy_bail("impossible");
        } else {
            switch (subtype.problem.tag) {
            case DY_CORE_FUNCTION:
                return dy_negative_implicit_function_is_subtype(ctx, subtype.problem.function, supertype, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
            case DY_CORE_PAIR:
                return dy_negative_implicit_pair_is_subtype(ctx, subtype.problem.pair, supertype, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
            case DY_CORE_RECURSION:
                return dy_negative_implicit_recursion_is_subtype(ctx, subtype.problem.recursion, supertype, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
            }

            dy_bail("impossible");
        }
    }

    if (supertype.tag == DY_CORE_EXPR_PROBLEM && supertype.problem.is_implicit) {
        if (supertype.problem.polarity == DY_POLARITY_POSITIVE) {
            switch (supertype.problem.tag) {
            case DY_CORE_FUNCTION:
                return dy_is_subtype_of_positive_implicit_function(ctx, subtype, supertype.problem.function, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
            case DY_CORE_PAIR:
                return dy_is_subtype_of_positive_implicit_pair(ctx, subtype, supertype.problem.pair, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
            case DY_CORE_RECURSION:
                return dy_is_subtype_of_positive_implicit_recursion(ctx, subtype, supertype.problem.recursion, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
            }

            dy_bail("impossible");
        } else {
            switch (supertype.problem.tag) {
            case DY_CORE_FUNCTION:
                return dy_is_subtype_of_negative_implicit_function(ctx, subtype, supertype.problem.function, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
            case DY_CORE_PAIR:
                return dy_is_subtype_of_negative_implicit_pair(ctx, subtype, supertype.problem.pair, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
            case DY_CORE_RECURSION:
                return dy_is_subtype_of_negative_implicit_recursion(ctx, subtype, supertype.problem.recursion, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
            }

            dy_bail("impossible");
        }
    }

    return DY_NO;
}

dy_ternary_t dy_problem_is_subtype_of_problem(struct dy_core_ctx *ctx, struct dy_core_problem subtype, struct dy_core_problem supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    if (subtype.is_implicit != supertype.is_implicit || subtype.tag != supertype.tag) {
        return DY_NO;
    }

    if (subtype.polarity == DY_POLARITY_NEGATIVE && supertype.polarity == DY_POLARITY_POSITIVE) {
        return DY_NO;
    }

    switch (subtype.tag) {
    case DY_CORE_FUNCTION:
        if (subtype.polarity == DY_POLARITY_POSITIVE && supertype.polarity == DY_POLARITY_POSITIVE) {
            return dy_positive_functions_are_subtypes(ctx, subtype.function, supertype.function, subtype.is_implicit, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
        }

        if (subtype.polarity == DY_POLARITY_NEGATIVE && supertype.polarity == DY_POLARITY_NEGATIVE) {
            return dy_negative_functions_are_subtypes(ctx, subtype.function, supertype.function, subtype.is_implicit, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
        }

        return dy_positive_function_is_subtype_of_negative_function(ctx, subtype.function, supertype.function, subtype.is_implicit, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
    case DY_CORE_PAIR:
        if (subtype.polarity == DY_POLARITY_POSITIVE && supertype.polarity == DY_POLARITY_POSITIVE) {
            return dy_positive_pairs_are_subtypes(ctx, subtype.pair, supertype.pair, subtype.is_implicit, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
        }

        if (subtype.polarity == DY_POLARITY_NEGATIVE && supertype.polarity == DY_POLARITY_NEGATIVE) {
            return dy_negative_pairs_are_subtypes(ctx, subtype.pair, supertype.pair, subtype.is_implicit, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
        }

        return dy_positive_pair_is_subtype_of_negative_pair(ctx, subtype.pair, supertype.pair, subtype.is_implicit, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
    case DY_CORE_RECURSION:
        if (subtype.polarity == DY_POLARITY_POSITIVE && supertype.polarity == DY_POLARITY_POSITIVE) {
            return dy_positive_recursions_are_subtypes(ctx, subtype.recursion, supertype.recursion, subtype.is_implicit, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
        }

        if (subtype.polarity == DY_POLARITY_NEGATIVE && supertype.polarity == DY_POLARITY_NEGATIVE) {
            return dy_negative_recursions_are_subtypes(ctx, subtype.recursion, supertype.recursion, subtype.is_implicit, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
        }

        return dy_positive_recursion_is_subtype_of_negative_recursion(ctx, subtype.recursion, supertype.recursion, subtype.is_implicit, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
    }

    dy_bail("Impossible!");
}

/*
 * Implements the following:
 *
 * (x : [v e1] -> e2) <: [v2 e3] -> e4
 *
 * e3 <: e1, f
 *
 * e2 <: e4, g
 *
 * x => [v2 e3] -> g (x (f v2))
 */
dy_ternary_t dy_positive_functions_are_subtypes(struct dy_core_ctx *ctx, struct dy_core_function subtype, struct dy_core_function supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    struct dy_core_expr var_expr = {
        .tag = DY_CORE_EXPR_VARIABLE,
        .variable_id = supertype.id
    };

    size_t constraint_start1 = ctx->constraints.num_elems;

    struct dy_core_expr transformed_var_expr;
    bool did_transform_var_expr = false;
    dy_ternary_t res1 = dy_is_subtype(ctx, *supertype.type, *subtype.type, var_expr, &transformed_var_expr, &did_transform_var_expr);

    if (!did_transform_var_expr) {
        transformed_var_expr = var_expr; // No need to retain here since var_expr doesn't contain pointers.
    }

    if (res1 == DY_NO) {
        dy_core_expr_release(ctx, transformed_var_expr);
        return DY_NO;
    }

    struct dy_core_expr result_type;
    if (!dy_substitute(ctx, *subtype.expr, subtype.id, transformed_var_expr, &result_type)) {
        result_type = dy_core_expr_retain(ctx, *subtype.expr);
    }

    struct dy_core_expr app = {
        .tag = DY_CORE_EXPR_APPLICATION,
        .application = {
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .solution = {
                .tag = DY_CORE_FUNCTION,
                .is_implicit = is_implicit,
                .expr = dy_core_expr_new(transformed_var_expr),
                .out = dy_core_expr_new(dy_core_expr_retain(ctx, result_type))
            },
            .check_result = res1,
            .eval_immediately = true
        }
    };

    size_t constraint_start2 = ctx->constraints.num_elems;

    struct dy_core_expr transformed_app;
    bool did_transform_app = false;
    dy_ternary_t res2 = dy_is_subtype(ctx, result_type, *supertype.expr, app, &transformed_app, &did_transform_app);

    if (!did_transform_app) {
        transformed_app = app;
    } else {
        dy_core_expr_release(ctx, app);
    }

    if (res2 == DY_NO) {
        dy_free_constraints_starting_at(ctx, constraint_start1);
        dy_core_expr_release(ctx, transformed_app);
        return DY_NO;
    }

    dy_join_constraints(ctx, constraint_start1, constraint_start2, DY_POLARITY_POSITIVE);

    if (did_transform_var_expr || did_transform_app) {
        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_PROBLEM,
            .problem = {
                .tag = DY_CORE_FUNCTION,
                .is_implicit = is_implicit,
                .polarity = DY_POLARITY_POSITIVE,
                .function = {
                    .id = supertype.id,
                    .type = dy_core_expr_retain_ptr(ctx, supertype.type),
                    .expr = dy_core_expr_new(transformed_app)
                }
            }
        };

        *did_transform_subtype_expr = true;
    } else {
        dy_core_expr_release(ctx, transformed_app);
    }

    if (res1 == DY_MAYBE || res2 == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

dy_ternary_t dy_negative_functions_are_subtypes(struct dy_core_ctx *ctx, struct dy_core_function subtype, struct dy_core_function supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    struct dy_core_expr var_expr = {
        .tag = DY_CORE_EXPR_VARIABLE,
        .variable_id = supertype.id
    };

    size_t constraint_start1 = ctx->constraints.num_elems;

    struct dy_core_expr transformed_var_expr;
    bool did_transform_var_expr = false;
    dy_ternary_t res1 = dy_is_subtype(ctx, *supertype.type, *subtype.type, var_expr, &transformed_var_expr, &did_transform_var_expr);

    if (!did_transform_var_expr) {
        transformed_var_expr = var_expr; // No need to retain here since var_expr doesn't contain pointers.
    }

    if (res1 == DY_NO) {
        dy_core_expr_release(ctx, transformed_var_expr);
        return DY_NO;
    }

    struct dy_core_expr result_type;
    if (!dy_substitute(ctx, *subtype.expr, subtype.id, transformed_var_expr, &result_type)) {
        result_type = dy_core_expr_retain(ctx, *subtype.expr);
    }

    struct dy_core_expr app = {
        .tag = DY_CORE_EXPR_APPLICATION,
        .application = {
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .solution = {
                .tag = DY_CORE_FUNCTION,
                .is_implicit = is_implicit,
                .expr = dy_core_expr_new(transformed_var_expr),
                .out = dy_core_expr_new(result_type)
            },
            .check_result = DY_MAYBE,
            .eval_immediately = true
        }
    };

    size_t constraint_start2 = ctx->constraints.num_elems;

    struct dy_core_expr transformed_app;
    bool did_transform_app = false;
    dy_ternary_t res2 = dy_is_subtype(ctx, result_type, *supertype.expr, app, &transformed_app, &did_transform_app);

    if (!did_transform_app) {
        transformed_app = app;
    } else {
        dy_core_expr_release(ctx, app);
    }

    if (res2 == DY_NO) {
        dy_core_expr_release(ctx, transformed_app);
        dy_free_constraints_starting_at(ctx, constraint_start1);
        return DY_NO;
    }

    dy_join_constraints(ctx, constraint_start1, constraint_start2, DY_POLARITY_POSITIVE);

    if (did_transform_var_expr || did_transform_app) {
        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_PROBLEM,
            .problem = {
                .tag = DY_CORE_FUNCTION,
                .is_implicit = is_implicit,
                .polarity = DY_POLARITY_POSITIVE,
                .function = {
                    .id = supertype.id,
                    .type = dy_core_expr_retain_ptr(ctx, supertype.type),
                    .expr = dy_core_expr_new(transformed_app)
                }
            }
        };

        *did_transform_subtype_expr = true;
    } else {
        dy_core_expr_release(ctx, transformed_app);
    }

    if (res1 == DY_MAYBE || res2 == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

dy_ternary_t dy_positive_function_is_subtype_of_negative_function(struct dy_core_ctx *ctx, struct dy_core_function subtype, struct dy_core_function supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    struct dy_core_expr var_expr = {
        .tag = DY_CORE_EXPR_VARIABLE,
        .variable_id = supertype.id
    };

    size_t constraint_start1 = ctx->constraints.num_elems;

    struct dy_core_expr transformed_var_expr;
    bool did_transform_var_expr = false;
    dy_ternary_t res1 = dy_is_subtype(ctx, *supertype.type, *subtype.type, var_expr, &transformed_var_expr, &did_transform_var_expr);

    if (!did_transform_var_expr) {
        transformed_var_expr = var_expr; // No need to retain here since var_expr doesn't contain pointers.
    }

    if (res1 == DY_NO) {
        dy_core_expr_release(ctx, transformed_var_expr);
        return DY_NO;
    }

    struct dy_core_expr result_type;
    if (!dy_substitute(ctx, *subtype.expr, subtype.id, transformed_var_expr, &result_type)) {
        result_type = dy_core_expr_retain(ctx, *subtype.expr);
    }

    struct dy_core_expr app = {
        .tag = DY_CORE_EXPR_APPLICATION,
        .application = {
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .solution = {
                .tag = DY_CORE_FUNCTION,
                .is_implicit = is_implicit,
                .expr = dy_core_expr_new(transformed_var_expr),
                .out = dy_core_expr_new(result_type)
            },
            .check_result = res1,
            .eval_immediately = true
        }
    };

    size_t constraint_start2 = ctx->constraints.num_elems;

    struct dy_core_expr transformed_app;
    bool did_transform_app = false;
    dy_ternary_t res2 = dy_is_subtype(ctx, result_type, *supertype.expr, app, &transformed_app, &did_transform_app);

    if (!did_transform_app) {
        transformed_app = app;
    } else {
        dy_core_expr_release(ctx, app);
    }

    if (res2 == DY_NO) {
        dy_core_expr_release(ctx, transformed_app);
        dy_free_constraints_starting_at(ctx, constraint_start1);
        return DY_NO;
    }

    dy_join_constraints(ctx, constraint_start1, constraint_start2, DY_POLARITY_POSITIVE);

    if (did_transform_var_expr || did_transform_app) {
        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_PROBLEM,
            .problem = {
                .tag = DY_CORE_FUNCTION,
                .is_implicit = is_implicit,
                .polarity = DY_POLARITY_POSITIVE,
                .function = {
                    .id = supertype.id,
                    .type = dy_core_expr_retain_ptr(ctx, supertype.type),
                    .expr = dy_core_expr_new(transformed_app)
                }
            }
        };

        *did_transform_subtype_expr = true;
    } else {
        dy_core_expr_release(ctx, transformed_app);
    }

    return DY_YES;
}

dy_ternary_t dy_positive_pairs_are_subtypes(struct dy_core_ctx *ctx, struct dy_core_pair subtype, struct dy_core_pair supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    struct dy_core_expr subtype_expr_left = {
        .tag = DY_CORE_EXPR_APPLICATION,
        .application = {
            .check_result = DY_YES,
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .solution = {
                .is_implicit = is_implicit,
                .tag = DY_CORE_PAIR,
                .direction = DY_LEFT,
                .out = dy_core_expr_retain_ptr(ctx, subtype.left)
            },
            .eval_immediately = true
        }
    };
    size_t constraint_start1 = ctx->constraints.num_elems;

    struct dy_core_expr new_subtype_expr_left;
    bool did_transform_subtype_expr_left = false;
    dy_ternary_t res1 = dy_is_subtype(ctx, *subtype.left, *supertype.left, subtype_expr_left, &new_subtype_expr_left, &did_transform_subtype_expr_left);

    if (!did_transform_subtype_expr_left) {
        new_subtype_expr_left = subtype_expr_left;
    } else {
        dy_core_expr_release(ctx, subtype_expr_left);
    }

    if (res1 == DY_NO) {
        dy_core_expr_release(ctx, new_subtype_expr_left);
        return DY_NO;
    }

    struct dy_core_expr subtype_expr_right = {
        .tag = DY_CORE_EXPR_APPLICATION,
        .application = {
            .check_result = DY_YES,
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .solution = {
                .is_implicit = is_implicit,
                .tag = DY_CORE_PAIR,
                .direction = DY_RIGHT,
                .out = dy_core_expr_retain_ptr(ctx, subtype.right)
            }
        }
    };

    size_t constraint_start2 = ctx->constraints.num_elems;

    struct dy_core_expr new_subtype_expr_right;
    bool did_transform_subtype_expr_right = false;
    dy_ternary_t res2 = dy_is_subtype(ctx, *subtype.right, *supertype.right, subtype_expr_right, &new_subtype_expr_right, &did_transform_subtype_expr_right);

    if (!did_transform_subtype_expr_right) {
        new_subtype_expr_right = subtype_expr_right;
    } else {
        dy_core_expr_release(ctx, subtype_expr_right);
    }

    if (res2 == DY_NO) {
        dy_core_expr_release(ctx, new_subtype_expr_right);
        dy_free_constraints_starting_at(ctx, constraint_start1);
        return DY_NO;
    }

    dy_join_constraints(ctx, constraint_start1, constraint_start2, DY_POLARITY_POSITIVE);

    if (did_transform_subtype_expr_left || did_transform_subtype_expr_right) {
        *did_transform_subtype_expr = true;

        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_PROBLEM,
            .problem = {
                .polarity = DY_POLARITY_POSITIVE,
                .is_implicit = is_implicit,
                .tag = DY_CORE_PAIR,
                .pair = {
                    .left = dy_core_expr_new(new_subtype_expr_left),
                    .right = dy_core_expr_new(new_subtype_expr_right)
                }
            }
        };
    } else {
        dy_core_expr_release(ctx, new_subtype_expr_left);
        dy_core_expr_release(ctx, new_subtype_expr_right);
    }

    if (res1 == DY_MAYBE || res2 == DY_MAYBE) {
        return DY_MAYBE;
    } else {
        return DY_YES;
    }
}

dy_ternary_t dy_negative_pairs_are_subtypes(struct dy_core_ctx *ctx, struct dy_core_pair subtype, struct dy_core_pair supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    size_t left_var_id = ctx->running_id++;

    struct dy_core_expr left_var_id_expr = {
        .tag = DY_CORE_EXPR_VARIABLE,
        .variable_id = left_var_id
    };

    struct dy_core_expr subtype_expr_left = {
        .tag = DY_CORE_EXPR_APPLICATION,
        .application = {
            .check_result = DY_YES,
            .expr = dy_core_expr_new(left_var_id_expr),
            .solution = {
                .is_implicit = is_implicit,
                .tag = DY_CORE_PAIR,
                .direction = DY_LEFT,
                .out = dy_core_expr_retain_ptr(ctx, subtype.left)
            },
            .eval_immediately = true
        }
    };

    size_t constraint_start1 = ctx->constraints.num_elems;

    struct dy_core_expr new_subtype_expr_left;
    bool did_transform_subtype_expr_left = false;
    dy_ternary_t res1 = dy_is_subtype(ctx, *subtype.left, *supertype.left, subtype_expr_left, &new_subtype_expr_left, &did_transform_subtype_expr_left);

    if (!did_transform_subtype_expr_left) {
        new_subtype_expr_left = subtype_expr_left;
    } else {
        dy_core_expr_release(ctx, subtype_expr_left);
    }

    size_t right_var_id = ctx->running_id++;

    struct dy_core_expr right_var_id_expr = {
        .tag = DY_CORE_EXPR_VARIABLE,
        .variable_id = right_var_id
    };

    struct dy_core_expr subtype_expr_right = {
        .tag = DY_CORE_EXPR_APPLICATION,
        .application = {
            .check_result = DY_YES,
            .expr = dy_core_expr_new(right_var_id_expr),
            .solution = {
                .is_implicit = is_implicit,
                .tag = DY_CORE_PAIR,
                .direction = DY_RIGHT,
                .out = dy_core_expr_retain_ptr(ctx, subtype.right)
            },
            .eval_immediately = true
        }
    };

    size_t constraint_start2 = ctx->constraints.num_elems;

    struct dy_core_expr new_subtype_expr_right;
    bool did_transform_subtype_expr_right = false;
    dy_ternary_t res2 = dy_is_subtype(ctx, *subtype.right, *supertype.right, subtype_expr_right, &new_subtype_expr_right, &did_transform_subtype_expr_right);

    if (!did_transform_subtype_expr_right) {
        new_subtype_expr_right = subtype_expr_right;
    } else {
        dy_core_expr_release(ctx, subtype_expr_right);
    }

    dy_join_constraints(ctx, constraint_start1, constraint_start2, DY_POLARITY_NEGATIVE);

    if (!did_transform_subtype_expr_left || !did_transform_subtype_expr_right) {
        struct dy_core_expr left_subtype_type = {
            .tag = DY_CORE_EXPR_SOLUTION,
            .solution = {
                .is_implicit = false,
                .tag = DY_CORE_PAIR,
                .direction = DY_LEFT,
                .out = dy_core_expr_retain_ptr(ctx, subtype.left)
            }
        };

        struct dy_core_expr left_subtype_expr = {
            .tag = DY_CORE_EXPR_SOLUTION,
            .solution = {
                .is_implicit = false,
                .tag = DY_CORE_PAIR,
                .direction = DY_LEFT,
                .out = dy_core_expr_new(new_subtype_expr_left)
            }
        };

        struct dy_core_expr left_fun = {
            .tag = DY_CORE_EXPR_PROBLEM,
            .problem = {
                .is_implicit = false,
                .polarity = DY_POLARITY_POSITIVE,
                .tag = DY_CORE_FUNCTION,
                .function = {
                    .id = left_var_id,
                    .type = dy_core_expr_new(left_subtype_type),
                    .expr = dy_core_expr_new(left_subtype_expr)
                }
            }
        };

        struct dy_core_expr right_subtype_type = {
            .tag = DY_CORE_EXPR_SOLUTION,
            .solution = {
                .is_implicit = false,
                .tag = DY_CORE_PAIR,
                .direction = DY_RIGHT,
                .out = dy_core_expr_retain_ptr(ctx, subtype.right)
            }
        };

        struct dy_core_expr right_subtype_expr = {
            .tag = DY_CORE_EXPR_SOLUTION,
            .solution = {
                .is_implicit = false,
                .tag = DY_CORE_PAIR,
                .direction = DY_RIGHT,
                .out = dy_core_expr_new(new_subtype_expr_right)
            }
        };

        struct dy_core_expr right_fun = {
            .tag = DY_CORE_EXPR_PROBLEM,
            .problem = {
                .is_implicit = false,
                .polarity = DY_POLARITY_POSITIVE,
                .tag = DY_CORE_FUNCTION,
                .function = {
                    .id = right_var_id,
                    .type = dy_core_expr_new(right_subtype_type),
                    .expr = dy_core_expr_new(right_subtype_expr)
                }
            }
        };

        struct dy_core_expr functions = {
            .tag = DY_CORE_EXPR_PROBLEM,
            .problem = {
                .is_implicit = true,
                .polarity = DY_POLARITY_POSITIVE,
                .tag = DY_CORE_PAIR,
                .pair = {
                    .left = dy_core_expr_new(left_fun),
                    .right = dy_core_expr_new(right_fun)
                }
            }
        };

        struct dy_core_expr supertype_expr = {
            .tag = DY_CORE_EXPR_PROBLEM,
            .problem = {
                .is_implicit = is_implicit,
                .polarity = DY_POLARITY_NEGATIVE,
                .tag = DY_CORE_PAIR,
                .pair = {
                    .left = dy_core_expr_retain_ptr(ctx, supertype.left),
                    .right = dy_core_expr_retain_ptr(ctx, supertype.right)
                }
            }
        };

        struct dy_core_expr app = {
            .tag = DY_CORE_EXPR_APPLICATION,
            .application = {
                .check_result = DY_MAYBE,
                .expr = dy_core_expr_new(dy_core_expr_retain(ctx, functions)),
                .solution = {
                    .is_implicit = false,
                    .tag = DY_CORE_FUNCTION,
                    .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
                    .out = dy_core_expr_new(supertype_expr)
                },
                .eval_immediately = true
            }
        };

        *new_subtype_expr = app;
        *did_transform_subtype_expr = true;
    } else {
        dy_core_expr_release(ctx, new_subtype_expr_left);
        dy_core_expr_release(ctx, new_subtype_expr_right);
    }

    if (did_transform_subtype_expr_left || did_transform_subtype_expr_right) {
        *did_transform_subtype_expr = true;

        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_PROBLEM,
            .problem = {
                .polarity = DY_POLARITY_POSITIVE,
                .is_implicit = is_implicit,
                .tag = DY_CORE_PAIR,
                .pair = {
                    .left = dy_core_expr_new(new_subtype_expr_left),
                    .right = dy_core_expr_new(new_subtype_expr_right)
                }
            }
        };
    } else {
        dy_core_expr_release(ctx, new_subtype_expr_left);
        dy_core_expr_release(ctx, new_subtype_expr_right);
    }

    if (res1 == DY_NO && res2 == DY_NO) {
        return DY_NO;
    }

    if (res1 == DY_MAYBE && res2 == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

dy_ternary_t dy_positive_pair_is_subtype_of_negative_pair(struct dy_core_ctx *ctx, struct dy_core_pair subtype, struct dy_core_pair supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    struct dy_core_expr subtype_expr_left = {
        .tag = DY_CORE_EXPR_APPLICATION,
        .application = {
            .check_result = DY_YES,
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .solution = {
                .is_implicit = is_implicit,
                .tag = DY_CORE_PAIR,
                .direction = DY_LEFT,
                .out = dy_core_expr_retain_ptr(ctx, subtype.left)
            },
            .eval_immediately = true
        }
    };


    size_t constraint_start1 = ctx->constraints.num_elems;

    struct dy_core_expr new_subtype_expr_left;
    bool did_transform_subtype_expr_left = false;
    dy_ternary_t res1 = dy_is_subtype(ctx, *subtype.left, *supertype.left, subtype_expr_left, &new_subtype_expr_left, &did_transform_subtype_expr_left);

    if (!did_transform_subtype_expr_left) {
        new_subtype_expr_left = subtype_expr_left;
    } else {
        dy_core_expr_release(ctx, subtype_expr_left);
    }

    struct dy_core_expr subtype_expr_right = {
        .tag = DY_CORE_EXPR_APPLICATION,
        .application = {
            .check_result = DY_YES,
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .solution = {
                .is_implicit = is_implicit,
                .tag = DY_CORE_PAIR,
                .direction = DY_RIGHT,
                .out = dy_core_expr_retain_ptr(ctx, subtype.right)
            },
            .eval_immediately = true
        }
    };


    size_t constraint_start2 = ctx->constraints.num_elems;

    struct dy_core_expr new_subtype_expr_right;
    bool did_transform_subtype_expr_right = false;
    dy_ternary_t res2 = dy_is_subtype(ctx, *subtype.right, *supertype.right, subtype_expr_right, &new_subtype_expr_right, &did_transform_subtype_expr_right);

    if (!did_transform_subtype_expr_right) {
        new_subtype_expr_right = subtype_expr_right;
    } else {
        dy_core_expr_release(ctx, subtype_expr_right);
    }

    dy_join_constraints(ctx, constraint_start1, constraint_start2, DY_POLARITY_NEGATIVE);

    if (did_transform_subtype_expr_left || did_transform_subtype_expr_right) {
        *did_transform_subtype_expr = true;

        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_PROBLEM,
            .problem = {
                .polarity = DY_POLARITY_POSITIVE,
                .is_implicit = is_implicit,
                .tag = DY_CORE_PAIR,
                .pair = {
                    .left = dy_core_expr_new(new_subtype_expr_left),
                    .right = dy_core_expr_new(new_subtype_expr_right)
                }
            }
        };
    } else {
        dy_core_expr_release(ctx, new_subtype_expr_left);
        dy_core_expr_release(ctx, new_subtype_expr_right);
    }

    if (res1 == DY_NO && res2 == DY_NO) {
        return DY_NO;
    }

    if (res1 == DY_MAYBE && res2 == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

dy_ternary_t dy_positive_recursions_are_subtypes(struct dy_core_ctx *ctx, struct dy_core_recursion subtype, struct dy_core_recursion supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    for (size_t i = 0; i < ctx->past_subtype_checks.num_elems; ++i) {
        const struct dy_core_past_subtype_check *past_check = dy_array_pos(&ctx->past_subtype_checks, i);
        if (dy_are_equal(ctx, past_check->subtype, *subtype.expr) == DY_YES && dy_are_equal(ctx, past_check->supertype, *supertype.expr) == DY_YES) {
            if (past_check->have_substitute_var_id) {
                *new_subtype_expr = (struct dy_core_expr){
                    .tag = DY_CORE_EXPR_VARIABLE,
                    .variable_id = past_check->substitute_var_id
                };
                *did_transform_subtype_expr = true;
                return DY_YES; // Not sure if correct here
            } else {
                return DY_NO;
            }
        }
    }

    struct dy_core_expr subtype_wrap = {
        .tag = DY_CORE_EXPR_PROBLEM,
        .problem = {
            .is_implicit = is_implicit,
            .polarity = DY_POLARITY_POSITIVE,
            .tag = DY_CORE_RECURSION,
            .recursion = subtype
        }
    };

    struct dy_core_expr unfolded_subtype;
    if (!dy_substitute(ctx, *subtype.expr, subtype.id, subtype_wrap, &unfolded_subtype)) {
        unfolded_subtype = dy_core_expr_retain(ctx, subtype_wrap);
    }

    struct dy_core_expr unfold = {
        .tag = DY_CORE_EXPR_APPLICATION,
        .application = {
            .check_result = DY_YES,
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .solution = {
                .is_implicit = is_implicit,
                .tag = DY_CORE_RECURSION,
                .out = dy_core_expr_new(dy_core_expr_retain(ctx, unfolded_subtype))
            },
            .eval_immediately = true
        }
    };


    struct dy_core_expr supertype_wrap = {
        .tag = DY_CORE_EXPR_PROBLEM,
        .problem = {
            .is_implicit = is_implicit,
            .polarity = DY_POLARITY_POSITIVE,
            .tag = DY_CORE_RECURSION,
            .recursion = supertype
        }
    };

    struct dy_core_expr unfolded_supertype;
    if (!dy_substitute(ctx, *supertype.expr, supertype.id, supertype_wrap, &unfolded_supertype)) {
        unfolded_supertype = dy_core_expr_retain(ctx, supertype_wrap);
    }

    dy_array_add(&ctx->past_subtype_checks, &(struct dy_core_past_subtype_check){
        .subtype = *subtype.expr,
        .supertype = *supertype.expr,
        .substitute_var_id = supertype.id,
        .have_substitute_var_id = true
    });

    struct dy_core_expr transformed_unfold;
    bool did_transform_unfold = false;
    dy_ternary_t res = dy_is_subtype(ctx, unfolded_subtype, unfolded_supertype, unfold, &transformed_unfold, &did_transform_unfold);

    ctx->past_subtype_checks.num_elems--;

    dy_core_expr_release(ctx, unfold);

    if (did_transform_unfold) {
        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_PROBLEM,
            .problem = {
                .is_implicit = is_implicit,
                .polarity = DY_POLARITY_POSITIVE,
                .tag = DY_CORE_RECURSION,
                .recursion = {
                    .id = supertype.id,
                    .expr = dy_core_expr_new(transformed_unfold)
                }
            }
        };

        *did_transform_subtype_expr = true;
    }

    return res;
}

dy_ternary_t dy_negative_recursions_are_subtypes(struct dy_core_ctx *ctx, struct dy_core_recursion subtype, struct dy_core_recursion supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    for (size_t i = 0; i < ctx->past_subtype_checks.num_elems; ++i) {
        const struct dy_core_past_subtype_check *past_check = dy_array_pos(&ctx->past_subtype_checks, i);
        if (dy_are_equal(ctx, past_check->subtype, *subtype.expr) == DY_YES && dy_are_equal(ctx, past_check->supertype, *supertype.expr) == DY_YES) {
            if (past_check->have_substitute_var_id) {
                *new_subtype_expr = (struct dy_core_expr){
                    .tag = DY_CORE_EXPR_VARIABLE,
                    .variable_id = past_check->substitute_var_id
                };
                *did_transform_subtype_expr = true;
                return DY_YES; // Not sure if correct here
            } else {
                return DY_NO;
            }
        }
    }

    struct dy_core_expr subtype_wrap = {
        .tag = DY_CORE_EXPR_PROBLEM,
        .problem = {
            .is_implicit = is_implicit,
            .polarity = DY_POLARITY_NEGATIVE,
            .tag = DY_CORE_RECURSION,
            .recursion = subtype
        }
    };

    struct dy_core_expr unfolded_subtype;
    if (!dy_substitute(ctx, *subtype.expr, subtype.id, subtype_wrap, &unfolded_subtype)) {
        unfolded_subtype = dy_core_expr_retain(ctx, subtype_wrap);
    }

    struct dy_core_expr unfold = {
        .tag = DY_CORE_EXPR_APPLICATION,
        .application = {
            .check_result = DY_MAYBE,
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .solution = {
                .is_implicit = is_implicit,
                .tag = DY_CORE_RECURSION,
                .out = dy_core_expr_new(unfolded_subtype)
            },
            .eval_immediately = true
        }
    };

    struct dy_core_expr supertype_wrap = {
        .tag = DY_CORE_EXPR_PROBLEM,
        .problem = {
            .is_implicit = is_implicit,
            .polarity = DY_POLARITY_NEGATIVE,
            .tag = DY_CORE_RECURSION,
            .recursion = supertype
        }
    };

    struct dy_core_expr unfolded_supertype;
    if (!dy_substitute(ctx, *supertype.expr, supertype.id, supertype_wrap, &unfolded_supertype)) {
        unfolded_supertype = dy_core_expr_retain(ctx, supertype_wrap);
    }

    dy_array_add(&ctx->past_subtype_checks, &(struct dy_core_past_subtype_check){
        .subtype = *subtype.expr,
        .supertype = *supertype.expr,
        .substitute_var_id = supertype.id,
        .have_substitute_var_id = true
    });

    struct dy_core_expr transformed_unfold;
    bool did_transform_unfold = false;
    dy_ternary_t res = dy_is_subtype(ctx, unfolded_subtype, unfolded_supertype, unfold, &transformed_unfold, &did_transform_unfold);

    ctx->past_subtype_checks.num_elems--;

    dy_core_expr_release(ctx, unfold);

    if (did_transform_unfold) {
        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_PROBLEM,
            .problem = {
                .is_implicit = is_implicit,
                .polarity = DY_POLARITY_POSITIVE,
                .tag = DY_CORE_RECURSION,
                .recursion = {
                    .id = supertype.id,
                    .expr = dy_core_expr_new(transformed_unfold)
                }
            }
        };

        *did_transform_subtype_expr = true;
    }

    return res;
}

dy_ternary_t dy_positive_recursion_is_subtype_of_negative_recursion(struct dy_core_ctx *ctx, struct dy_core_recursion subtype, struct dy_core_recursion supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    for (size_t i = 0; i < ctx->past_subtype_checks.num_elems; ++i) {
        const struct dy_core_past_subtype_check *past_check = dy_array_pos(&ctx->past_subtype_checks, i);
        if (dy_are_equal(ctx, past_check->subtype, *subtype.expr) == DY_YES && dy_are_equal(ctx, past_check->supertype, *supertype.expr) == DY_YES) {
            if (past_check->have_substitute_var_id) {
                *new_subtype_expr = (struct dy_core_expr){
                    .tag = DY_CORE_EXPR_VARIABLE,
                    .variable_id = past_check->substitute_var_id
                };
                *did_transform_subtype_expr = true;
                return DY_YES; // Not sure if correct here
            } else {
                return DY_NO;
            }
        }
    }

    struct dy_core_expr subtype_wrap = {
        .tag = DY_CORE_EXPR_PROBLEM,
        .problem = {
            .is_implicit = is_implicit,
            .polarity = DY_POLARITY_POSITIVE,
            .tag = DY_CORE_RECURSION,
            .recursion = subtype
        }
    };

    struct dy_core_expr unfolded_subtype;
    if (!dy_substitute(ctx, *subtype.expr, subtype.id, subtype_wrap, &unfolded_subtype)) {
        unfolded_subtype = dy_core_expr_retain(ctx, subtype_wrap);
    }

    struct dy_core_expr unfold = {
        .tag = DY_CORE_EXPR_APPLICATION,
        .application = {
            .check_result = DY_YES,
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .solution = {
                .is_implicit = is_implicit,
                .tag = DY_CORE_RECURSION,
                .out = dy_core_expr_new(unfolded_subtype)
            },
            .eval_immediately = true
        }
    };


    struct dy_core_expr supertype_wrap = {
        .tag = DY_CORE_EXPR_PROBLEM,
        .problem = {
            .is_implicit = is_implicit,
            .polarity = DY_POLARITY_NEGATIVE,
            .tag = DY_CORE_RECURSION,
            .recursion = supertype
        }
    };

    struct dy_core_expr unfolded_supertype;
    if (!dy_substitute(ctx, *supertype.expr, supertype.id, supertype_wrap, &unfolded_supertype)) {
        unfolded_supertype = dy_core_expr_retain(ctx, supertype_wrap);
    }

    dy_array_add(&ctx->past_subtype_checks, &(struct dy_core_past_subtype_check){
        .subtype = *subtype.expr,
        .supertype = *supertype.expr,
        .substitute_var_id = supertype.id,
        .have_substitute_var_id = true
    });

    struct dy_core_expr transformed_unfold;
    bool did_transform_unfold = false;
    dy_ternary_t res = dy_is_subtype(ctx, unfolded_subtype, unfolded_supertype, unfold, &transformed_unfold, &did_transform_unfold);

    ctx->past_subtype_checks.num_elems--;

    dy_core_expr_release(ctx, unfold);

    if (did_transform_unfold) {
        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_PROBLEM,
            .problem = {
                .is_implicit = is_implicit,
                .polarity = DY_POLARITY_POSITIVE,
                .tag = DY_CORE_RECURSION,
                .recursion = {
                    .id = supertype.id,
                    .expr = dy_core_expr_new(transformed_unfold)
                }
            }
        };

        *did_transform_subtype_expr = true;
    }

    return res;
}

dy_ternary_t dy_function_is_subtype_of_solution(struct dy_core_ctx *ctx, struct dy_core_function subtype, struct dy_core_solution supertype, bool is_implicit, enum dy_polarity polarity, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    struct dy_core_expr type_of_arg = dy_type_of(ctx, *supertype.expr);

    size_t constraint_start1 = ctx->constraints.num_elems;

    struct dy_core_expr transformed_supertype_expr;
    bool did_transform_supertype_expr = false;
    dy_ternary_t res1 = dy_is_subtype(ctx, type_of_arg, *subtype.type, *supertype.expr, &transformed_supertype_expr, &did_transform_supertype_expr);

    if (!did_transform_supertype_expr) {
        transformed_supertype_expr = dy_core_expr_retain(ctx, *supertype.expr);
    }

    if (res1 == DY_NO) {
        dy_core_expr_release(ctx, transformed_supertype_expr);
        return DY_NO;
    }

    struct dy_core_expr subst_subtype;
    if (!dy_substitute(ctx, *subtype.expr, subtype.id, transformed_supertype_expr, &subst_subtype)) {
        subst_subtype = dy_core_expr_retain(ctx, *subtype.expr);
    }

    struct dy_core_expr app = {
        .tag = DY_CORE_EXPR_APPLICATION,
        .application = {
            .check_result = polarity == DY_POLARITY_NEGATIVE ? DY_MAYBE : res1,
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .solution = {
                .is_implicit = is_implicit,
                .tag = DY_CORE_FUNCTION,
                .expr = dy_core_expr_new(transformed_supertype_expr),
                .out = dy_core_expr_new(dy_core_expr_retain(ctx, subst_subtype))
            },
            .eval_immediately = true
        }
    };


    size_t constraint_start2 = ctx->constraints.num_elems;

    struct dy_core_expr transformed_app;
    bool did_transform_app = false;
    dy_ternary_t res2 = dy_is_subtype(ctx, subst_subtype, *supertype.out, app, &transformed_app, &did_transform_app);

    dy_core_expr_release(ctx, subst_subtype);

    if (!did_transform_app) {
        transformed_app = app;
    } else {
        dy_core_expr_release(ctx, app);
    }

    if (res2 == DY_NO) {
        dy_core_expr_release(ctx, transformed_app);
        dy_free_constraints_starting_at(ctx, constraint_start1);
        return DY_NO;
    }

    dy_join_constraints(ctx, constraint_start1, constraint_start2, DY_POLARITY_POSITIVE);

    if (did_transform_supertype_expr || did_transform_app) {
        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_SOLUTION,
            .solution = {
                .is_implicit = is_implicit,
                .tag = DY_CORE_FUNCTION,
                .expr = dy_core_expr_retain_ptr(ctx, supertype.expr),
                .out = dy_core_expr_new(transformed_app)
            }
        };
        *did_transform_subtype_expr = true;
    } else {
        dy_core_expr_release(ctx, transformed_app);
    }

    if (res1 == DY_MAYBE || res2 == DY_MAYBE || polarity == DY_POLARITY_NEGATIVE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

dy_ternary_t dy_pair_is_subtype_of_solution(struct dy_core_ctx *ctx, struct dy_core_pair subtype, struct dy_core_solution supertype, bool is_implicit, enum dy_polarity polarity, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    if (supertype.direction == DY_LEFT) {
        struct dy_core_expr app = {
            .tag = DY_CORE_EXPR_APPLICATION,
            .application = {
                .check_result = polarity == DY_POLARITY_NEGATIVE ? DY_MAYBE : DY_YES,
                .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
                .solution = {
                    .is_implicit = is_implicit,
                    .tag = DY_CORE_PAIR,
                    .direction = DY_LEFT,
                    .out = dy_core_expr_retain_ptr(ctx, subtype.left)
                },
                .eval_immediately = true
            }
        };

        struct dy_core_expr transformed_app;
        bool did_transform_app = false;
        dy_ternary_t res = dy_is_subtype(ctx, *subtype.left, *supertype.out, app, &transformed_app, &did_transform_app);

        if (did_transform_app) {
            *new_subtype_expr = (struct dy_core_expr){
                .tag = DY_CORE_EXPR_SOLUTION,
                .solution = {
                    .is_implicit = is_implicit,
                    .tag = DY_CORE_PAIR,
                    .direction = DY_LEFT,
                    .out = dy_core_expr_new(transformed_app)
                }
            };
            *did_transform_subtype_expr = true;
        } else {
            dy_core_expr_release(ctx, app);
        }

        if (res == DY_YES && polarity == DY_POLARITY_NEGATIVE) {
            return DY_MAYBE;
        } else {
            return res;
        }
    } else {
        struct dy_core_expr app = {
            .tag = DY_CORE_EXPR_APPLICATION,
            .application = {
                .check_result = polarity == DY_POLARITY_NEGATIVE ? DY_MAYBE : DY_YES,
                .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
                .solution = {
                    .is_implicit = is_implicit,
                    .tag = DY_CORE_PAIR,
                    .direction = DY_RIGHT,
                    .out = dy_core_expr_retain_ptr(ctx, subtype.right)
                },
                .eval_immediately = true
            }
        };

        struct dy_core_expr transformed_app;
        bool did_transform_app = false;
        dy_ternary_t res = dy_is_subtype(ctx, *subtype.right, *supertype.out, app, &transformed_app, &did_transform_app);

        if (did_transform_app) {
            *new_subtype_expr = (struct dy_core_expr){
                .tag = DY_CORE_EXPR_SOLUTION,
                .solution = {
                    .is_implicit = is_implicit,
                    .tag = DY_CORE_PAIR,
                    .direction = DY_RIGHT,
                    .out = dy_core_expr_new(transformed_app)
                }
            };
            *did_transform_subtype_expr = true;
        } else {
            dy_core_expr_release(ctx, app);
        }

        if (res == DY_YES && polarity == DY_POLARITY_NEGATIVE) {
            return DY_MAYBE;
        } else {
            return res;
        }
    }
}

dy_ternary_t dy_recursion_is_subtype_of_solution(struct dy_core_ctx *ctx, struct dy_core_recursion subtype, struct dy_core_solution supertype, bool is_implicit, enum dy_polarity polarity, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    struct dy_core_expr subtype_wrap = {
        .tag = DY_CORE_EXPR_PROBLEM,
        .problem = {
            .is_implicit = is_implicit,
            .polarity = polarity,
            .tag = DY_CORE_RECURSION,
            .recursion = subtype
        }
    };

    struct dy_core_expr unfolded_subtype;
    if (!dy_substitute(ctx, *subtype.expr, subtype.id, subtype_wrap, &unfolded_subtype)) {
        unfolded_subtype = dy_core_expr_retain(ctx, subtype_wrap);
    }

    struct dy_core_expr unfold = {
        .tag = DY_CORE_EXPR_APPLICATION,
        .application = {
            .check_result = polarity == DY_POLARITY_POSITIVE ? DY_YES : DY_MAYBE,
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .solution = {
                .is_implicit = is_implicit,
                .tag = DY_CORE_RECURSION,
                .out = dy_core_expr_new(unfolded_subtype)
            },
            .eval_immediately = true
        }
    };

    struct dy_core_expr transformed_unfold;
    bool did_transform_unfold = false;
    dy_ternary_t res = dy_is_subtype(ctx, unfolded_subtype, *supertype.out, unfold, &transformed_unfold, &did_transform_unfold);

    if (did_transform_unfold) {
        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_SOLUTION,
            .solution = {
                .is_implicit = is_implicit,
                .tag = DY_CORE_RECURSION,
                .out = dy_core_expr_new(unfold)
            }
        };
        *did_transform_subtype_expr = true;
    } else {
        dy_core_expr_release(ctx, unfold);
    }

    if (res == DY_YES && polarity == DY_POLARITY_NEGATIVE) {
        return DY_MAYBE;
    } else {
        return DY_YES;
    }
}

dy_ternary_t dy_solution_is_subtype_of_function(struct dy_core_ctx *ctx, struct dy_core_solution subtype, struct dy_core_function supertype, struct dy_core_expr subtype_expr, bool is_implicit, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    struct dy_core_expr type_of_subtype_arg = dy_type_of(ctx, *subtype.expr);

    size_t constraint_start1 = ctx->constraints.num_elems;

    struct dy_core_expr transformed_subtype_arg;
    bool did_transform_subtype_arg = false;
    dy_ternary_t res1 = dy_is_subtype(ctx, type_of_subtype_arg, *supertype.type, *subtype.expr, &transformed_subtype_arg, &did_transform_subtype_arg);

    if (!did_transform_subtype_arg) {
        transformed_subtype_arg = dy_core_expr_retain(ctx, *subtype.expr);
    }

    if (res1 == DY_NO) {
        dy_core_expr_release(ctx, transformed_subtype_arg);
        return DY_NO;
    }

    struct dy_core_expr subs_fun_expr;
    if (!dy_substitute(ctx, *supertype.expr, supertype.id, transformed_subtype_arg, &subs_fun_expr)) {
        subs_fun_expr = dy_core_expr_retain(ctx, *supertype.expr);
    }

    struct dy_core_expr app = {
        .tag = DY_CORE_EXPR_APPLICATION,
        .application = {
            .check_result = res1,
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .solution = {
                .is_implicit = is_implicit,
                .tag = DY_CORE_FUNCTION,
                .expr = dy_core_expr_retain_ptr(ctx, subtype.expr),
                .out = dy_core_expr_retain_ptr(ctx, subtype.out)
            },
            .eval_immediately = true
        }
    };

    size_t constraint_start2 = ctx->constraints.num_elems;

    struct dy_core_expr transformed_expr;
    bool did_transform_expr = false;
    dy_ternary_t res2 = dy_is_subtype(ctx, app, subs_fun_expr, *subtype.out, &transformed_expr, &did_transform_expr);

    if (!did_transform_expr) {
        transformed_expr = dy_core_expr_retain(ctx, app);
    } else {
        dy_core_expr_release(ctx, app);
    }

    if (res2 == DY_NO) {
        dy_core_expr_release(ctx, transformed_expr);
        dy_free_constraints_starting_at(ctx, constraint_start1);
        return DY_NO;
    }

    dy_join_constraints(ctx, constraint_start1, constraint_start2, DY_POLARITY_POSITIVE);

    if (did_transform_subtype_arg || did_transform_expr) {
        *did_transform_subtype_expr = true;
        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_SOLUTION,
            .solution = {
                .is_implicit = is_implicit,
                .tag = DY_CORE_FUNCTION,
                .expr = dy_core_expr_new(transformed_subtype_arg),
                .out = dy_core_expr_new(transformed_expr)
            }
        };
    } else {
        dy_core_expr_release(ctx, transformed_subtype_arg);
        dy_core_expr_release(ctx, transformed_expr);
    }

    if (res1 == DY_MAYBE || res2 == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

dy_ternary_t dy_solution_is_subtype_of_pair(struct dy_core_ctx *ctx, struct dy_core_solution subtype, struct dy_core_pair supertype, struct dy_core_expr subtype_expr, bool is_implicit, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    if (subtype.direction == DY_LEFT) {
        struct dy_core_expr app = {
            .tag = DY_CORE_EXPR_APPLICATION,
            .application = {
                .check_result = DY_YES,
                .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
                .solution = {
                    .is_implicit = is_implicit,
                    .tag = DY_CORE_PAIR,
                    .direction = DY_LEFT,
                    .out = dy_core_expr_retain_ptr(ctx, subtype.out)
                },
                .eval_immediately = true
            }
        };


        struct dy_core_expr transformed_app;
        bool did_transform_app = false;
        dy_ternary_t res = dy_is_subtype(ctx, *subtype.out, *supertype.left, app, &transformed_app, &did_transform_app);

        dy_core_expr_release(ctx, app);

        if (did_transform_app) {
            *did_transform_subtype_expr = true;
            *new_subtype_expr = (struct dy_core_expr){
                .tag = DY_CORE_EXPR_SOLUTION,
                .solution = {
                    .is_implicit = is_implicit,
                    .tag = DY_CORE_PAIR,
                    .direction = DY_LEFT,
                    .out = dy_core_expr_new(transformed_app)
                }
            };
        }

        return res;
    } else {
        struct dy_core_expr app = {
            .tag = DY_CORE_EXPR_APPLICATION,
            .application = {
                .check_result = DY_YES,
                .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
                .solution = {
                    .is_implicit = is_implicit,
                    .tag = DY_CORE_PAIR,
                    .direction = DY_RIGHT,
                    .out = dy_core_expr_retain_ptr(ctx, subtype.out)
                },
                .eval_immediately = true
            }
        };


        struct dy_core_expr transformed_app;
        bool did_transform_app = false;
        dy_ternary_t res = dy_is_subtype(ctx, *subtype.out, *supertype.left, app, &transformed_app, &did_transform_app);

        dy_core_expr_release(ctx, app);

        if (did_transform_app) {
            *did_transform_subtype_expr = true;
            *new_subtype_expr = (struct dy_core_expr){
                .tag = DY_CORE_EXPR_SOLUTION,
                .solution = {
                    .is_implicit = is_implicit,
                    .tag = DY_CORE_PAIR,
                    .direction = DY_RIGHT,
                    .out = dy_core_expr_new(transformed_app)
                }
            };
        }

        return res;
    }
}

dy_ternary_t dy_solution_is_subtype_of_recursion(struct dy_core_ctx *ctx, struct dy_core_solution subtype, struct dy_core_recursion supertype, struct dy_core_expr subtype_expr, bool is_implicit, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    struct dy_core_expr app = {
        .tag = DY_CORE_EXPR_APPLICATION,
        .application = {
            .check_result = DY_YES,
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .solution = {
                .is_implicit = is_implicit,
                .tag = DY_CORE_RECURSION,
                .out = dy_core_expr_retain_ptr(ctx, subtype.out)
            },
            .eval_immediately = true
        }
    };


    struct dy_core_expr supertype_wrap = {
        .tag = DY_CORE_EXPR_PROBLEM,
        .problem = {
            .polarity = DY_POLARITY_NEGATIVE,
            .is_implicit = is_implicit,
            .tag = DY_CORE_RECURSION,
            .recursion = supertype
        }
    };

    struct dy_core_expr unfolded_type;
    if (!dy_substitute(ctx, *supertype.expr, supertype.id, supertype_wrap, &unfolded_type)) {
        unfolded_type = dy_core_expr_retain(ctx, supertype_wrap);
    }

    struct dy_core_expr transformed_app;
    bool did_transform_app = false;
    dy_ternary_t res = dy_is_subtype(ctx, *subtype.out, unfolded_type, app, &transformed_app, &did_transform_app);

    dy_core_expr_release(ctx, app);

    if (did_transform_app) {
        *did_transform_subtype_expr = true;
        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_SOLUTION,
            .solution = {
                .is_implicit = is_implicit,
                .tag = DY_CORE_RECURSION,
                .out = dy_core_expr_new(transformed_app)
            }
        };
    }

    return res;
}

dy_ternary_t dy_function_solution_is_subtype_of_solution(struct dy_core_ctx *ctx, struct dy_core_solution subtype, struct dy_core_solution supertype, struct dy_core_expr subtype_expr, bool is_implicit, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    dy_ternary_t res1 = dy_are_equal(ctx, *subtype.expr, *supertype.expr);
    if (res1 == DY_NO) {
        return DY_NO;
    }

    struct dy_core_expr app = {
        .tag = DY_CORE_EXPR_APPLICATION,
        .application = {
            .check_result = res1,
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .solution = {
                .is_implicit = is_implicit,
                .tag = DY_CORE_FUNCTION,
                .expr = dy_core_expr_retain_ptr(ctx, subtype.expr),
                .out = dy_core_expr_retain_ptr(ctx, subtype.out)
            },
            .eval_immediately = true
        }
    };


    struct dy_core_expr transformed_app;
    bool did_transform_app = false;
    dy_ternary_t res2 = dy_is_subtype(ctx, *subtype.out, *supertype.out, app, &transformed_app, &did_transform_app);
    if (res2 == DY_NO) {
        return DY_NO;
    }

    dy_core_expr_release(ctx, app);

    if (did_transform_app) {
        *did_transform_subtype_expr = true;
        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_SOLUTION,
            .solution = {
                .is_implicit = is_implicit,
                .tag = DY_CORE_FUNCTION,
                .expr = dy_core_expr_retain_ptr(ctx, subtype.expr),
                .out = dy_core_expr_new(transformed_app)
            }
        };
    }

    if (res1 == DY_MAYBE || res2 == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

dy_ternary_t dy_pair_solution_is_subtype_of_solution(struct dy_core_ctx *ctx, struct dy_core_solution subtype, struct dy_core_solution supertype, struct dy_core_expr subtype_expr, bool is_implicit, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    if (subtype.direction != supertype.direction) {
        return DY_NO;
    }

    struct dy_core_expr app = {
        .tag = DY_CORE_EXPR_APPLICATION,
        .application = {
            .check_result = DY_YES,
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .solution = {
                .is_implicit = is_implicit,
                .tag = DY_CORE_PAIR,
                .direction = subtype.direction,
                .out = dy_core_expr_retain_ptr(ctx, subtype.out)
            },
            .eval_immediately = true
        }
    };


    struct dy_core_expr transformed_app;
    bool did_transform_app = false;
    dy_ternary_t res = dy_is_subtype(ctx, *subtype.out, *supertype.out, app, &transformed_app, &did_transform_app);

    dy_core_expr_release(ctx, app);

    if (did_transform_app) {
        *did_transform_subtype_expr = true;
        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_SOLUTION,
            .solution = {
                .is_implicit = is_implicit,
                .tag = DY_CORE_PAIR,
                .direction = subtype.direction,
                .out = dy_core_expr_new(transformed_app)
            }
        };
    }

    return res;
}

dy_ternary_t dy_recursion_solution_is_subtype_of_solution(struct dy_core_ctx *ctx, struct dy_core_solution subtype, struct dy_core_solution supertype, struct dy_core_expr subtype_expr, bool is_implicit, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    struct dy_core_expr app = {
        .tag = DY_CORE_EXPR_APPLICATION,
        .application = {
            .check_result = DY_YES,
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .solution = {
                .is_implicit = is_implicit,
                .tag = DY_CORE_RECURSION,
                .out = dy_core_expr_retain_ptr(ctx, subtype.out)
            },
            .eval_immediately = true
        }
    };


    struct dy_core_expr transformed_app;
    bool did_transform_app = false;
    dy_ternary_t res = dy_is_subtype(ctx, *subtype.out, *supertype.out, app, &transformed_app, &did_transform_app);

    if (did_transform_app) {
        *did_transform_subtype_expr = true;
        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_SOLUTION,
            .solution = {
                .is_implicit = is_implicit,
                .tag = DY_CORE_RECURSION,
                .out = dy_core_expr_new(transformed_app)
            }
        };
    }

    return res;
}

dy_ternary_t dy_positive_implicit_function_is_subtype(struct dy_core_ctx *ctx, struct dy_core_function subtype, struct dy_core_expr supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    size_t inference_id = ctx->running_id++;

    dy_array_add(&ctx->recovered_negative_inference_ids, &inference_id);

    struct dy_core_expr inference_id_expr = {
        .tag = DY_CORE_EXPR_INFERENCE_VAR,
        .inference_var_id = inference_id
    };

    struct dy_core_expr type;
    if (!dy_substitute(ctx, *subtype.expr, subtype.id, inference_id_expr, &type)) {
        type = dy_core_expr_retain(ctx, *subtype.expr);
    }

    struct dy_core_expr app = {
        .tag = DY_CORE_EXPR_APPLICATION,
        .application = {
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .solution = {
                .is_implicit = true,
                .tag = DY_CORE_FUNCTION,
                .expr = dy_core_expr_new(inference_id_expr),
                .out = dy_core_expr_new(type)
            },
            .check_result = DY_MAYBE,
            .eval_immediately = true
        }
    };

    struct dy_core_expr transformed_app;
    bool did_transform_app = false;
    dy_ternary_t res = dy_is_subtype(ctx, *app.application.solution.out, supertype, app, &transformed_app, &did_transform_app);

    if (did_transform_app) {
        dy_core_expr_release(ctx, app);
        *new_subtype_expr = transformed_app;
    } else {
        *new_subtype_expr = app;
    }

    *did_transform_subtype_expr = true;

    return res;
}

dy_ternary_t dy_positive_implicit_pair_is_subtype(struct dy_core_ctx *ctx, struct dy_core_pair subtype, struct dy_core_expr supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_tranform_subtype_expr)
{
    struct dy_core_expr app_left = {
        .tag = DY_CORE_EXPR_APPLICATION,
        .application = {
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .solution = {
                .is_implicit = true,
                .tag = DY_CORE_PAIR,
                .direction = DY_LEFT,
                .out = dy_core_expr_retain_ptr(ctx, subtype.left)
            },
            .check_result = DY_YES,
            .eval_immediately = true
        }
    };


    size_t constraint_start1 = ctx->constraints.num_elems;

    struct dy_core_expr left_subtype_expr;
    bool did_transform_left_subtype_expr = false;
    dy_ternary_t res1 = dy_is_subtype(ctx, *app_left.application.solution.out, supertype, app_left, &left_subtype_expr, &did_transform_left_subtype_expr);

    if (!did_transform_left_subtype_expr) {
        left_subtype_expr = dy_core_expr_retain(ctx, app_left);
    }

    if (res1 == DY_YES) {
        *new_subtype_expr = left_subtype_expr;
        *did_tranform_subtype_expr = true;
        return DY_YES;
    }

    struct dy_core_expr app_right = {
        .tag = DY_CORE_EXPR_APPLICATION,
        .application = {
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .solution = {
                .is_implicit = true,
                .tag = DY_CORE_PAIR,
                .direction = DY_RIGHT,
                .out = dy_core_expr_retain_ptr(ctx, subtype.right)
            },
            .check_result = DY_YES,
            .eval_immediately = true
        }
    };


    size_t constraint_start2 = ctx->constraints.num_elems;

    struct dy_core_expr right_subtype_expr;
    bool did_transform_right_subtype_expr = false;
    dy_ternary_t res2 = dy_is_subtype(ctx, *app_right.application.solution.out, supertype, app_right, &right_subtype_expr, &did_transform_right_subtype_expr);

    if (!did_transform_right_subtype_expr) {
        right_subtype_expr = dy_core_expr_retain(ctx, app_right);
    }

    if (res1 == DY_NO && res2 == DY_NO) {
        dy_free_constraints_starting_at(ctx, constraint_start1);
        return DY_NO;
    }

    if (res2 == DY_NO) {
        *new_subtype_expr = left_subtype_expr;
        *did_tranform_subtype_expr = true;
        return res1;
    }

    if (res1 == DY_NO) {
        *new_subtype_expr = right_subtype_expr;
        *did_tranform_subtype_expr = true;
        return res2;
    }

    dy_join_constraints(ctx, constraint_start1, constraint_start2, DY_POLARITY_NEGATIVE);

    *new_subtype_expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_PROBLEM,
        .problem = {
            .is_implicit = true,
            .polarity = DY_POLARITY_POSITIVE,
            .tag = DY_CORE_PAIR,
            .pair = {
                .left = dy_core_expr_new(left_subtype_expr),
                .right = dy_core_expr_new(right_subtype_expr)
            }
        }
    };

    *did_tranform_subtype_expr = true;

    return DY_MAYBE;
}

dy_ternary_t dy_positive_implicit_recursion_is_subtype(struct dy_core_ctx *ctx, struct dy_core_recursion subtype, struct dy_core_expr supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    for (size_t i = 0; i < ctx->past_subtype_checks.num_elems; ++i) {
        const struct dy_core_past_subtype_check *past_check = dy_array_pos(&ctx->past_subtype_checks, i);
        if (dy_are_equal(ctx, past_check->subtype, *subtype.expr) == DY_YES && dy_are_equal(ctx, past_check->supertype, supertype) == DY_YES) {
            if (past_check->have_substitute_var_id) {
                *new_subtype_expr = (struct dy_core_expr){
                    .tag = DY_CORE_EXPR_VARIABLE,
                    .variable_id = past_check->substitute_var_id
                };
                *did_transform_subtype_expr = true;
                return DY_YES; // Not sure if correct here
            } else {
                return DY_NO;
            }
        }
    }

    struct dy_core_expr subtype_wrap = {
        .tag = DY_CORE_EXPR_PROBLEM,
        .problem = {
            .is_implicit = true,
            .polarity = DY_POLARITY_POSITIVE,
            .tag = DY_CORE_RECURSION,
            .recursion = subtype
        }
    };

    struct dy_core_expr type;
    if (!dy_substitute(ctx, *subtype.expr, subtype.id, subtype_wrap, &type)) {
        type = dy_core_expr_retain(ctx, *subtype.expr);
    }

    struct dy_core_expr app = {
        .tag = DY_CORE_EXPR_APPLICATION,
        .application = {
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .solution = {
                .is_implicit = true,
                .tag = DY_CORE_RECURSION,
                .out = dy_core_expr_new(dy_core_expr_retain(ctx, type))
            },
            .check_result = DY_YES,
            .eval_immediately = true
        }
    };

    dy_array_add(&ctx->past_subtype_checks, &(struct dy_core_past_subtype_check){
        .subtype = *subtype.expr,
        .supertype = supertype,
        .have_substitute_var_id = false
    });

    dy_ternary_t res = dy_is_subtype(ctx, type, supertype, app, new_subtype_expr, did_transform_subtype_expr);

    ctx->past_subtype_checks.num_elems--;

    if (!*did_transform_subtype_expr) {
        *new_subtype_expr = app;
        *did_transform_subtype_expr = true;
    }

    return res;
}

dy_ternary_t dy_negative_implicit_function_is_subtype(struct dy_core_ctx *ctx, struct dy_core_function subtype, struct dy_core_expr supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    dy_bail("not yet");
}

dy_ternary_t dy_negative_implicit_pair_is_subtype(struct dy_core_ctx *ctx, struct dy_core_pair subtype, struct dy_core_expr supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_tranform_subtype_expr)
{
    dy_bail("not yet");
}

dy_ternary_t dy_negative_implicit_recursion_is_subtype(struct dy_core_ctx *ctx, struct dy_core_recursion subtype, struct dy_core_expr supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    dy_bail("not yet");
}

dy_ternary_t dy_is_subtype_of_positive_implicit_function(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_function supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    dy_bail("not yet");
}

dy_ternary_t dy_is_subtype_of_positive_implicit_pair(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_pair supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    size_t constraint_start1 = ctx->constraints.num_elems;

    struct dy_core_expr left;
    bool did_transform_left = false;
    dy_ternary_t left_res = dy_is_subtype(ctx, subtype, *supertype.left, subtype_expr, &left, &did_transform_left);

    if (!did_transform_left) {
        left = dy_core_expr_retain(ctx, subtype_expr);
    }


    size_t constraint_start2 = ctx->constraints.num_elems;

    struct dy_core_expr right;
    bool did_transform_right = false;
    dy_ternary_t right_res = dy_is_subtype(ctx, subtype, *supertype.right, subtype_expr, &right, &did_transform_right);

    if (!did_transform_right) {
        right = dy_core_expr_retain(ctx, subtype_expr);
    }


    if (left_res == DY_NO || right_res == DY_NO) {
        dy_free_constraints_starting_at(ctx, constraint_start1);
        dy_core_expr_release(ctx, left);
        dy_core_expr_release(ctx, right);
        return DY_NO;
    }

    dy_join_constraints(ctx, constraint_start1, constraint_start2, DY_POLARITY_POSITIVE);

    *new_subtype_expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_PROBLEM,
        .problem = {
            .is_implicit = true,
            .polarity = DY_POLARITY_POSITIVE,
            .tag = DY_CORE_PAIR,
            .pair = {
                .left = dy_core_expr_new(left),
                .right = dy_core_expr_new(right)
            }
        }
    };

    *did_transform_subtype_expr = true;

    if (left_res == DY_MAYBE || right_res == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

dy_ternary_t dy_is_subtype_of_positive_implicit_recursion(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_recursion supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    dy_bail("not yet");
}

dy_ternary_t dy_is_subtype_of_negative_implicit_function(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_function supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    dy_bail("not yet");
}

dy_ternary_t dy_is_subtype_of_negative_implicit_pair(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_pair supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    dy_bail("not yet");
}

dy_ternary_t dy_is_subtype_of_negative_implicit_recursion(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_recursion supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    dy_bail("not yet");
}
