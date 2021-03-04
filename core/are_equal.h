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

static inline dy_ternary_t dy_assumptions_are_equal(struct dy_core_ctx *ctx, struct dy_core_assumption ass1, struct dy_core_assumption ass2);

static inline dy_ternary_t dy_choices_are_equal(struct dy_core_ctx *ctx, struct dy_core_choice choice1, struct dy_core_choice choice2);

static inline dy_ternary_t dy_recursions_are_equal(struct dy_core_ctx *ctx, struct dy_core_recursion rec1, struct dy_core_recursion rec2);

static inline dy_ternary_t dy_simples_are_equal(struct dy_core_ctx *ctx, struct dy_core_simple simple1, struct dy_core_simple simple2);

static inline dy_ternary_t dy_elims_are_equal(struct dy_core_ctx *ctx, struct dy_core_elim elim1, struct dy_core_elim elim2);

static inline dy_ternary_t dy_variables_are_equal(struct dy_core_ctx *ctx, size_t id1, size_t id2);

static inline dy_ternary_t dy_maps_are_equal(struct dy_core_ctx *ctx, struct dy_core_map f1, struct dy_core_map f2);

static inline dy_ternary_t dy_map_assumption_are_equal(struct dy_core_ctx *ctx, struct dy_core_map_assumption ass1, struct dy_core_map_assumption ass2);

static inline dy_ternary_t dy_map_choice_are_equal(struct dy_core_ctx *ctx, struct dy_core_map_choice choic1, struct dy_core_map_choice choice2);

static inline dy_ternary_t dy_map_recursion_are_equal(struct dy_core_ctx *ctx, struct dy_core_map_recursion rec1, struct dy_core_map_recursion rec2);

