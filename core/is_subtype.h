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
 *    - A transformed 'subtype expr' as a result of subtype-checking implicit complex types.
 *    - Inference variables created by transforming implicit assumptions;
 *      They are supposed to be resolved by the caller of 'is_subtype'.
 */

static inline dy_ternary_t dy_is_subtype(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_expr supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);


static inline dy_ternary_t dy_positive_assumptions_are_subtypes(struct dy_core_ctx *ctx, struct dy_core_assumption subtype, struct dy_core_assumption supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t dy_positive_choices_are_subtypes(struct dy_core_ctx *ctx, struct dy_core_choice subtype, struct dy_core_choice supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t dy_positive_recursions_are_subtypes(struct dy_core_ctx *ctx, struct dy_core_recursion subtype, struct dy_core_recursion supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);


static inline dy_ternary_t dy_negative_assumptions_are_subtypes(struct dy_core_ctx *ctx, struct dy_core_assumption subtype, struct dy_core_assumption supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t dy_negative_choices_are_subtypes(struct dy_core_ctx *ctx, struct dy_core_choice subtype, struct dy_core_choice supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t dy_negative_recursions_are_subtypes(struct dy_core_ctx *ctx, struct dy_core_recursion subtype, struct dy_core_recursion supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);


static inline dy_ternary_t dy_simple_is_subtype_of_unwrap(struct dy_core_ctx *ctx, struct dy_core_simple simple, struct dy_core_expr unwrap_out, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);


static inline dy_ternary_t dy_proofs_are_subtypes(struct dy_core_ctx *ctx, struct dy_core_simple subtype, struct dy_core_simple supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t dy_decisions_are_subtypes(struct dy_core_ctx *ctx, struct dy_core_simple subtype, struct dy_core_simple supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t dy_unfolds_are_subtypes(struct dy_core_ctx *ctx, struct dy_core_simple subtype, struct dy_core_simple supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);


static inline dy_ternary_t dy_negative_assumption_is_subtype_of_unwrap(struct dy_core_ctx *ctx, struct dy_core_assumption subtype, struct dy_core_expr unwrap_out, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t dy_negative_choice_is_subtype_of_unwrap(struct dy_core_ctx *ctx, struct dy_core_choice subtype, struct dy_core_expr unwrap_out, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t dy_negative_recursion_is_subtype_of_unwrap(struct dy_core_ctx *ctx, struct dy_core_recursion subtype, struct dy_core_expr unwrap_out, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);


static inline dy_ternary_t dy_assumption_is_subtype_of_proof(struct dy_core_ctx *ctx, struct dy_core_assumption subtype, struct dy_core_simple supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t dy_choice_is_subtype_of_decision(struct dy_core_ctx *ctx, struct dy_core_choice subtype, struct dy_core_simple supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t dy_recursion_is_subtype_of_unfold(struct dy_core_ctx *ctx, struct dy_core_recursion subtype, struct dy_core_expr unfold_out, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);


static inline dy_ternary_t dy_proof_is_subtype_of_assumption(struct dy_core_ctx *ctx, struct dy_core_simple subtype, struct dy_core_assumption supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t dy_decision_is_subtype_of_choice(struct dy_core_ctx *ctx, struct dy_core_simple subtype, struct dy_core_choice supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t dy_unfold_is_subtype_of_recursion(struct dy_core_ctx *ctx, struct dy_core_expr unfold_out, struct dy_core_recursion supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);


static inline dy_ternary_t dy_implicit_assumption_is_subtype(struct dy_core_ctx *ctx, struct dy_core_assumption subtype, struct dy_core_expr supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t dy_implicit_choice_is_subtype(struct dy_core_ctx *ctx, struct dy_core_choice subtype, struct dy_core_expr supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t dy_implicit_recursion_is_subtype(struct dy_core_ctx *ctx, struct dy_core_recursion subtype, struct dy_core_expr supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);


static inline dy_ternary_t dy_is_subtype_of_implicit_assumption(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_assumption supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t dy_is_subtype_of_implicit_choice(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_choice supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t dy_is_subtype_of_implicit_recursion(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_recursion supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);


dy_ternary_t dy_is_subtype(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_expr supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    if (subtype.tag == DY_CORE_EXPR_INTRO && supertype.tag == DY_CORE_EXPR_INTRO && subtype.intro.is_implicit == supertype.intro.is_implicit) {
        if (subtype.intro.polarity == DY_POLARITY_NEGATIVE && supertype.intro.polarity == DY_POLARITY_POSITIVE) {
            return DY_NO;
        }

        if (subtype.intro.tag == DY_CORE_INTRO_COMPLEX && supertype.intro.tag == DY_CORE_INTRO_COMPLEX) {
            if (subtype.intro.polarity == DY_POLARITY_POSITIVE && supertype.intro.polarity == DY_POLARITY_NEGATIVE) {
                return DY_NO;
            }

            if (subtype.intro.complex.tag != supertype.intro.complex.tag) {
                return DY_NO;
            }

            if (subtype.intro.polarity == DY_POLARITY_POSITIVE) {
                switch (subtype.intro.complex.tag) {
                case DY_CORE_COMPLEX_ASSUMPTION:
                    return dy_positive_assumptions_are_subtypes(ctx, subtype.intro.complex.assumption, supertype.intro.complex.assumption, subtype.intro.is_implicit, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
                case DY_CORE_COMPLEX_CHOICE:
                    return dy_positive_choices_are_subtypes(ctx, subtype.intro.complex.choice, supertype.intro.complex.choice, subtype.intro.is_implicit, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
                case DY_CORE_COMPLEX_RECURSION:
                    return dy_positive_recursions_are_subtypes(ctx, subtype.intro.complex.recursion, supertype.intro.complex.recursion, subtype.intro.is_implicit, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
                }

                dy_bail("impossible");
            } else {
                switch (subtype.intro.complex.tag) {
                case DY_CORE_COMPLEX_ASSUMPTION:
                    return dy_negative_assumptions_are_subtypes(ctx, subtype.intro.complex.assumption, supertype.intro.complex.assumption, subtype.intro.is_implicit, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
                case DY_CORE_COMPLEX_CHOICE:
                    return dy_negative_choices_are_subtypes(ctx, subtype.intro.complex.choice, supertype.intro.complex.choice, subtype.intro.is_implicit, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
                case DY_CORE_COMPLEX_RECURSION:
                    return dy_negative_recursions_are_subtypes(ctx, subtype.intro.complex.recursion, supertype.intro.complex.recursion, subtype.intro.is_implicit, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
                }

                dy_bail("impossible");
            }
        }

        if (subtype.intro.tag == DY_CORE_INTRO_SIMPLE && supertype.intro.tag == DY_CORE_INTRO_SIMPLE) {
            if (supertype.intro.simple.tag == DY_CORE_SIMPLE_UNWRAP) {
                return dy_simple_is_subtype_of_unwrap(ctx, subtype.intro.simple, *supertype.intro.simple.out, subtype.intro.is_implicit, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
            }

            if (subtype.intro.simple.tag != supertype.intro.simple.tag) {
                return DY_NO;
            }

            switch (subtype.intro.simple.tag) {
            case DY_CORE_SIMPLE_PROOF:
                return dy_proofs_are_subtypes(ctx, subtype.intro.simple, supertype.intro.simple, subtype.intro.is_implicit, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
            case DY_CORE_SIMPLE_DECISION:
                return dy_decisions_are_subtypes(ctx, subtype.intro.simple, supertype.intro.simple, subtype.intro.is_implicit, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
            case DY_CORE_SIMPLE_UNFOLD:
                return dy_unfolds_are_subtypes(ctx, subtype.intro.simple, supertype.intro.simple, subtype.intro.is_implicit, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
            case DY_CORE_SIMPLE_UNWRAP:
                return dy_simple_is_subtype_of_unwrap(ctx, subtype.intro.simple, *supertype.intro.simple.out, subtype.intro.is_implicit, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
            }

            dy_bail("impossible");
        }

        if (subtype.intro.tag == DY_CORE_INTRO_COMPLEX && supertype.intro.tag == DY_CORE_INTRO_SIMPLE) {
            if (supertype.intro.polarity != DY_POLARITY_NEGATIVE) {
                return DY_NO;
            }

            if (supertype.intro.simple.tag == DY_CORE_SIMPLE_UNWRAP) {
                if (subtype.intro.polarity != DY_POLARITY_NEGATIVE) {
                    return DY_NO;
                }

                switch (subtype.intro.complex.tag) {
                case DY_CORE_COMPLEX_ASSUMPTION:
                    return dy_negative_assumption_is_subtype_of_unwrap(ctx, subtype.intro.complex.assumption, *supertype.intro.simple.out, subtype.intro.is_implicit, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
                case DY_CORE_COMPLEX_CHOICE:
                    return dy_negative_choice_is_subtype_of_unwrap(ctx, subtype.intro.complex.choice, *supertype.intro.simple.out, subtype.intro.is_implicit, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
                case DY_CORE_COMPLEX_RECURSION:
                    return dy_negative_recursion_is_subtype_of_unwrap(ctx, subtype.intro.complex.recursion, *supertype.intro.simple.out, subtype.intro.is_implicit, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
                }

                dy_bail("impossible");
            }

            if (subtype.intro.polarity != DY_POLARITY_POSITIVE) {
                return DY_NO;
            }

            if (subtype.intro.complex.tag == DY_CORE_COMPLEX_ASSUMPTION && supertype.intro.simple.tag == DY_CORE_SIMPLE_PROOF) {
                return dy_assumption_is_subtype_of_proof(ctx, subtype.intro.complex.assumption, supertype.intro.simple, subtype.intro.is_implicit, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
            }

            if (subtype.intro.complex.tag == DY_CORE_COMPLEX_CHOICE && supertype.intro.simple.tag == DY_CORE_SIMPLE_DECISION) {
                return dy_choice_is_subtype_of_decision(ctx, subtype.intro.complex.choice, supertype.intro.simple, subtype.intro.is_implicit, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
            }

            if (subtype.intro.complex.tag == DY_CORE_COMPLEX_RECURSION && supertype.intro.simple.tag == DY_CORE_SIMPLE_UNFOLD) {
                return dy_recursion_is_subtype_of_unfold(ctx, subtype.intro.complex.recursion, *supertype.intro.simple.out, subtype.intro.is_implicit, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
            }

            return DY_NO;
        }

        if (subtype.intro.tag == DY_CORE_INTRO_SIMPLE && supertype.intro.tag == DY_CORE_INTRO_COMPLEX) {
            if (subtype.intro.polarity != DY_POLARITY_POSITIVE && supertype.intro.polarity != DY_POLARITY_NEGATIVE) {
                return DY_NO;
            }

            if (subtype.intro.simple.tag == DY_CORE_SIMPLE_PROOF && supertype.intro.complex.tag == DY_CORE_COMPLEX_ASSUMPTION) {
                return dy_proof_is_subtype_of_assumption(ctx, subtype.intro.simple, supertype.intro.complex.assumption, subtype.intro.is_implicit, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
            }

            if (subtype.intro.simple.tag == DY_CORE_SIMPLE_DECISION && supertype.intro.complex.tag == DY_CORE_COMPLEX_CHOICE) {
                return dy_decision_is_subtype_of_choice(ctx, subtype.intro.simple, supertype.intro.complex.choice, subtype.intro.is_implicit, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
            }

            if (subtype.intro.simple.tag == DY_CORE_SIMPLE_UNFOLD && supertype.intro.complex.tag == DY_CORE_COMPLEX_RECURSION) {
                return dy_unfold_is_subtype_of_recursion(ctx, *subtype.intro.simple.out, supertype.intro.complex.recursion, subtype.intro.is_implicit, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
            }

            return DY_NO;
        }

        dy_bail("impossible");
    }

    if (subtype.tag == DY_CORE_EXPR_ELIM && supertype.tag == DY_CORE_EXPR_ELIM) {
        return dy_elims_are_equal(ctx, subtype.elim, supertype.elim);
    }

    if (subtype.tag == DY_CORE_EXPR_MAP && supertype.tag == DY_CORE_EXPR_MAP) {
        return dy_maps_are_equal(ctx, subtype.map, supertype.map);
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

        dy_join_constraints(ctx, constraint_start1, constraint_start1 + 1);

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

    if (subtype.tag == DY_CORE_EXPR_ELIM || subtype.tag == DY_CORE_EXPR_VARIABLE || subtype.tag == DY_CORE_EXPR_MAP || supertype.tag == DY_CORE_EXPR_ELIM || supertype.tag == DY_CORE_EXPR_VARIABLE || supertype.tag == DY_CORE_EXPR_MAP) {
        return DY_MAYBE;
    }

    if (subtype.tag == DY_CORE_EXPR_INTRO && subtype.intro.tag == DY_CORE_INTRO_COMPLEX && subtype.intro.is_implicit && subtype.intro.polarity == DY_POLARITY_POSITIVE) {
        switch (subtype.intro.complex.tag) {
        case DY_CORE_COMPLEX_ASSUMPTION:
            return dy_implicit_assumption_is_subtype(ctx, subtype.intro.complex.assumption, supertype, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
        case DY_CORE_COMPLEX_CHOICE:
            return dy_implicit_choice_is_subtype(ctx, subtype.intro.complex.choice, supertype, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
        case DY_CORE_COMPLEX_RECURSION:
            return dy_implicit_recursion_is_subtype(ctx, subtype.intro.complex.recursion, supertype, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
        }

        dy_bail("impossible");
    }

    if (supertype.tag == DY_CORE_EXPR_INTRO && supertype.intro.tag == DY_CORE_INTRO_COMPLEX && supertype.intro.is_implicit && supertype.intro.polarity == DY_POLARITY_NEGATIVE) {
        switch (supertype.intro.complex.tag) {
        case DY_CORE_COMPLEX_ASSUMPTION:
            return dy_is_subtype_of_implicit_assumption(ctx, subtype, supertype.intro.complex.assumption, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
        case DY_CORE_COMPLEX_CHOICE:
            return dy_is_subtype_of_implicit_choice(ctx, subtype, supertype.intro.complex.choice, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
        case DY_CORE_COMPLEX_RECURSION:
            return dy_is_subtype_of_implicit_recursion(ctx, subtype, supertype.intro.complex.recursion, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
        }

        dy_bail("impossible");
    }

    return DY_NO;
}

dy_ternary_t dy_positive_assumptions_are_subtypes(struct dy_core_ctx *ctx, struct dy_core_assumption subtype, struct dy_core_assumption supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    struct dy_core_expr id_expr = {
        .tag = DY_CORE_EXPR_VARIABLE,
        .variable_id = supertype.id
    };

    size_t constraint_start1 = ctx->constraints.num_elems;

    size_t recovered_pos_inf_vars_start = ctx->recovered_positive_inference_ids.num_elems;
    size_t recovered_neg_inf_vars_start = ctx->recovered_negative_inference_ids.num_elems;

    struct dy_core_expr transformed_id_expr;
    bool did_transform_id_expr = false;
    dy_ternary_t res1 = dy_is_subtype(ctx, *supertype.type, *subtype.type, id_expr, &transformed_id_expr, &did_transform_id_expr);
    if (res1 == DY_NO) {
        return DY_NO;
    }

    if (!did_transform_id_expr) {
        transformed_id_expr = id_expr;
    }

    struct dy_core_expr subst_subtype_expr;
    if (!dy_substitute(ctx, *subtype.expr, subtype.id, transformed_id_expr, &subst_subtype_expr)) {
        subst_subtype_expr = dy_core_expr_retain(ctx, *subtype.expr);
    }

    struct dy_core_expr elim = {
        .tag = DY_CORE_EXPR_ELIM,
        .elim = {
            .check_result = res1,
            .is_implicit = is_implicit,
            .eval_immediately = true,
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .simple = {
                .tag = DY_CORE_SIMPLE_PROOF,
                .proof = dy_core_expr_new(transformed_id_expr),
                .out = dy_core_expr_new(subst_subtype_expr)
            }
        }
    };

    dy_array_add(&ctx->free_ids_arrays, &(struct dy_free_var){
        .id = supertype.id,
        .type = *supertype.type
    });

    size_t constraint_start2 = ctx->constraints.num_elems;

    struct dy_core_expr transformed_elim;
    bool did_transform_elim = false;
    dy_ternary_t res2 = dy_is_subtype(ctx, subst_subtype_expr, *supertype.expr, elim, &transformed_elim, &did_transform_elim);

    dy_remove_mentions_in_constraints(ctx, supertype.id, constraint_start2);

    --ctx->free_ids_arrays.num_elems;

    dy_join_constraints(ctx, constraint_start1, constraint_start2);

    if (did_transform_elim) {
        dy_core_expr_release(ctx, elim);
    } else {
        transformed_elim = elim;
    }

    if (res2 == DY_NO) {
        dy_free_constraints_in_range(ctx, constraint_start1, ctx->constraints.num_elems);

        ctx->recovered_positive_inference_ids.num_elems = recovered_pos_inf_vars_start;
        ctx->recovered_negative_inference_ids.num_elems = recovered_neg_inf_vars_start;

        dy_core_expr_release(ctx, transformed_elim);

        return DY_NO;
    }

    if (did_transform_id_expr || did_transform_elim) {
        *did_transform_subtype_expr = true;
        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_INTRO,
            .intro = {
                .polarity = DY_POLARITY_POSITIVE,
                .is_implicit = is_implicit,
                .tag = DY_CORE_INTRO_COMPLEX,
                .complex = {
                    .tag = DY_CORE_COMPLEX_ASSUMPTION,
                    .assumption = {
                        .id = supertype.id,
                        .type = dy_core_expr_retain_ptr(ctx, supertype.type),
                        .expr = dy_core_expr_new(transformed_elim)
                    }
                }
            }
        };
    } else {
        dy_core_expr_release(ctx, transformed_elim);
    }

    if (res1 == DY_MAYBE || res2 == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

dy_ternary_t dy_positive_choices_are_subtypes(struct dy_core_ctx *ctx, struct dy_core_choice subtype, struct dy_core_choice supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    struct dy_core_expr left_elim = {
        .tag = DY_CORE_EXPR_ELIM,
        .elim = {
            .check_result = DY_YES,
            .eval_immediately = true,
            .is_implicit = is_implicit,
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .simple = {
                .tag = DY_CORE_SIMPLE_DECISION,
                .direction = DY_LEFT,
                .out = dy_core_expr_retain_ptr(ctx, subtype.left)
            }
        }
    };

    size_t constraint_start1 = ctx->constraints.num_elems;

    size_t recovered_pos_inf_vars_start = ctx->recovered_positive_inference_ids.num_elems;
    size_t recovered_neg_inf_vars_start = ctx->recovered_negative_inference_ids.num_elems;

    struct dy_core_expr transformed_left_elim;
    bool did_transform_left_elim = false;
    dy_ternary_t res_left = dy_is_subtype(ctx, *subtype.left, *supertype.left, left_elim, &transformed_left_elim, &did_transform_left_elim);
    if (res_left == DY_NO) {
        dy_core_expr_release(ctx, left_elim);
        return DY_NO;
    }

    if (did_transform_left_elim) {
        dy_core_expr_release(ctx, left_elim);
    } else {
        transformed_left_elim = left_elim;
    }

    struct dy_core_expr right_elim = {
        .tag = DY_CORE_EXPR_ELIM,
        .elim = {
            .check_result = DY_YES,
            .eval_immediately = true,
            .is_implicit = is_implicit,
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .simple = {
                .tag = DY_CORE_SIMPLE_DECISION,
                .direction = DY_RIGHT,
                .out = dy_core_expr_retain_ptr(ctx, subtype.right)
            }
        }
    };

    size_t constraint_start2 = ctx->constraints.num_elems;

    struct dy_core_expr transformed_right_elim;
    bool did_transform_right_elim = false;
    dy_ternary_t res_right = dy_is_subtype(ctx, *subtype.right, *supertype.right, right_elim, &transformed_right_elim, &did_transform_right_elim);

    if (res_right == DY_NO) {
        dy_core_expr_release(ctx, transformed_left_elim);
        dy_core_expr_release(ctx, right_elim);

        dy_free_constraints_in_range(ctx, constraint_start1, ctx->constraints.num_elems);

        ctx->recovered_positive_inference_ids.num_elems = recovered_pos_inf_vars_start;
        ctx->recovered_negative_inference_ids.num_elems = recovered_neg_inf_vars_start;

        return DY_NO;
    }

    if (did_transform_right_elim) {
        dy_core_expr_release(ctx, right_elim);
    } else {
        transformed_right_elim = right_elim;
    }

    dy_join_constraints(ctx, constraint_start1, constraint_start2);

    if (did_transform_left_elim || did_transform_right_elim) {
        *did_transform_subtype_expr = true;
        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_INTRO,
            .intro = {
                .polarity = DY_POLARITY_POSITIVE,
                .is_implicit = is_implicit,
                .tag = DY_CORE_INTRO_COMPLEX,
                .complex = {
                    .tag = DY_CORE_COMPLEX_CHOICE,
                    .choice = {
                        .left = dy_core_expr_new(transformed_left_elim),
                        .right = dy_core_expr_new(transformed_right_elim)
                    }
                }
            }
        };
    } else {
        dy_core_expr_release(ctx, transformed_left_elim);
        dy_core_expr_release(ctx, transformed_right_elim);
    }

    if (res_left == DY_MAYBE || res_right == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

dy_ternary_t dy_positive_recursions_are_subtypes(struct dy_core_ctx *ctx, struct dy_core_recursion subtype, struct dy_core_recursion supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    dy_bail("not yet implemented");
}

dy_ternary_t dy_negative_assumptions_are_subtypes(struct dy_core_ctx *ctx, struct dy_core_assumption subtype, struct dy_core_assumption supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    dy_ternary_t res1 = dy_are_equal(ctx, *subtype.type, *supertype.type);
    if (res1 == DY_NO) {
        return DY_NO;
    }

    dy_array_add(&ctx->equal_variables, &(struct dy_equal_variables){
        .id1 = subtype.id,
        .id2 = supertype.id
    });

    size_t id = ctx->running_id++;
    struct dy_core_expr id_expr = {
        .tag = DY_CORE_EXPR_VARIABLE,
        .variable_id = id
    };

    struct dy_core_expr transformed_id_expr;
    bool did_transform_id_expr = false;
    dy_ternary_t res2 = dy_is_subtype(ctx, *subtype.expr, *supertype.expr, id_expr, &transformed_id_expr, &did_transform_id_expr);

    ctx->equal_variables.num_elems--;

    if (res2 == DY_NO) {
        return DY_NO;
    }

    if (did_transform_id_expr) {
        *did_transform_subtype_expr = true;
        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_ELIM,
            .elim = {
                .expr = dy_core_expr_new((struct dy_core_expr){
                    .tag = DY_CORE_EXPR_MAP,
                    .map = {
                        .is_implicit = is_implicit,
                        .tag = DY_CORE_MAP_ASSUMPTION,
                        .assumption = {
                            .dependence = DY_CORE_MAP_DEPENDENCE_INDEPENDENT,
                            .id = subtype.id,
                            .type = dy_core_expr_retain_ptr(ctx, subtype.type),
                            .assumption = {
                                .id = id,
                                .type = dy_core_expr_retain_ptr(ctx, subtype.expr),
                                .expr = dy_core_expr_new(transformed_id_expr)
                            }
                        }
                    }
                }),
                .simple = {
                    .tag = DY_CORE_SIMPLE_PROOF,
                    .proof = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
                    .out = dy_core_expr_new((struct dy_core_expr){
                        .tag = DY_CORE_EXPR_INTRO,
                        .intro = {
                            .polarity = DY_POLARITY_NEGATIVE,
                            .is_implicit = is_implicit,
                            .tag = DY_CORE_INTRO_COMPLEX,
                            .complex = {
                                .tag = DY_CORE_COMPLEX_ASSUMPTION,
                                .assumption = {
                                    .id = supertype.id,
                                    .type = dy_core_expr_retain_ptr(ctx, supertype.type),
                                    .expr = dy_core_expr_retain_ptr(ctx, supertype.expr)
                                }
                            }
                        }
                    })
                },
                .is_implicit = false,
                .check_result = DY_MAYBE,
                .eval_immediately = false
            }
        };
    }

    if (res1 == DY_MAYBE || res2 == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

dy_ternary_t dy_negative_choices_are_subtypes(struct dy_core_ctx *ctx, struct dy_core_choice subtype, struct dy_core_choice supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    size_t left_id = ctx->running_id++;
    struct dy_core_expr left_id_expr = {
        .tag = DY_CORE_EXPR_VARIABLE,
        .variable_id = left_id
    };

    size_t constraint_start1 = ctx->constraints.num_elems;

    size_t recovered_pos_inf_vars_start = ctx->recovered_positive_inference_ids.num_elems;
    size_t recovered_neg_inf_vars_start = ctx->recovered_negative_inference_ids.num_elems;

    struct dy_core_expr left_transform;
    bool have_left_transform = false;
    dy_ternary_t left_res = dy_is_subtype(ctx, *subtype.left, *supertype.left, left_id_expr, &left_transform, &have_left_transform);
    if (left_res == DY_NO) {
        return DY_NO;
    }

    if (!have_left_transform) {
        left_transform = dy_core_expr_retain(ctx, left_id_expr);
    }

    size_t right_id = ctx->running_id++;
    struct dy_core_expr right_id_expr = {
        .tag = DY_CORE_EXPR_VARIABLE,
        .variable_id = right_id
    };

    size_t constraint_start2 = ctx->constraints.num_elems;

    struct dy_core_expr right_transform;
    bool have_right_transform = false;
    dy_ternary_t right_res = dy_is_subtype(ctx, *subtype.right, *supertype.right, right_id_expr, &right_transform, &have_right_transform);
    if (right_res == DY_NO) {
        dy_free_constraints_in_range(ctx, constraint_start1, ctx->constraints.num_elems);

        ctx->recovered_positive_inference_ids.num_elems = recovered_pos_inf_vars_start;
        ctx->recovered_negative_inference_ids.num_elems = recovered_neg_inf_vars_start;

        dy_core_expr_release(ctx, left_transform);

        return DY_NO;
    }

    if (!have_right_transform) {
        right_transform = dy_core_expr_retain(ctx, right_id_expr);
    }

    dy_join_constraints(ctx, constraint_start1, constraint_start2);

    if (have_left_transform || have_right_transform) {
        *did_transform_subtype_expr = true;
        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_ELIM,
            .elim = {
                .expr = dy_core_expr_new((struct dy_core_expr){
                    .tag = DY_CORE_EXPR_MAP,
                    .map = {
                        .is_implicit = is_implicit,
                        .tag = DY_CORE_MAP_CHOICE,
                        .choice = {
                            .left_dependence = DY_CORE_MAP_DEPENDENCE_INDEPENDENT,
                            .right_dependence = DY_CORE_MAP_DEPENDENCE_INDEPENDENT,
                            .assumption_left = {
                                .id = left_id,
                                .type = dy_core_expr_retain_ptr(ctx, subtype.left),
                                .expr = dy_core_expr_new(left_transform)
                            },
                            .assumption_right = {
                                .id = right_id,
                                .type = dy_core_expr_retain_ptr(ctx, subtype.right),
                                .expr = dy_core_expr_new(right_transform)
                            }
                        }
                    }
                }),
                .simple = {
                    .tag = DY_CORE_SIMPLE_PROOF,
                    .proof = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
                    .out = dy_core_expr_new((struct dy_core_expr){
                        .tag = DY_CORE_EXPR_INTRO,
                        .intro = {
                            .polarity = DY_POLARITY_NEGATIVE,
                            .is_implicit = is_implicit,
                            .tag = DY_CORE_INTRO_COMPLEX,
                            .complex = {
                                .tag = DY_CORE_COMPLEX_CHOICE,
                                .choice = {
                                    .left = dy_core_expr_retain_ptr(ctx, supertype.left),
                                    .right = dy_core_expr_retain_ptr(ctx, supertype.right)
                                }
                            }
                        }
                    })
                },
                .is_implicit = false,
                .check_result = DY_MAYBE,
                .eval_immediately = false
            }
        };
    } else {
        dy_core_expr_release(ctx, left_transform);
        dy_core_expr_release(ctx, right_transform);
    }

    if (left_res == DY_MAYBE || right_res == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

dy_ternary_t dy_negative_recursions_are_subtypes(struct dy_core_ctx *ctx, struct dy_core_recursion subtype, struct dy_core_recursion supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    dy_bail("not yet implemented");
}

dy_ternary_t dy_simple_is_subtype_of_unwrap(struct dy_core_ctx *ctx, struct dy_core_simple simple, struct dy_core_expr unwrap_out, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    struct dy_core_expr elim = {
        .tag = DY_CORE_EXPR_ELIM,
        .elim = {
            .is_implicit = is_implicit,
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .simple = {
                .tag = DY_CORE_SIMPLE_UNWRAP,
                .out = dy_core_expr_retain_ptr(ctx, simple.out)
            },
            .eval_immediately = true,
            .check_result = DY_YES,
        }
    };

    struct dy_core_expr transformed_elim;
    bool did_transform_elim = false;
    dy_ternary_t res = dy_is_subtype(ctx, *simple.out, unwrap_out, elim, &transformed_elim, &did_transform_elim);

    dy_core_expr_release(ctx, elim);

    if (did_transform_elim) {
        *did_transform_subtype_expr = true;

        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_INTRO,
            .intro = {
                .polarity = DY_POLARITY_POSITIVE,
                .is_implicit = is_implicit,
                .tag = DY_CORE_INTRO_SIMPLE,
                .simple = {
                    .tag = DY_CORE_SIMPLE_UNWRAP,
                    .out = dy_core_expr_new(transformed_elim)
                }
            }
        };
    }

    return res;
}

dy_ternary_t dy_proofs_are_subtypes(struct dy_core_ctx *ctx, struct dy_core_simple subtype, struct dy_core_simple supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    dy_ternary_t res1 = dy_are_equal(ctx, *subtype.proof, *supertype.proof);
    if (res1 == DY_NO) {
        return DY_NO;
    }

    struct dy_core_expr elim = {
        .tag = DY_CORE_EXPR_ELIM,
        .elim = {
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .simple = {
                .tag = DY_CORE_SIMPLE_PROOF,
                .proof = dy_core_expr_retain_ptr(ctx, subtype.proof),
                .out = dy_core_expr_retain_ptr(ctx, subtype.out)
            },
            .is_implicit = is_implicit,
            .check_result = res1,
            .eval_immediately = true
        }
    };

    struct dy_core_expr transformed_elim;
    bool did_transform_elim = false;
    dy_ternary_t res2 = dy_is_subtype(ctx, *subtype.out, *supertype.out, elim, &transformed_elim, &did_transform_elim);

    dy_core_expr_release(ctx, elim);

    if (res2 == DY_NO) {
        return DY_NO;
    }

    if (did_transform_elim) {
        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_INTRO,
            .intro = {
                .polarity = DY_POLARITY_POSITIVE,
                .is_implicit = is_implicit,
                .tag = DY_CORE_INTRO_SIMPLE,
                .simple = {
                    .tag = DY_CORE_SIMPLE_PROOF,
                    .proof = dy_core_expr_retain_ptr(ctx, supertype.proof),
                    .out = dy_core_expr_new(transformed_elim)
                }
            }
        };
        *did_transform_subtype_expr = true;
    }

    if (res1 == DY_MAYBE || res2 == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

dy_ternary_t dy_decisions_are_subtypes(struct dy_core_ctx *ctx, struct dy_core_simple subtype, struct dy_core_simple supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    if (subtype.direction != supertype.direction) {
        return DY_NO;
    }

    struct dy_core_expr elim = {
        .tag = DY_CORE_EXPR_ELIM,
        .elim = {
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .simple = {
                .tag = DY_CORE_SIMPLE_DECISION,
                .direction = subtype.direction,
                .out = dy_core_expr_retain_ptr(ctx, subtype.out)
            },
            .is_implicit = is_implicit,
            .check_result = DY_YES,
            .eval_immediately = true
        }
    };

    struct dy_core_expr transformed_elim;
    bool did_transform_elim = false;
    dy_ternary_t res = dy_is_subtype(ctx, *subtype.out, *supertype.out, elim, &transformed_elim, &did_transform_elim);

    dy_core_expr_release(ctx, elim);

    if (res == DY_NO) {
        return DY_NO;
    }

    if (did_transform_elim) {
        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_INTRO,
            .intro = {
                .polarity = DY_POLARITY_POSITIVE,
                .is_implicit = is_implicit,
                .tag = DY_CORE_INTRO_SIMPLE,
                .simple = {
                    .tag = DY_CORE_SIMPLE_DECISION,
                    .direction = subtype.direction,
                    .out = dy_core_expr_new(transformed_elim)
                }
            }
        };
        *did_transform_subtype_expr = true;
    }

    return res;
}

dy_ternary_t dy_unfolds_are_subtypes(struct dy_core_ctx *ctx, struct dy_core_simple subtype, struct dy_core_simple supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    struct dy_core_expr elim = {
        .tag = DY_CORE_EXPR_ELIM,
        .elim = {
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .simple = {
                .tag = DY_CORE_SIMPLE_UNFOLD,
                .out = dy_core_expr_retain_ptr(ctx, subtype.out)
            },
            .is_implicit = is_implicit,
            .check_result = DY_YES,
            .eval_immediately = true
        }
    };

    struct dy_core_expr transformed_elim;
    bool did_transform_elim = false;
    dy_ternary_t res = dy_is_subtype(ctx, *subtype.out, *supertype.out, elim, &transformed_elim, &did_transform_elim);

    dy_core_expr_release(ctx, elim);

    if (res == DY_NO) {
        return DY_NO;
    }

    if (did_transform_elim) {
        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_INTRO,
            .intro = {
                .polarity = DY_POLARITY_POSITIVE,
                .is_implicit = is_implicit,
                .tag = DY_CORE_INTRO_SIMPLE,
                .simple = {
                    .tag = DY_CORE_SIMPLE_UNFOLD,
                    .out = dy_core_expr_new(transformed_elim)
                }
            }
        };
        *did_transform_subtype_expr = true;
    }

    return res;
}

dy_ternary_t dy_negative_assumption_is_subtype_of_unwrap(struct dy_core_ctx *ctx, struct dy_core_assumption subtype, struct dy_core_expr unwrap_out, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    if (dy_core_expr_contains_this_variable(ctx, subtype.id, *subtype.expr)) {
        return DY_NO;
    }

    struct dy_core_expr elim = {
        .tag = DY_CORE_EXPR_ELIM,
        .elim = {
            .is_implicit = is_implicit,
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .simple = {
                .tag = DY_CORE_SIMPLE_UNWRAP,
                .out = dy_core_expr_retain_ptr(ctx, subtype.expr)
            },
            .eval_immediately = true,
            .check_result = DY_YES,
        }
    };

    struct dy_core_expr transformed_elim;
    bool did_transform_elim = false;
    dy_ternary_t res = dy_is_subtype(ctx, *subtype.expr, unwrap_out, elim, &transformed_elim, &did_transform_elim);

    dy_core_expr_release(ctx, elim);

    if (did_transform_elim) {
        *did_transform_subtype_expr = true;

        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_INTRO,
            .intro = {
                .polarity = DY_POLARITY_POSITIVE,
                .is_implicit = is_implicit,
                .tag = DY_CORE_INTRO_SIMPLE,
                .simple = {
                    .tag = DY_CORE_SIMPLE_UNWRAP,
                    .out = dy_core_expr_new(transformed_elim)
                }
            }
        };
    }

    return res;
}

dy_ternary_t dy_negative_choice_is_subtype_of_unwrap(struct dy_core_ctx *ctx, struct dy_core_choice subtype, struct dy_core_expr unwrap_out, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    if (dy_are_equal(ctx, *subtype.left, *subtype.right) != DY_YES) {
        return DY_NO;
    }

    struct dy_core_expr elim = {
        .tag = DY_CORE_EXPR_ELIM,
        .elim = {
            .is_implicit = is_implicit,
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .simple = {
                .tag = DY_CORE_SIMPLE_UNWRAP,
                .out = dy_core_expr_retain_ptr(ctx, subtype.left)
            },
            .eval_immediately = true,
            .check_result = DY_YES,
        }
    };

    struct dy_core_expr transformed_elim;
    bool did_transform_elim = false;
    dy_ternary_t res = dy_is_subtype(ctx, *subtype.left, unwrap_out, elim, &transformed_elim, &did_transform_elim);

    dy_core_expr_release(ctx, elim);

    if (did_transform_elim) {
        *did_transform_subtype_expr = true;

        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_INTRO,
            .intro = {
                .polarity = DY_POLARITY_POSITIVE,
                .is_implicit = is_implicit,
                .tag = DY_CORE_INTRO_SIMPLE,
                .simple = {
                    .tag = DY_CORE_SIMPLE_UNWRAP,
                    .out = dy_core_expr_new(transformed_elim)
                }
            }
        };
    }

    return res;
}

dy_ternary_t dy_negative_recursion_is_subtype_of_unwrap(struct dy_core_ctx *ctx, struct dy_core_recursion subtype, struct dy_core_expr unwrap_out, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    if (dy_core_expr_contains_this_variable(ctx, subtype.id, *subtype.expr)) {
        return DY_NO;
    }

    struct dy_core_expr elim = {
        .tag = DY_CORE_EXPR_ELIM,
        .elim = {
            .is_implicit = is_implicit,
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .simple = {
                .tag = DY_CORE_SIMPLE_UNWRAP,
                .out = dy_core_expr_retain_ptr(ctx, subtype.expr)
            },
            .eval_immediately = true,
            .check_result = DY_YES,
        }
    };

    struct dy_core_expr transformed_elim;
    bool did_transform_elim = false;
    dy_ternary_t res = dy_is_subtype(ctx, *subtype.expr, unwrap_out, elim, &transformed_elim, &did_transform_elim);

    dy_core_expr_release(ctx, elim);

    if (did_transform_elim) {
        *did_transform_subtype_expr = true;

        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_INTRO,
            .intro = {
                .polarity = DY_POLARITY_POSITIVE,
                .is_implicit = is_implicit,
                .tag = DY_CORE_INTRO_SIMPLE,
                .simple = {
                    .tag = DY_CORE_SIMPLE_UNWRAP,
                    .out = dy_core_expr_new(transformed_elim)
                }
            }
        };
    }

    return res;
}

dy_ternary_t dy_assumption_is_subtype_of_proof(struct dy_core_ctx *ctx, struct dy_core_assumption subtype, struct dy_core_simple supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    struct dy_core_expr type_of_proof = dy_type_of(ctx, *supertype.proof);

    size_t constraint_start1 = ctx->constraints.num_elems;

    size_t recovered_pos_inf_vars_start = ctx->recovered_positive_inference_ids.num_elems;
    size_t recovered_neg_inf_vars_start = ctx->recovered_negative_inference_ids.num_elems;

    struct dy_core_expr transformed_supertype_expr;
    bool did_transform_supertype_expr = false;
    dy_ternary_t res1 = dy_is_subtype(ctx, type_of_proof, *subtype.type, *supertype.proof, &transformed_supertype_expr, &did_transform_supertype_expr);

    dy_core_expr_release(ctx, type_of_proof);

    if (!did_transform_supertype_expr) {
        transformed_supertype_expr = dy_core_expr_retain(ctx, *supertype.proof);
    }

    if (res1 == DY_NO) {
        dy_core_expr_release(ctx, transformed_supertype_expr);
        return DY_NO;
    }

    struct dy_core_expr subst_subtype;
    if (!dy_substitute(ctx, *subtype.expr, subtype.id, transformed_supertype_expr, &subst_subtype)) {
        subst_subtype = dy_core_expr_retain(ctx, *subtype.expr);
    }

    struct dy_core_expr elim = {
        .tag = DY_CORE_EXPR_ELIM,
        .elim = {
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .simple = {
                .tag = DY_CORE_SIMPLE_PROOF,
                .proof = dy_core_expr_new(transformed_supertype_expr),
                .out = dy_core_expr_new(subst_subtype)
            },
            .is_implicit = is_implicit,
            .check_result = res1,
            .eval_immediately = true
        }
    };


    size_t constraint_start2 = ctx->constraints.num_elems;

    struct dy_core_expr transformed_elim;
    bool did_transform_elim = false;
    dy_ternary_t res2 = dy_is_subtype(ctx, subst_subtype, *supertype.out, elim, &transformed_elim, &did_transform_elim);

    if (!did_transform_elim) {
        transformed_elim = elim;
    } else {
        dy_core_expr_release(ctx, elim);
    }

    if (res2 == DY_NO) {
        dy_core_expr_release(ctx, transformed_elim);

        dy_free_constraints_in_range(ctx, constraint_start1, ctx->constraints.num_elems);

        ctx->recovered_positive_inference_ids.num_elems = recovered_pos_inf_vars_start;
        ctx->recovered_negative_inference_ids.num_elems = recovered_neg_inf_vars_start;

        return DY_NO;
    }

    dy_join_constraints(ctx, constraint_start1, constraint_start2);

    if (did_transform_supertype_expr || did_transform_elim) {
        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_INTRO,
            .intro = {
                .polarity = DY_POLARITY_POSITIVE,
                .is_implicit = is_implicit,
                .tag = DY_CORE_INTRO_SIMPLE,
                .simple = {
                    .tag = DY_CORE_SIMPLE_PROOF,
                    .proof = dy_core_expr_retain_ptr(ctx, supertype.proof),
                    .out = dy_core_expr_new(transformed_elim)
                }
            }
        };
        *did_transform_subtype_expr = true;
    } else {
        dy_core_expr_release(ctx, transformed_elim);
    }

    if (res1 == DY_MAYBE || res2 == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

dy_ternary_t dy_choice_is_subtype_of_decision(struct dy_core_ctx *ctx, struct dy_core_choice subtype, struct dy_core_simple supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    if (supertype.direction == DY_LEFT) {
        struct dy_core_expr elim = {
            .tag = DY_CORE_EXPR_ELIM,
            .elim = {
                .check_result = DY_YES,
                .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
                .simple = {
                    .tag = DY_CORE_SIMPLE_DECISION,
                    .direction = DY_LEFT,
                    .out = dy_core_expr_retain_ptr(ctx, subtype.left)
                },
                .is_implicit = is_implicit,
                .eval_immediately = true
            }
        };

        struct dy_core_expr transformed_elim;
        bool did_transform_elim = false;
        dy_ternary_t res = dy_is_subtype(ctx, *subtype.left, *supertype.out, elim, &transformed_elim, &did_transform_elim);

        dy_core_expr_release(ctx, elim);

        if (did_transform_elim) {
            *did_transform_subtype_expr = true;
            *new_subtype_expr = (struct dy_core_expr){
                .tag = DY_CORE_EXPR_INTRO,
                .intro = {
                    .polarity = DY_POLARITY_POSITIVE,
                    .is_implicit = is_implicit,
                    .tag = DY_CORE_INTRO_SIMPLE,
                    .simple = {
                        .tag = DY_CORE_SIMPLE_DECISION,
                        .direction = DY_LEFT,
                        .out = dy_core_expr_new(transformed_elim)
                    }
                }
            };
        }

        return res;
    } else {
        struct dy_core_expr elim = {
            .tag = DY_CORE_EXPR_ELIM,
            .elim = {
                .check_result = DY_YES,
                .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
                .simple = {
                    .tag = DY_CORE_SIMPLE_DECISION,
                    .direction = DY_RIGHT,
                    .out = dy_core_expr_retain_ptr(ctx, subtype.right)
                },
                .is_implicit = is_implicit,
                .eval_immediately = true
            }
        };

        struct dy_core_expr transformed_elim;
        bool did_transform_elim = false;
        dy_ternary_t res = dy_is_subtype(ctx, *subtype.right, *supertype.out, elim, &transformed_elim, &did_transform_elim);

        dy_core_expr_release(ctx, elim);

        if (did_transform_elim) {
            *did_transform_subtype_expr = true;
            *new_subtype_expr = (struct dy_core_expr){
                .tag = DY_CORE_EXPR_INTRO,
                .intro = {
                    .polarity = DY_POLARITY_POSITIVE,
                    .is_implicit = is_implicit,
                    .tag = DY_CORE_INTRO_SIMPLE,
                    .simple = {
                        .tag = DY_CORE_SIMPLE_DECISION,
                        .direction = DY_RIGHT,
                        .out = dy_core_expr_new(transformed_elim)
                    }
                }
            };
        }

        return res;
    }
}

dy_ternary_t dy_recursion_is_subtype_of_unfold(struct dy_core_ctx *ctx, struct dy_core_recursion subtype, struct dy_core_expr unfold_out, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    struct dy_core_expr subtype_wrap = {
        .tag = DY_CORE_EXPR_INTRO,
        .intro = {
            .polarity = DY_POLARITY_POSITIVE,
            .is_implicit = is_implicit,
            .tag = DY_CORE_INTRO_COMPLEX,
            .complex = {
                .tag = DY_CORE_COMPLEX_RECURSION,
                .recursion = subtype
            }
        }
    };

    struct dy_core_expr subst_subtype_expr;
    if (!dy_substitute(ctx, *subtype.expr, subtype.id, subtype_wrap, &subst_subtype_expr)) {
        subst_subtype_expr = dy_core_expr_retain(ctx, *subtype.expr);
    }

    struct dy_core_expr elim = {
        .tag = DY_CORE_EXPR_ELIM,
        .elim = {
            .check_result = DY_YES,
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .simple = {
                .tag = DY_CORE_SIMPLE_UNFOLD,
                .out = dy_core_expr_new(subst_subtype_expr)
            },
            .is_implicit = is_implicit,
            .eval_immediately = true
        }
    };

    struct dy_core_expr transformed_elim;
    bool did_transform_elim = false;
    dy_ternary_t res = dy_is_subtype(ctx, subst_subtype_expr, unfold_out, elim, &transformed_elim, &did_transform_elim);

    dy_core_expr_release(ctx, elim);

    if (did_transform_elim) {
        *did_transform_subtype_expr = true;
        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_INTRO,
            .intro = {
                .polarity = DY_POLARITY_POSITIVE,
                .is_implicit = is_implicit,
                .tag = DY_CORE_INTRO_SIMPLE,
                .simple = {
                    .tag = DY_CORE_SIMPLE_UNFOLD,
                    .out = dy_core_expr_new(transformed_elim)
                }
            }
        };
    }

    return res;
}

dy_ternary_t dy_proof_is_subtype_of_assumption(struct dy_core_ctx *ctx, struct dy_core_simple subtype, struct dy_core_assumption supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    struct dy_core_expr type_of_proof = dy_type_of(ctx, *subtype.proof);

    size_t constraint_start1 = ctx->constraints.num_elems;

    size_t recovered_pos_inf_vars_start = ctx->recovered_positive_inference_ids.num_elems;
    size_t recovered_neg_inf_vars_start = ctx->recovered_negative_inference_ids.num_elems;

    struct dy_core_expr transformed_proof;
    bool did_transform_proof = false;
    dy_ternary_t res1 = dy_is_subtype(ctx, type_of_proof, *supertype.type, *subtype.proof, &transformed_proof, &did_transform_proof);

    if (!did_transform_proof) {
        transformed_proof = dy_core_expr_retain(ctx, *subtype.proof);
    }

    if (res1 == DY_NO) {
        dy_core_expr_release(ctx, transformed_proof);
        return DY_NO;
    }

    struct dy_core_expr elim = {
        .tag = DY_CORE_EXPR_ELIM,
        .elim = {
            .is_implicit = is_implicit,
            .eval_immediately = true,
            .check_result = res1,
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .simple = {
                .tag = DY_CORE_SIMPLE_PROOF,
                .proof = dy_core_expr_retain_ptr(ctx, subtype.proof),
                .out = dy_core_expr_retain_ptr(ctx, subtype.out)
            }
        }
    };

    struct dy_core_expr subst_supertype_expr;
    if (!dy_substitute(ctx, *supertype.expr, supertype.id, transformed_proof, &subst_supertype_expr)) {
        subst_supertype_expr = dy_core_expr_retain(ctx, *supertype.expr);
    }

    size_t constraint_start2 = ctx->constraints.num_elems;

    struct dy_core_expr transformed_elim;
    bool did_transform_elim = false;
    dy_ternary_t res2 = dy_is_subtype(ctx, *subtype.out, subst_supertype_expr, elim, &transformed_elim, &did_transform_elim);

    dy_core_expr_release(ctx, subst_supertype_expr);

    if (did_transform_elim) {
        dy_core_expr_release(ctx, elim);
    } else {
        transformed_elim = elim;
    }

    if (res2 == DY_NO) {
        dy_free_constraints_in_range(ctx, constraint_start1, ctx->constraints.num_elems);

        ctx->recovered_positive_inference_ids.num_elems = recovered_pos_inf_vars_start;
        ctx->recovered_negative_inference_ids.num_elems = recovered_neg_inf_vars_start;

        dy_core_expr_release(ctx, transformed_proof);
        dy_core_expr_release(ctx, transformed_elim);

        return DY_NO;
    }

    dy_join_constraints(ctx, constraint_start1, constraint_start2);

    if (did_transform_proof || did_transform_elim) {
        *did_transform_subtype_expr = true;
        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_INTRO,
            .intro = {
                .polarity = DY_POLARITY_POSITIVE,
                .is_implicit = is_implicit,
                .tag = DY_CORE_INTRO_SIMPLE,
                .simple = {
                    .tag = DY_CORE_SIMPLE_PROOF,
                    .proof = dy_core_expr_new(transformed_proof),
                    .out = dy_core_expr_new(transformed_elim)
                }
            }
        };
    } else {
        dy_core_expr_release(ctx, transformed_proof);
        dy_core_expr_release(ctx, transformed_elim);
    }

    if (res1 == DY_MAYBE || res2 == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

dy_ternary_t dy_decision_is_subtype_of_choice(struct dy_core_ctx *ctx, struct dy_core_simple subtype, struct dy_core_choice supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    if (subtype.direction == DY_LEFT) {
        struct dy_core_expr elim = {
            .tag = DY_CORE_EXPR_ELIM,
            .elim = {
                .check_result = DY_YES,
                .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
                .simple = {
                    .tag = DY_CORE_SIMPLE_DECISION,
                    .direction = DY_LEFT,
                    .out = dy_core_expr_retain_ptr(ctx, subtype.out)
                },
                .is_implicit = is_implicit,
                .eval_immediately = true
            }
        };

        struct dy_core_expr transformed_elim;
        bool did_transform_elim = false;
        dy_ternary_t res = dy_is_subtype(ctx, *subtype.out, *supertype.left, elim, &transformed_elim, &did_transform_elim);

        dy_core_expr_release(ctx, elim);

        if (did_transform_elim) {
            *did_transform_subtype_expr = true;
            *new_subtype_expr = (struct dy_core_expr){
                .tag = DY_CORE_EXPR_INTRO,
                .intro = {
                    .polarity = DY_POLARITY_POSITIVE,
                    .is_implicit = is_implicit,
                    .tag = DY_CORE_INTRO_SIMPLE,
                    .simple = {
                        .tag = DY_CORE_SIMPLE_DECISION,
                        .direction = DY_LEFT,
                        .out = dy_core_expr_new(transformed_elim)
                    }
                }
            };
        }

        return res;
    } else {
        struct dy_core_expr elim = {
            .tag = DY_CORE_EXPR_ELIM,
            .elim = {
                .check_result = DY_YES,
                .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
                .simple = {
                    .tag = DY_CORE_SIMPLE_DECISION,
                    .direction = DY_RIGHT,
                    .out = dy_core_expr_retain_ptr(ctx, subtype.out)
                },
                .is_implicit = is_implicit,
                .eval_immediately = true
            }
        };

        struct dy_core_expr transformed_elim;
        bool did_transform_elim = false;
        dy_ternary_t res = dy_is_subtype(ctx, *subtype.out, *supertype.right, elim, &transformed_elim, &did_transform_elim);

        dy_core_expr_release(ctx, elim);

        if (did_transform_elim) {
            *did_transform_subtype_expr = true;
            *new_subtype_expr = (struct dy_core_expr){
                .tag = DY_CORE_EXPR_INTRO,
                .intro = {
                    .polarity = DY_POLARITY_POSITIVE,
                    .is_implicit = is_implicit,
                    .tag = DY_CORE_INTRO_SIMPLE,
                    .simple = {
                        .tag = DY_CORE_SIMPLE_DECISION,
                        .direction = DY_RIGHT,
                        .out = dy_core_expr_new(transformed_elim)
                    }
                }
            };
        }

        return res;
    }
}

dy_ternary_t dy_unfold_is_subtype_of_recursion(struct dy_core_ctx *ctx, struct dy_core_expr unfold_out, struct dy_core_recursion supertype, bool is_implicit, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    struct dy_core_expr supertype_wrap = {
        .tag = DY_CORE_EXPR_INTRO,
        .intro = {
            .polarity = DY_POLARITY_POSITIVE,
            .is_implicit = is_implicit,
            .tag = DY_CORE_INTRO_COMPLEX,
            .complex = {
                .tag = DY_CORE_COMPLEX_RECURSION,
                .recursion = supertype
            }
        }
    };

    struct dy_core_expr subst_supertype_expr;
    if (!dy_substitute(ctx, *supertype.expr, supertype.id, supertype_wrap, &subst_supertype_expr)) {
        subst_supertype_expr = dy_core_expr_retain(ctx, *supertype.expr);
    }

    struct dy_core_expr elim = {
        .tag = DY_CORE_EXPR_ELIM,
        .elim = {
            .check_result = DY_YES,
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .simple = {
                .tag = DY_CORE_SIMPLE_UNFOLD,
                .out = dy_core_expr_new(unfold_out)
            },
            .is_implicit = is_implicit,
            .eval_immediately = true
        }
    };

    struct dy_core_expr transformed_elim;
    bool did_transform_elim = false;
    dy_ternary_t res = dy_is_subtype(ctx, unfold_out, subst_supertype_expr, elim, &transformed_elim, &did_transform_elim);

    dy_core_expr_release(ctx, elim);

    if (did_transform_elim) {
        *did_transform_subtype_expr = true;
        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_INTRO,
            .intro = {
                .polarity = DY_POLARITY_POSITIVE,
                .is_implicit = is_implicit,
                .tag = DY_CORE_INTRO_SIMPLE,
                .simple = {
                    .tag = DY_CORE_SIMPLE_UNFOLD,
                    .out = dy_core_expr_new(transformed_elim)
                }
            }
        };
    }

    return res;
}

dy_ternary_t dy_implicit_assumption_is_subtype(struct dy_core_ctx *ctx, struct dy_core_assumption subtype, struct dy_core_expr supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
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

    struct dy_core_expr elim = {
        .tag = DY_CORE_EXPR_ELIM,
        .elim = {
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .simple = {
                .tag = DY_CORE_SIMPLE_PROOF,
                .proof = dy_core_expr_new(inference_id_expr),
                .out = dy_core_expr_new(type)
            },
            .is_implicit = true,
            .check_result = DY_MAYBE,
            .eval_immediately = true
        }
    };

    struct dy_core_expr transformed_elim;
    bool did_transform_elim = false;
    dy_ternary_t res = dy_is_subtype(ctx, type, supertype, elim, &transformed_elim, &did_transform_elim);

    if (did_transform_elim) {
        dy_core_expr_release(ctx, elim);
        *new_subtype_expr = transformed_elim;
    } else {
        *new_subtype_expr = elim;
    }

    *did_transform_subtype_expr = true;

    return res;
}

dy_ternary_t dy_implicit_choice_is_subtype(struct dy_core_ctx *ctx, struct dy_core_choice subtype, struct dy_core_expr supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    struct dy_core_expr elim_left = {
        .tag = DY_CORE_EXPR_ELIM,
        .elim = {
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .simple = {
                .tag = DY_CORE_SIMPLE_DECISION,
                .direction = DY_LEFT,
                .out = dy_core_expr_retain_ptr(ctx, subtype.left)
            },
            .is_implicit = true,
            .check_result = DY_YES,
            .eval_immediately = true
        }
    };

    size_t constraint_start1 = ctx->constraints.num_elems;

    size_t recovered_pos_inf_vars_start = ctx->recovered_positive_inference_ids.num_elems;
    size_t recovered_neg_inf_vars_start = ctx->recovered_negative_inference_ids.num_elems;

    struct dy_core_expr transformed_elim_left;
    bool did_transform_elim_left = false;
    dy_ternary_t res_left = dy_is_subtype(ctx, *subtype.left, supertype, elim_left, &transformed_elim_left, &did_transform_elim_left);

    if (did_transform_elim_left) {
        dy_core_expr_release(ctx, elim_left);
    } else {
        transformed_elim_left = elim_left;
    }

    if (res_left == DY_YES) {
        *new_subtype_expr = transformed_elim_left;
        *did_transform_subtype_expr = true;
        return DY_YES;
    }

    struct dy_core_expr elim_right = {
        .tag = DY_CORE_EXPR_ELIM,
        .elim = {
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .simple = {
                .tag = DY_CORE_SIMPLE_DECISION,
                .direction = DY_RIGHT,
                .out = dy_core_expr_retain_ptr(ctx, subtype.right)
            },
            .is_implicit = true,
            .check_result = DY_YES,
            .eval_immediately = true
        }
    };


    size_t constraint_start2 = ctx->constraints.num_elems;

    struct dy_core_expr transformed_elim_right;
    bool did_transform_elim_right = false;
    dy_ternary_t res_right = dy_is_subtype(ctx, *subtype.right, supertype, elim_right, &transformed_elim_right, &did_transform_elim_right);

    if (did_transform_elim_right) {
        dy_core_expr_release(ctx, elim_right);
    } else {
        transformed_elim_right = elim_right;
    }

    if (res_left == DY_NO && res_right == DY_NO) {
        dy_core_expr_release(ctx, transformed_elim_left);
        dy_core_expr_release(ctx, transformed_elim_right);
        return DY_NO;
    }

    if (res_right == DY_YES || res_left == DY_NO) {
        dy_free_constraints_in_range(ctx, constraint_start1, constraint_start2);

        ctx->recovered_positive_inference_ids.num_elems = recovered_pos_inf_vars_start;
        ctx->recovered_negative_inference_ids.num_elems = recovered_neg_inf_vars_start;

        dy_core_expr_release(ctx, transformed_elim_left);

        *new_subtype_expr = transformed_elim_right;
        *did_transform_subtype_expr = true;
        return res_right;
    }

    if (res_right == DY_NO) {
        dy_core_expr_release(ctx, transformed_elim_right);
        *new_subtype_expr = transformed_elim_left;
        *did_transform_subtype_expr = true;
        return res_left;
    }

    dy_free_constraints_in_range(ctx, constraint_start1, ctx->constraints.num_elems);

    ctx->recovered_positive_inference_ids.num_elems = recovered_pos_inf_vars_start;
    ctx->recovered_negative_inference_ids.num_elems = recovered_neg_inf_vars_start;

    dy_core_expr_release(ctx, transformed_elim_left);
    dy_core_expr_release(ctx, transformed_elim_right);

    return DY_NO;
}

dy_ternary_t dy_implicit_recursion_is_subtype(struct dy_core_ctx *ctx, struct dy_core_recursion subtype, struct dy_core_expr supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    dy_bail("not yet implemented");
}

dy_ternary_t dy_is_subtype_of_implicit_assumption(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_assumption supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    size_t inference_id = ctx->running_id++;

    dy_array_add(&ctx->recovered_positive_inference_ids, &inference_id);

    struct dy_core_expr inference_id_expr = {
        .tag = DY_CORE_EXPR_INFERENCE_VAR,
        .inference_var_id = inference_id
    };

    struct dy_core_expr type;
    if (!dy_substitute(ctx, *supertype.expr, supertype.id, inference_id_expr, &type)) {
        type = dy_core_expr_retain(ctx, *supertype.expr);
    }

    struct dy_core_expr transformed_expr;
    bool did_transform_expr = false;
    dy_ternary_t res = dy_is_subtype(ctx, subtype, type, subtype_expr, &transformed_expr, &did_transform_expr);

    dy_core_expr_release(ctx, type);

    if (!did_transform_expr) {
        transformed_expr = dy_core_expr_retain(ctx, subtype_expr);
    }

    if (res == DY_NO) {
        dy_core_expr_release(ctx, transformed_expr);
        return DY_NO;
    }

    *new_subtype_expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_INTRO,
        .intro = {
            .polarity = DY_POLARITY_POSITIVE,
            .is_implicit = true,
            .tag = DY_CORE_INTRO_SIMPLE,
            .simple = {
                .tag = DY_CORE_SIMPLE_PROOF,
                .proof = dy_core_expr_new(inference_id_expr),
                .out = dy_core_expr_new(transformed_expr)
            }
        }
    };

    *did_transform_subtype_expr = true;

    return res;
}

dy_ternary_t dy_is_subtype_of_implicit_choice(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_choice supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    size_t constraint_start1 = ctx->constraints.num_elems;

    size_t recovered_pos_inf_vars_start = ctx->recovered_positive_inference_ids.num_elems;
    size_t recovered_neg_inf_vars_start = ctx->recovered_negative_inference_ids.num_elems;

    struct dy_core_expr transformed_left;
    bool did_transform_left = false;
    dy_ternary_t res_left = dy_is_subtype(ctx, subtype, *supertype.left, subtype_expr, &transformed_left, &did_transform_left);

    if (!did_transform_left) {
        transformed_left = dy_core_expr_retain(ctx, subtype_expr);
    }

    if (res_left == DY_YES) {
        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_INTRO,
            .intro = {
                .polarity = DY_POLARITY_POSITIVE,
                .is_implicit = true,
                .tag = DY_CORE_INTRO_SIMPLE,
                .simple = {
                    .tag = DY_CORE_SIMPLE_DECISION,
                    .direction = DY_LEFT,
                    .out = dy_core_expr_new(transformed_left)
                }
            }
        };
        *did_transform_subtype_expr = true;
        return DY_YES;
    }

    size_t constraint_start2 = ctx->constraints.num_elems;

    struct dy_core_expr transformed_right;
    bool did_transform_right = false;
    dy_ternary_t res_right = dy_is_subtype(ctx, subtype, *supertype.right, subtype_expr, &transformed_right, &did_transform_right);

    if (!did_transform_right) {
        transformed_right = dy_core_expr_retain(ctx, subtype_expr);
    }

    if (res_left == DY_NO && res_right == DY_NO) {
        dy_core_expr_release(ctx, transformed_left);
        dy_core_expr_release(ctx, transformed_right);
        return DY_NO;
    }

    if (res_right == DY_YES || res_left == DY_NO) {
        dy_free_constraints_in_range(ctx, constraint_start1, constraint_start2);

        ctx->recovered_positive_inference_ids.num_elems = recovered_pos_inf_vars_start;
        ctx->recovered_negative_inference_ids.num_elems = recovered_neg_inf_vars_start;

        dy_core_expr_release(ctx, transformed_left);

        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_INTRO,
            .intro = {
                .polarity = DY_POLARITY_POSITIVE,
                .is_implicit = true,
                .tag = DY_CORE_INTRO_SIMPLE,
                .simple = {
                    .tag = DY_CORE_SIMPLE_DECISION,
                    .direction = DY_RIGHT,
                    .out = dy_core_expr_new(transformed_right)
                }
            }
        };
        *did_transform_subtype_expr = true;
        return res_right;
    }

    if (res_right == DY_NO) {
        dy_core_expr_release(ctx, transformed_right);

        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_INTRO,
            .intro = {
                .polarity = DY_POLARITY_POSITIVE,
                .is_implicit = true,
                .tag = DY_CORE_INTRO_SIMPLE,
                .simple = {
                    .tag = DY_CORE_SIMPLE_DECISION,
                    .direction = DY_LEFT,
                    .out = dy_core_expr_new(transformed_left)
                }
            }
        };
        *did_transform_subtype_expr = true;
        return res_left;
    }

    dy_free_constraints_in_range(ctx, constraint_start1, ctx->constraints.num_elems);

    ctx->recovered_positive_inference_ids.num_elems = recovered_pos_inf_vars_start;
    ctx->recovered_negative_inference_ids.num_elems = recovered_neg_inf_vars_start;

    dy_core_expr_release(ctx, transformed_left);
    dy_core_expr_release(ctx, transformed_right);

    return DY_NO;
}

dy_ternary_t dy_is_subtype_of_implicit_recursion(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_recursion supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    dy_bail("not yet implemented");
}