dy_ternary_t dy_are_equal(struct dy_core_ctx *ctx, struct dy_core_expr e1, struct dy_core_expr e2)
{
    if (e1.tag == DY_CORE_EXPR_INTRO && e2.tag == DY_CORE_EXPR_INTRO) {
        if (e1.intro.is_implicit != e2.intro.is_implicit || e1.intro.polarity != e2.intro.polarity || e1.intro.tag != e2.intro.tag) {
            return DY_NO;
        }

        switch (e1.intro.tag) {
        case DY_CORE_INTRO_COMPLEX:
            if (e1.intro.complex.tag != e2.intro.complex.tag) {
                return DY_NO;
            }

            switch (e1.intro.complex.tag) {
            case DY_CORE_COMPLEX_ASSUMPTION:
                return dy_assumptions_are_equal(ctx, e1.intro.complex.assumption, e2.intro.complex.assumption);
            case DY_CORE_COMPLEX_CHOICE:
                return dy_choices_are_equal(ctx, e1.intro.complex.choice, e2.intro.complex.choice);
            case DY_CORE_COMPLEX_RECURSION:
                return dy_recursions_are_equal(ctx, e1.intro.complex.recursion, e2.intro.complex.recursion);
            }

            dy_bail("impossible");
        case DY_CORE_INTRO_SIMPLE:
            return dy_simples_are_equal(ctx, e1.intro.simple, e2.intro.simple);
        }

        dy_bail("impossible");
    }

    if (e1.tag == DY_CORE_EXPR_ELIM && e2.tag == DY_CORE_EXPR_ELIM) {
        return dy_elims_are_equal(ctx, e1.elim, e2.elim);
    }

    if (e1.tag == DY_CORE_EXPR_MAP && e2.tag == DY_CORE_EXPR_MAP) {
        return dy_maps_are_equal(ctx, e1.map, e2.map);
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

    if (e1.tag == DY_CORE_EXPR_ELIM || e2.tag == DY_CORE_EXPR_ELIM || e1.tag == DY_CORE_EXPR_VARIABLE || e2.tag == DY_CORE_EXPR_VARIABLE || e1.tag == DY_CORE_EXPR_INFERENCE_VAR || e2.tag == DY_CORE_EXPR_INFERENCE_VAR) {
        return DY_MAYBE;
    } else {
        return DY_NO;
    }
}

dy_ternary_t dy_assumptions_are_equal(struct dy_core_ctx *ctx, struct dy_core_assumption ass1, struct dy_core_assumption ass2)
{
    dy_array_add(&ctx->equal_variables, &(struct dy_equal_variables){
        .id1 = ass1.id,
        .id2 = ass2.id
    });

    dy_ternary_t res1 = dy_are_equal(ctx, *ass1.type, *ass2.type);

    dy_ternary_t res2 = dy_are_equal(ctx, *ass1.expr, *ass2.expr);

    --ctx->equal_variables.num_elems;

    if (res1 == DY_NO || res2 == DY_NO) {
        return DY_NO;
    }

    if (res1 == DY_MAYBE || res2 == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

dy_ternary_t dy_choices_are_equal(struct dy_core_ctx *ctx, struct dy_core_choice choice1, struct dy_core_choice choice2)
{
    dy_ternary_t res1 = dy_are_equal(ctx, *choice1.left, *choice2.left);

    dy_ternary_t res2 = dy_are_equal(ctx, *choice1.right, *choice2.right);

    if (res1 == DY_NO || res2 == DY_NO) {
        return DY_NO;
    }

    if (res1 == DY_MAYBE || res2 == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
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

dy_ternary_t dy_simples_are_equal(struct dy_core_ctx *ctx, struct dy_core_simple simple1, struct dy_core_simple simple2)
{
    if (simple1.tag != simple2.tag) {
        return DY_NO;
    }

    dy_ternary_t ret = DY_YES;
    if (simple1.tag == DY_CORE_SIMPLE_PROOF) {
        ret = dy_are_equal(ctx, *simple1.proof, *simple2.proof);
    }

    dy_ternary_t ret2 = dy_are_equal(ctx, *simple1.out, *simple2.out);

    if (ret == DY_NO || ret2 == DY_NO) {
        return DY_NO;
    }

    if (ret == DY_MAYBE || ret2 == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

dy_ternary_t dy_elims_are_equal(struct dy_core_ctx *ctx, struct dy_core_elim elim1, struct dy_core_elim elim2)
{
    if (dy_are_equal(ctx, *elim1.expr, *elim2.expr) != DY_YES) {
        return DY_MAYBE;
    }

    if (elim1.is_implicit != elim2.is_implicit) {
        return DY_MAYBE;
    }

    if (dy_simples_are_equal(ctx, elim1.simple, elim2.simple) != DY_YES) {
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

dy_ternary_t dy_maps_are_equal(struct dy_core_ctx *ctx, struct dy_core_map f1, struct dy_core_map f2)
{
    if (f1.tag != f2.tag || f1.is_implicit != f2.is_implicit) {
        return DY_NO;
    }

    switch (f1.tag) {
    case DY_CORE_MAP_ASSUMPTION:
        return dy_map_assumption_are_equal(ctx, f1.assumption, f2.assumption);
    case DY_CORE_MAP_CHOICE:
        return dy_map_choice_are_equal(ctx, f1.choice, f2.choice);
    case DY_CORE_MAP_RECURSION:
        return dy_map_recursion_are_equal(ctx, f1.recursion, f2.recursion);
    }

    dy_bail("impossible");
}

dy_ternary_t dy_map_assumption_are_equal(struct dy_core_ctx *ctx, struct dy_core_map_assumption ass1, struct dy_core_map_assumption ass2)
{
    dy_array_add(&ctx->equal_variables, &(struct dy_equal_variables){
        .id1 = ass1.id,
        .id2 = ass2.id
    });

    dy_ternary_t res1 = dy_are_equal(ctx, *ass1.type, *ass2.type);

    dy_ternary_t res2 = dy_assumptions_are_equal(ctx, ass1.assumption, ass2.assumption);

    --ctx->equal_variables.num_elems;

    if (res1 == DY_NO || res2 == DY_NO) {
        return DY_NO;
    }

    if (res1 == DY_MAYBE || res2 == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

dy_ternary_t dy_map_choice_are_equal(struct dy_core_ctx *ctx, struct dy_core_map_choice choice1, struct dy_core_map_choice choice2)
{
    dy_ternary_t res1 = dy_assumptions_are_equal(ctx, choice1.assumption_left, choice2.assumption_left);

    dy_ternary_t res2 = dy_assumptions_are_equal(ctx, choice1.assumption_right, choice2.assumption_right);

    if (res1 == DY_NO || res2 == DY_NO) {
        return DY_NO;
    }

    if (res1 == DY_MAYBE || res2 == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

dy_ternary_t dy_map_recursion_are_equal(struct dy_core_ctx *ctx, struct dy_core_map_recursion rec1, struct dy_core_map_recursion rec2)
{
    dy_array_add(&ctx->equal_variables, &(struct dy_equal_variables){
        .id1 = rec1.id,
        .id2 = rec2.id
    });

    dy_ternary_t res = dy_assumptions_are_equal(ctx, rec1.assumption, rec2.assumption);

    --ctx->equal_variables.num_elems;

    return res;
}
