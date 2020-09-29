/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
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
 * A subtype check does three things:
 *   (1) Check whether the supposed subtype actually is a subtype of the supposed supertype.
 *       This results in a ternary result.
 *   (2) Collect constraints generated while doing (1).
 *   (3) Transform the expression that is of type 'subtype' according to the information recovered during (2).
 */

static inline dy_ternary_t dy_is_subtype(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_expr supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t equality_map_is_subtype_of_equality_map(struct dy_core_ctx *ctx, struct dy_core_equality_map emap1, struct dy_core_equality_map emap2, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t positive_equality_map_is_subtype_of_negative_type_map(struct dy_core_ctx *ctx, struct dy_core_equality_map emap, struct dy_core_type_map tmap, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t positive_type_map_is_subtype_of_negative_equality_map(struct dy_core_ctx *ctx, struct dy_core_type_map type_map, struct dy_core_equality_map equality_map, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t type_map_is_subtype_of_type_map(struct dy_core_ctx *ctx, struct dy_core_type_map tmap1, struct dy_core_type_map tmap2, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t positive_implicit_type_map_is_subtype(struct dy_core_ctx *ctx, struct dy_core_type_map type_map, struct dy_core_expr supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t positive_junction_is_subtype(struct dy_core_ctx *ctx, struct dy_core_junction junction, struct dy_core_expr expr, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);
static inline dy_ternary_t negative_junction_is_subtype(struct dy_core_ctx *ctx, struct dy_core_junction junction, struct dy_core_expr expr, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t is_subtype_of_positive_junction(struct dy_core_ctx *ctx, struct dy_core_expr expr, struct dy_core_junction junction, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);
static inline dy_ternary_t is_subtype_of_negative_junction(struct dy_core_ctx *ctx, struct dy_core_expr expr, struct dy_core_junction junction, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t positive_recursion_is_subtype(struct dy_core_ctx *ctx, struct dy_core_recursion rec, struct dy_core_expr supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t negative_recursion_is_subtype(struct dy_core_ctx *ctx, struct dy_core_recursion rec, struct dy_core_expr supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t is_subtype_of_positive_recursion(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_recursion rec, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t is_subtype_of_negative_recursion(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_recursion rec, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static inline dy_ternary_t dy_is_subtype_no_transformation(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_expr supertype);

static inline bool dy_is_inference_var(struct dy_core_ctx *ctx, size_t id, enum dy_core_polarity *polarity);

dy_ternary_t dy_is_subtype(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_expr supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    if (subtype.tag == DY_CORE_EXPR_RECURSION && supertype.tag == DY_CORE_EXPR_RECURSION && subtype.recursion.polarity == supertype.recursion.polarity && subtype.recursion.id == supertype.recursion.id) {
        return dy_is_subtype(ctx, *subtype.recursion.expr, *supertype.recursion.expr, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
    }
    
    if (subtype.tag == DY_CORE_EXPR_RECURSION && subtype.recursion.polarity == DY_CORE_POLARITY_POSITIVE) {
        return positive_recursion_is_subtype(ctx, subtype.recursion, supertype, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
    }

    if (subtype.tag == DY_CORE_EXPR_RECURSION && subtype.recursion.polarity == DY_CORE_POLARITY_NEGATIVE) {
        return negative_recursion_is_subtype(ctx, subtype.recursion, supertype, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
    }
    
    if (supertype.tag == DY_CORE_EXPR_RECURSION && supertype.recursion.polarity == DY_CORE_POLARITY_POSITIVE) {
        return is_subtype_of_positive_recursion(ctx, subtype, supertype.recursion, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
    }

    if (supertype.tag == DY_CORE_EXPR_RECURSION && supertype.recursion.polarity == DY_CORE_POLARITY_NEGATIVE) {
        return is_subtype_of_negative_recursion(ctx, subtype, supertype.recursion, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
    }
    
    if (supertype.tag == DY_CORE_EXPR_JUNCTION && supertype.junction.polarity == DY_CORE_POLARITY_POSITIVE) {
        return is_subtype_of_positive_junction(ctx, subtype, supertype.junction, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
    }
    
    if (subtype.tag == DY_CORE_EXPR_JUNCTION && subtype.junction.polarity == DY_CORE_POLARITY_NEGATIVE) {
        return negative_junction_is_subtype(ctx, subtype.junction, supertype, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
    }

    if (supertype.tag == DY_CORE_EXPR_JUNCTION && supertype.junction.polarity == DY_CORE_POLARITY_NEGATIVE) {
        return is_subtype_of_negative_junction(ctx, subtype, supertype.junction, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
    }

    if (subtype.tag == DY_CORE_EXPR_JUNCTION && subtype.junction.polarity == DY_CORE_POLARITY_POSITIVE) {
        return positive_junction_is_subtype(ctx, subtype.junction, supertype, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
    }
    
    if (subtype.tag == DY_CORE_EXPR_VARIABLE) {
        if (supertype.tag == DY_CORE_EXPR_VARIABLE && subtype.variable_id == supertype.variable_id) {
            return DY_YES;
        }
        
        enum dy_core_polarity polarity;
        if (dy_is_inference_var(ctx, subtype.variable_id, &polarity) && polarity == DY_CORE_POLARITY_NEGATIVE) {
            dy_array_add(&ctx->constraints, &(struct dy_constraint){
                .id = subtype.variable_id,
                .expr = dy_core_expr_retain(ctx, supertype),
                .polarity = DY_CORE_POLARITY_NEGATIVE
            });
        }
        
        return DY_MAYBE;
    }
    
    if (supertype.tag == DY_CORE_EXPR_VARIABLE) {
        if (subtype.tag == DY_CORE_EXPR_VARIABLE && subtype.variable_id == supertype.variable_id) {
            return DY_YES;
        }
        
        enum dy_core_polarity polarity;
        if (dy_is_inference_var(ctx, supertype.variable_id, &polarity) && polarity == DY_CORE_POLARITY_POSITIVE) {
            dy_array_add(&ctx->constraints, &(struct dy_constraint){
                .id = supertype.variable_id,
                .expr = dy_core_expr_retain(ctx, subtype),
                .polarity = DY_CORE_POLARITY_POSITIVE
            });
        }
        
        return DY_MAYBE;
    }

    if (supertype.tag == DY_CORE_EXPR_END && supertype.end_polarity == DY_CORE_POLARITY_NEGATIVE) {
        return DY_YES;
    }

    if (subtype.tag == DY_CORE_EXPR_END && subtype.end_polarity == DY_CORE_POLARITY_POSITIVE) {
        return DY_YES;
    }

    if (supertype.tag == DY_CORE_EXPR_EQUALITY_MAP_ELIM || supertype.tag == DY_CORE_EXPR_TYPE_MAP_ELIM || supertype.tag == DY_CORE_EXPR_VARIABLE || supertype.tag == DY_CORE_EXPR_ALTERNATIVE || supertype.tag == DY_CORE_EXPR_SYMBOL) {
        return dy_are_equal(ctx, subtype, supertype);
    }

    if (subtype.tag == DY_CORE_EXPR_EQUALITY_MAP_ELIM || subtype.tag == DY_CORE_EXPR_TYPE_MAP_ELIM || subtype.tag == DY_CORE_EXPR_VARIABLE || subtype.tag == DY_CORE_EXPR_ALTERNATIVE || subtype.tag == DY_CORE_EXPR_SYMBOL) {
        return dy_are_equal(ctx, subtype, supertype);
    }

    if (subtype.tag == DY_CORE_EXPR_END && subtype.end_polarity == DY_CORE_POLARITY_NEGATIVE) {
        return DY_MAYBE;
    }

    if (supertype.tag == DY_CORE_EXPR_END && supertype.end_polarity == DY_CORE_POLARITY_POSITIVE) {
        return DY_NO;
    }

    if (subtype.tag == DY_CORE_EXPR_EQUALITY_MAP) {
        if (subtype.equality_map.polarity == DY_CORE_POLARITY_POSITIVE) {
            if (supertype.tag == DY_CORE_EXPR_EQUALITY_MAP && subtype.equality_map.is_implicit == supertype.equality_map.is_implicit) {
                return equality_map_is_subtype_of_equality_map(ctx, subtype.equality_map, supertype.equality_map, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
            }

            if (supertype.tag == DY_CORE_EXPR_TYPE_MAP && supertype.type_map.polarity == DY_CORE_POLARITY_NEGATIVE && subtype.equality_map.is_implicit == supertype.type_map.is_implicit) {
                return positive_equality_map_is_subtype_of_negative_type_map(ctx, subtype.equality_map, supertype.type_map, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
            }

            return DY_NO;
        } else {
            if (supertype.tag == DY_CORE_EXPR_EQUALITY_MAP && supertype.equality_map.polarity == DY_CORE_POLARITY_NEGATIVE && subtype.equality_map.is_implicit == supertype.equality_map.is_implicit) {
                return equality_map_is_subtype_of_equality_map(ctx, subtype.equality_map, supertype.equality_map, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
            }

            return DY_NO;
        }
    }

    if (subtype.tag == DY_CORE_EXPR_TYPE_MAP) {
        if (subtype.type_map.polarity == DY_CORE_POLARITY_POSITIVE) {
            if (supertype.tag == DY_CORE_EXPR_EQUALITY_MAP && supertype.equality_map.polarity == DY_CORE_POLARITY_NEGATIVE && subtype.type_map.is_implicit == supertype.equality_map.is_implicit) {
                return positive_type_map_is_subtype_of_negative_equality_map(ctx, subtype.type_map, supertype.equality_map, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
            }

            if (supertype.tag == DY_CORE_EXPR_TYPE_MAP && supertype.type_map.polarity == DY_CORE_POLARITY_POSITIVE && subtype.type_map.is_implicit == supertype.type_map.is_implicit) {
                return type_map_is_subtype_of_type_map(ctx, subtype.type_map, supertype.type_map, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
            }

            if (subtype.type_map.is_implicit) {
                return positive_implicit_type_map_is_subtype(ctx, subtype.type_map, supertype, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
            }

            return DY_NO;
        } else {
            if (supertype.tag == DY_CORE_EXPR_TYPE_MAP && supertype.type_map.polarity == DY_CORE_POLARITY_NEGATIVE && subtype.type_map.is_implicit == supertype.type_map.is_implicit) {
                return type_map_is_subtype_of_type_map(ctx, subtype.type_map, supertype.type_map, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
            }

            // TODO: Handle implicit negative type map as supertype.

            return DY_NO;
        }
    }

    if (subtype.tag == DY_CORE_EXPR_CUSTOM) {
        const struct dy_core_custom_shared *s = dy_array_pos(ctx->custom_shared, subtype.custom.id);
        return s->is_subtype(subtype.custom.data, ctx, supertype, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
    }

    if (supertype.tag == DY_CORE_EXPR_CUSTOM) {
        const struct dy_core_custom_shared *s = dy_array_pos(ctx->custom_shared, supertype.custom.id);
        return s->is_supertype(supertype.custom.data, ctx, subtype, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
    }

    dy_bail("Should be unreachable!");
}

/*
 * Implements the following:
 *
 * (x : e1 -> e2) <: e3 -> e4 (or e3 ~> e4)
 *
 * e1 = e3
 *
 * e2 <: e4, f
 *
 * x => e1 -> f (x e1)
 */
dy_ternary_t equality_map_is_subtype_of_equality_map(struct dy_core_ctx *ctx, struct dy_core_equality_map equality_map1, struct dy_core_equality_map equality_map2, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    dy_ternary_t are_equal = dy_are_equal(ctx, *equality_map1.e1, *equality_map2.e1);
    if (are_equal == DY_NO) {
        return DY_NO;
    }

    struct dy_core_expr subtype_expr_emap_e2 = {
        .tag = DY_CORE_EXPR_EQUALITY_MAP_ELIM,
        .equality_map_elim = {
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .map = {
                .e1 = dy_core_expr_retain_ptr(equality_map1.e1),
                .e2 = dy_core_expr_retain_ptr(equality_map1.e2),
                .polarity = DY_CORE_POLARITY_NEGATIVE,
                .is_implicit = equality_map1.is_implicit,
            },
            .check_result = DY_MAYBE,
        }
    };

    struct dy_core_expr new_subtype_expr_emap_e2;
    bool did_transform_subtype_expr_emap_e2 = false;
    dy_ternary_t is_subtype = dy_is_subtype(ctx, *equality_map1.e2, *equality_map2.e2, subtype_expr_emap_e2, &new_subtype_expr_emap_e2, &did_transform_subtype_expr_emap_e2);

    if (did_transform_subtype_expr_emap_e2) {
        dy_core_expr_release(ctx, subtype_expr_emap_e2);
    } else {
        new_subtype_expr_emap_e2 = subtype_expr_emap_e2;
    }

    if (is_subtype == DY_NO) {
        dy_core_expr_release(ctx, new_subtype_expr_emap_e2);
        return DY_NO;
    }

    if (did_transform_subtype_expr_emap_e2) {
        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_EQUALITY_MAP,
            .equality_map = {
                .e1 = dy_core_expr_retain_ptr(equality_map1.e1),
                .e2 = dy_core_expr_new(new_subtype_expr_emap_e2),
                .polarity = DY_CORE_POLARITY_POSITIVE,
                .is_implicit = equality_map1.is_implicit,
            }
        };

        *did_transform_subtype_expr = true;
    } else {
        dy_core_expr_release(ctx, new_subtype_expr_emap_e2);
    }

    if (are_equal == DY_MAYBE || is_subtype == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

/*
 * Implements the following:
 *
 * (x : e1 -> e2) <: [v e3] ~> e4
 *
 * ty e1 <: e3, f
 *
 * e2 <: e4, g
 *
 * x => f e1 -> g (x e1)
 */
dy_ternary_t positive_equality_map_is_subtype_of_negative_type_map(struct dy_core_ctx *ctx, struct dy_core_equality_map equality_map, struct dy_core_type_map type_map, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    struct dy_core_expr type_of_equality_map_e1 = dy_type_of(ctx, *equality_map.e1);

    size_t constraint_start1 = ctx->constraints.num_elems;
    struct dy_core_expr emap_e1 = *equality_map.e1;
    bool did_transform_emap_e1 = false;
    struct dy_core_expr new_emap_e1;
    dy_ternary_t is_subtype_in = dy_is_subtype(ctx, type_of_equality_map_e1, *type_map.type, emap_e1, &new_emap_e1, &did_transform_emap_e1);

    dy_core_expr_release(ctx, type_of_equality_map_e1);

    if (is_subtype_in == DY_NO) {
        return DY_NO;
    }

    if (!did_transform_emap_e1) {
        new_emap_e1 = dy_core_expr_retain(ctx, emap_e1);
    }

    struct dy_core_expr emap_e2 = {
        .tag = DY_CORE_EXPR_EQUALITY_MAP_ELIM,
        .equality_map_elim = {
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .map = {
                .e1 = dy_core_expr_retain_ptr(equality_map.e1),
                .e2 = dy_core_expr_retain_ptr(equality_map.e2),
                .polarity = DY_CORE_POLARITY_NEGATIVE,
                .is_implicit = equality_map.is_implicit,
            },
            .check_result = DY_MAYBE,
        }
    };

    dy_array_add(&ctx->bindings, &(struct dy_core_binding){
        .id = type_map.id,
        .type = *type_map.type,
        .is_inference_var = false
    });

    size_t constraint_start2 = ctx->constraints.num_elems;
    struct dy_core_expr new_emap_e2;
    bool did_transform_emap_e2 = false;
    dy_ternary_t is_subtype_out = dy_is_subtype(ctx, *equality_map.e2, *type_map.expr, emap_e2, &new_emap_e2, &did_transform_emap_e2);

    --ctx->bindings.num_elems;

    if (is_subtype_out == DY_NO) {
        dy_free_first_constraints(ctx, constraint_start1, ctx->constraints.num_elems);
        dy_core_expr_release(ctx, new_emap_e1);
        dy_core_expr_release(ctx, emap_e2);
        return DY_NO;
    }

    if (did_transform_emap_e2) {
        dy_core_expr_release(ctx, emap_e2);
    } else {
        new_emap_e2 = emap_e2;
    }

    if (did_transform_emap_e1 || did_transform_emap_e2) {
        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_EQUALITY_MAP,
            .equality_map = {
                .e1 = dy_core_expr_new(new_emap_e1),
                .e2 = dy_core_expr_new(new_emap_e2),
                .polarity = equality_map.polarity,
                .is_implicit = equality_map.is_implicit,
            }
        };

        *did_transform_subtype_expr = true;
    } else {
        dy_core_expr_release(ctx, new_emap_e1);
        dy_core_expr_release(ctx, new_emap_e2);
    }

    dy_join_constraints(ctx, constraint_start1, constraint_start2, DY_CORE_POLARITY_POSITIVE);

    if (is_subtype_in == DY_MAYBE || is_subtype_out == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

/*
 * Implements the following:
 *
 * (x : [v e1] -> e2) <: e3 ~> e4
 *
 * ty e3 <: e1, f
 *
 * e2 <: e4, g
 *
 * x => e3 -> g (x (f e3))
 */
dy_ternary_t positive_type_map_is_subtype_of_negative_equality_map(struct dy_core_ctx *ctx, struct dy_core_type_map type_map, struct dy_core_equality_map equality_map, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    struct dy_core_expr type_of_supertype_e1 = dy_type_of(ctx, *equality_map.e1);

    size_t constraint_start1 = ctx->constraints.num_elems;
    struct dy_core_expr new_supertype_e1;
    bool did_transform_supertype_e1 = false;
    dy_ternary_t is_subtype_in = dy_is_subtype(ctx, type_of_supertype_e1, *type_map.type, *equality_map.e1, &new_supertype_e1, &did_transform_supertype_e1);

    dy_core_expr_release(ctx, type_of_supertype_e1);

    if (is_subtype_in == DY_NO) {
        return DY_NO;
    }

    if (!did_transform_supertype_e1) {
        new_supertype_e1 = dy_core_expr_retain(ctx, *equality_map.e1);
    }

    struct dy_core_expr new_type_map_expr;
    if (!substitute(ctx, *type_map.expr, type_map.id, new_supertype_e1, &new_type_map_expr)) {
        new_type_map_expr = dy_core_expr_retain(ctx, *type_map.expr);
    }

    struct dy_core_expr emap_e2 = {
        .tag = DY_CORE_EXPR_EQUALITY_MAP_ELIM,
        .equality_map_elim = {
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .map = {
                .e1 = dy_core_expr_new(new_supertype_e1),
                .e2 = dy_core_expr_new(new_type_map_expr),
                .polarity = DY_CORE_POLARITY_NEGATIVE,
                .is_implicit = type_map.is_implicit,
            },
            .check_result = DY_MAYBE,
        }
    };

    dy_array_add(&ctx->bindings, &(struct dy_core_binding){
        .id = type_map.id,
        .type = *type_map.type,
        .is_inference_var = false
    });

    size_t constraint_start2 = ctx->constraints.num_elems;
    struct dy_core_expr new_emap_e2;
    bool did_transform_emap_e2 = false;
    dy_ternary_t is_subtype_out = dy_is_subtype(ctx, new_type_map_expr, *equality_map.e2, emap_e2, &new_emap_e2, &did_transform_emap_e2);

    --ctx->bindings.num_elems;

    if (is_subtype_out == DY_NO) {
        dy_free_first_constraints(ctx, constraint_start1, ctx->constraints.num_elems);
        dy_core_expr_release(ctx, emap_e2);
        return DY_NO;
    }

    if (did_transform_emap_e2) {
        dy_core_expr_release(ctx, emap_e2);
    } else {
        new_emap_e2 = emap_e2;
    }

    if (did_transform_supertype_e1 || did_transform_emap_e2) {
        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_EQUALITY_MAP,
            .equality_map = {
                .e1 = dy_core_expr_retain_ptr(equality_map.e1),
                .e2 = dy_core_expr_new(new_emap_e2),
                .polarity = DY_CORE_POLARITY_POSITIVE,
                .is_implicit = type_map.is_implicit,
            }
        };

        *did_transform_subtype_expr = true;
    } else {
        dy_core_expr_release(ctx, new_emap_e2);
    }

    dy_join_constraints(ctx, constraint_start1, constraint_start2, DY_CORE_POLARITY_POSITIVE);

    if (is_subtype_in == DY_MAYBE || is_subtype_out == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
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
dy_ternary_t type_map_is_subtype_of_type_map(struct dy_core_ctx *ctx, struct dy_core_type_map type_map1, struct dy_core_type_map type_map2, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    struct dy_core_expr var_expr = {
        .tag = DY_CORE_EXPR_VARIABLE,
        .variable_id = type_map2.id
    };

    size_t constraint_start1 = ctx->constraints.num_elems;
    struct dy_core_expr new_var_expr;
    bool did_transform_var_expr = false;
    dy_ternary_t is_subtype_in = dy_is_subtype(ctx, *type_map2.type, *type_map1.type, var_expr, &new_var_expr, &did_transform_var_expr);
    if (is_subtype_in == DY_NO) {
        return DY_NO;
    }

    if (!did_transform_var_expr) {
        new_var_expr = dy_core_expr_retain(ctx, var_expr);
    }

    struct dy_core_expr tmap_e2 = {
        .tag = DY_CORE_EXPR_EQUALITY_MAP_ELIM,
        .equality_map_elim = {
            .expr = dy_core_expr_new(dy_core_expr_retain(ctx, subtype_expr)),
            .map = {
                .e1 = dy_core_expr_new(new_var_expr),
                .e2 = dy_core_expr_retain_ptr(type_map1.expr),
                .polarity = DY_CORE_POLARITY_NEGATIVE,
                .is_implicit = type_map1.is_implicit,
            },
            .check_result = DY_MAYBE,
        }
    };

    dy_array_add(&ctx->equal_variables, &(struct dy_equal_variables){
        .id1 = type_map1.id,
        .id2 = type_map2.id
    });

    dy_array_add(&ctx->bindings, &(struct dy_core_binding){
        .id = type_map1.id,
        .type = *type_map1.type,
        .is_inference_var = false
    });

    dy_array_add(&ctx->bindings, &(struct dy_core_binding){
        .id = type_map2.id,
        .type = *type_map2.type,
        .is_inference_var = false
    });

    size_t constraint_start2 = ctx->constraints.num_elems;
    struct dy_core_expr new_tmap_e2;
    bool did_transform_tmap_e2 = false;
    dy_ternary_t is_subtype_out = dy_is_subtype(ctx, *type_map1.expr, *type_map2.expr, tmap_e2, &new_tmap_e2, &did_transform_tmap_e2);

    --ctx->equal_variables.num_elems;
    --ctx->bindings.num_elems;
    --ctx->bindings.num_elems;

    if (is_subtype_out == DY_NO) {
        dy_free_first_constraints(ctx, constraint_start1, ctx->constraints.num_elems);
        dy_core_expr_release(ctx, tmap_e2);
        return DY_NO;
    }

    if (did_transform_tmap_e2) {
        dy_core_expr_release(ctx, tmap_e2);
    } else {
        new_tmap_e2 = tmap_e2;
    }

    if (did_transform_var_expr || did_transform_tmap_e2) {
        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_TYPE_MAP,
            .type_map = {
                .id = type_map2.id,
                .type = dy_core_expr_retain_ptr(type_map2.type),
                .expr = dy_core_expr_new(new_tmap_e2),
                .polarity = DY_CORE_POLARITY_POSITIVE,
                .is_implicit = type_map1.is_implicit,
            }
        };

        *did_transform_subtype_expr = true;
    } else {
        dy_core_expr_release(ctx, new_tmap_e2);
    }

    dy_join_constraints(ctx, constraint_start1, constraint_start2, DY_CORE_POLARITY_POSITIVE);

    if (is_subtype_in == DY_MAYBE || is_subtype_out == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

dy_ternary_t positive_implicit_type_map_is_subtype(struct dy_core_ctx *ctx, struct dy_core_type_map type_map, struct dy_core_expr supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    size_t id = ctx->running_id++;

    dy_array_add(&ctx->subtype_implicits, &(struct dy_core_binding){
        .id = id,
        .type = dy_core_expr_retain(ctx, *type_map.type)
    });

    struct dy_core_expr unknown = {
        .tag = DY_CORE_EXPR_VARIABLE,
        .variable_id = id
    };

    struct dy_core_expr equality_map_expr = {
        .tag = DY_CORE_EXPR_EQUALITY_MAP,
        .equality_map = {
            .e1 = dy_core_expr_new(unknown),
            .e2 = dy_core_expr_new(dy_core_expr_retain(ctx, supertype)),
            .polarity = DY_CORE_POLARITY_NEGATIVE,
            .is_implicit = true,
        }
    };

    struct dy_core_expr type_map_expr = {
        .tag = DY_CORE_EXPR_TYPE_MAP,
        .type_map = type_map
    };

    struct dy_core_expr e;
    bool did_transform_e = false;
    dy_ternary_t res = dy_is_subtype(ctx, type_map_expr, equality_map_expr, subtype_expr, &e, &did_transform_e);

    if (!did_transform_e) {
        e = dy_core_expr_retain(ctx, subtype_expr);
    }

    *new_subtype_expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_EQUALITY_MAP_ELIM,
        .equality_map_elim = {
            .expr = dy_core_expr_new(e),
            .map = equality_map_expr.equality_map,
            .check_result = res,
        }
    };

    *did_transform_subtype_expr = true;

    return res;
}

dy_ternary_t positive_junction_is_subtype(struct dy_core_ctx *ctx, struct dy_core_junction junction, struct dy_core_expr expr, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    size_t constraint_start1 = ctx->constraints.num_elems;
    struct dy_core_expr e1;
    bool did_transform_e1 = false;
    dy_ternary_t first_res = dy_is_subtype(ctx, *junction.e1, expr, subtype_expr, &e1, &did_transform_e1);
    if (first_res == DY_YES) {
        *new_subtype_expr = e1;
        *did_transform_subtype_expr = did_transform_e1;
        return DY_YES;
    }

    size_t constraint_start2 = ctx->constraints.num_elems;
    struct dy_core_expr e2;
    bool did_transform_e2 = false;
    dy_ternary_t second_res = dy_is_subtype(ctx, *junction.e2, expr, subtype_expr, &e2, &did_transform_e2);
    if (second_res == DY_YES) {
        if (did_transform_e1) {
            dy_core_expr_release(ctx, e1);
        }
        dy_free_first_constraints(ctx, constraint_start1, constraint_start2);
        *new_subtype_expr = e2;
        *did_transform_subtype_expr = did_transform_e2;
        return DY_YES;
    }

    if (first_res == DY_NO && second_res == DY_NO) {
        return DY_NO;
    }

    if (second_res == DY_NO) {
        *new_subtype_expr = e1;
        *did_transform_subtype_expr = did_transform_e1;
        return first_res;
    }

    if (first_res == DY_NO) {
        *new_subtype_expr = e2;
        *did_transform_subtype_expr = did_transform_e2;
        return second_res;
    }

    if (!did_transform_e1) {
        e1 = dy_core_expr_retain(ctx, subtype_expr);
    }

    if (!did_transform_e2) {
        e2 = dy_core_expr_retain(ctx, subtype_expr);
    }

    dy_join_constraints(ctx, constraint_start1, constraint_start2, DY_CORE_POLARITY_NEGATIVE);

    if (did_transform_e1 || did_transform_e2) {
        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_JUNCTION,
            .junction = {
                .e1 = dy_core_expr_new(e1),
                .e2 = dy_core_expr_new(e2),
                .polarity = DY_CORE_POLARITY_POSITIVE,
            }
        };

        *did_transform_subtype_expr = true;
    } else {
        dy_core_expr_release(ctx, e1);
        dy_core_expr_release(ctx, e2);
    }

    return DY_MAYBE;
}

dy_ternary_t negative_junction_is_subtype(struct dy_core_ctx *ctx, struct dy_core_junction junction, struct dy_core_expr expr, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    size_t constraint_start1 = ctx->constraints.num_elems;
    struct dy_core_expr e1;
    bool did_transform_e1 = false;
    dy_ternary_t first_result = dy_is_subtype(ctx, *junction.e1, expr, subtype_expr, &e1, &did_transform_e1);

    size_t constraint_start2 = ctx->constraints.num_elems;
    struct dy_core_expr e2;
    bool did_transform_e2 = false;
    dy_ternary_t second_result = dy_is_subtype(ctx, *junction.e2, expr, subtype_expr, &e2, &did_transform_e2);

    if (first_result == DY_NO && second_result == DY_NO) {
        return DY_NO;
    }

    if (second_result == DY_NO) {
        *new_subtype_expr = e1;
        *did_transform_subtype_expr = did_transform_e1;
        return DY_MAYBE;
    }

    if (first_result == DY_NO) {
        *new_subtype_expr = e2;
        *did_transform_subtype_expr = did_transform_e2;
        return DY_MAYBE;
    }

    if (!did_transform_e1) {
        e1 = dy_core_expr_retain(ctx, subtype_expr);
    }

    if (!did_transform_e2) {
        e2 = dy_core_expr_retain(ctx, subtype_expr);
    }

    dy_join_constraints(ctx, constraint_start1, constraint_start2, DY_CORE_POLARITY_POSITIVE);

    if (did_transform_e1 || did_transform_e2) {
        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_ALTERNATIVE,
            .alternative = {
                .first = dy_core_expr_new(e1),
                .second = dy_core_expr_new(e2),
            }
        };

        *did_transform_subtype_expr = true;
    } else {
        dy_core_expr_release(ctx, e1);
        dy_core_expr_release(ctx, e2);
    }

    if (first_result == DY_YES && second_result == DY_YES) {
        return DY_YES;
    }

    return DY_MAYBE;
}

dy_ternary_t is_subtype_of_positive_junction(struct dy_core_ctx *ctx, struct dy_core_expr expr, struct dy_core_junction junction, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    size_t constraint_start1 = ctx->constraints.num_elems;
    struct dy_core_expr e1;
    bool did_transform_e1 = false;
    dy_ternary_t first_result = dy_is_subtype(ctx, expr, *junction.e1, subtype_expr, &e1, &did_transform_e1);
    if (first_result == DY_NO) {
        return DY_NO;
    }

    size_t constraint_start2 = ctx->constraints.num_elems;
    struct dy_core_expr e2;
    bool did_transform_e2 = false;
    dy_ternary_t second_result = dy_is_subtype(ctx, expr, *junction.e2, subtype_expr, &e2, &did_transform_e2);
    if (second_result == DY_NO) {
        dy_free_first_constraints(ctx, constraint_start1, ctx->constraints.num_elems);

        if (did_transform_e1) {
            dy_core_expr_release(ctx, e1);
        }

        return DY_NO;
    }

    if (!did_transform_e1) {
        e1 = dy_core_expr_retain(ctx, subtype_expr);
    }

    if (!did_transform_e2) {
        e2 = dy_core_expr_retain(ctx, subtype_expr);
    }

    dy_join_constraints(ctx, constraint_start1, constraint_start2, DY_CORE_POLARITY_POSITIVE);

    if (did_transform_e1 || did_transform_e2) {
        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_JUNCTION,
            .junction = {
                .e1 = dy_core_expr_new(e1),
                .e2 = dy_core_expr_new(e2),
                .polarity = DY_CORE_POLARITY_POSITIVE,
            }
        };

        *did_transform_subtype_expr = true;
    } else {
        dy_core_expr_release(ctx, e1);
        dy_core_expr_release(ctx, e2);
    }

    if (first_result == DY_MAYBE || second_result == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

dy_ternary_t is_subtype_of_negative_junction(struct dy_core_ctx *ctx, struct dy_core_expr expr, struct dy_core_junction junction, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    size_t constraint_start1 = ctx->constraints.num_elems;
    struct dy_core_expr e1;
    bool did_transform_e1 = false;
    dy_ternary_t first_res = dy_is_subtype(ctx, expr, *junction.e1, subtype_expr, &e1, &did_transform_e1);
    if (first_res == DY_YES) {
        *new_subtype_expr = e1;
        *did_transform_subtype_expr = did_transform_e1;
        return DY_YES;
    }

    size_t constraint_start2 = ctx->constraints.num_elems;
    struct dy_core_expr e2;
    bool did_transform_e2 = false;
    dy_ternary_t second_res = dy_is_subtype(ctx, expr, *junction.e2, subtype_expr, &e2, &did_transform_e2);
    if (second_res == DY_YES) {
        if (did_transform_e1) {
            dy_core_expr_release(ctx, e1);
        }
        dy_free_first_constraints(ctx, constraint_start1, constraint_start2);
        *new_subtype_expr = e2;
        *did_transform_subtype_expr = did_transform_e2;
        return DY_YES;
    }

    if (first_res == DY_NO && second_res == DY_NO) {
        return DY_NO;
    }

    if (second_res == DY_NO) {
        *new_subtype_expr = e1;
        *did_transform_subtype_expr = did_transform_e1;
        return first_res;
    }

    if (first_res == DY_NO) {
        *new_subtype_expr = e2;
        *did_transform_subtype_expr = did_transform_e2;
        return second_res;
    }

    if (!did_transform_e1) {
        e1 = dy_core_expr_retain(ctx, subtype_expr);
    }

    if (!did_transform_e2) {
        e2 = dy_core_expr_retain(ctx, subtype_expr);
    }

    dy_join_constraints(ctx, constraint_start1, constraint_start2, DY_CORE_POLARITY_NEGATIVE);

    if (did_transform_e1 || did_transform_e2) {
        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_JUNCTION,
            .junction = {
                .e1 = dy_core_expr_new(e1),
                .e2 = dy_core_expr_new(e2),
                .polarity = DY_CORE_POLARITY_POSITIVE,
            }
        };

        *did_transform_subtype_expr = true;
    } else {
        dy_core_expr_release(ctx, e1);
        dy_core_expr_release(ctx, e2);
    }

    return DY_MAYBE;
}

dy_ternary_t positive_recursion_is_subtype(struct dy_core_ctx *ctx, struct dy_core_recursion rec, struct dy_core_expr supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    for (size_t i = 0; i < ctx->supertype_assumption_cache.num_elems; ++i) {
        struct dy_subtype_assumption assumption;
        dy_array_get(ctx->supertype_assumption_cache, i, &assumption);

        if (rec.id == assumption.rec_binding_id && dy_are_equal(ctx, supertype, assumption.type) == DY_YES) {
            // Treat as All <: X
            struct dy_core_expr any = {
                .tag = DY_CORE_EXPR_END,
                .end_polarity = DY_CORE_POLARITY_POSITIVE
            };

            return dy_is_subtype(ctx, any, supertype, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
        }
    }

    struct dy_subtype_assumption assumption = {
        .type = supertype,
        .rec_binding_id = rec.id
    };

    size_t old_size = dy_array_add(&ctx->supertype_assumption_cache, &assumption);

    struct dy_core_expr rec_expr = {
        .tag = DY_CORE_EXPR_RECURSION,
        .recursion = rec
    };

    struct dy_core_expr new_subtype;
    if (!substitute(ctx, *rec.expr, rec.id, rec_expr, &new_subtype)) {
        new_subtype = dy_core_expr_retain(ctx, *rec.expr);
    }

    dy_ternary_t result = dy_is_subtype(ctx, new_subtype, supertype, subtype_expr, new_subtype_expr, did_transform_subtype_expr);

    dy_core_expr_release(ctx, new_subtype);

    ctx->supertype_assumption_cache.num_elems = old_size;

    return result;
}

dy_ternary_t negative_recursion_is_subtype(struct dy_core_ctx *ctx, struct dy_core_recursion rec, struct dy_core_expr supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    for (size_t i = 0; i < ctx->supertype_assumption_cache.num_elems; ++i) {
        struct dy_subtype_assumption assumption;
        dy_array_get(ctx->supertype_assumption_cache, i, &assumption);

        if (rec.id == assumption.rec_binding_id && dy_are_equal(ctx, supertype, assumption.type) == DY_YES) {
            // Treat as Any <: X
            struct dy_core_expr all = {
                .tag = DY_CORE_EXPR_END,
                .end_polarity = DY_CORE_POLARITY_NEGATIVE
            };

            return dy_is_subtype(ctx, all, supertype, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
        }
    }

    struct dy_subtype_assumption assumption = {
        .type = supertype,
        .rec_binding_id = rec.id
    };

    size_t old_size = dy_array_add(&ctx->supertype_assumption_cache, &assumption);

    struct dy_core_expr rec_expr = {
        .tag = DY_CORE_EXPR_RECURSION,
        .recursion = rec
    };

    struct dy_core_expr new_expr;
    if (!substitute(ctx, *rec.expr, rec.id, rec_expr, &new_expr)) {
        new_expr = dy_core_expr_retain(ctx, *rec.expr);
    }

    dy_ternary_t result = dy_is_subtype(ctx, new_expr, supertype, subtype_expr, new_subtype_expr, did_transform_subtype_expr);

    dy_core_expr_release(ctx, new_expr);

    ctx->supertype_assumption_cache.num_elems = old_size;

    return result;
}

dy_ternary_t is_subtype_of_positive_recursion(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_recursion rec, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    for (size_t i = 0; i < ctx->subtype_assumption_cache.num_elems; ++i) {
        struct dy_subtype_assumption assumption;
        dy_array_get(ctx->subtype_assumption_cache, i, &assumption);

        if (rec.id == assumption.rec_binding_id && dy_are_equal(ctx, subtype, assumption.type) == DY_YES) {
            // Treat as X <: All
            struct dy_core_expr any = {
                .tag = DY_CORE_EXPR_END,
                .end_polarity = DY_CORE_POLARITY_POSITIVE
            };

            return dy_is_subtype(ctx, subtype, any, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
        }
    }

    struct dy_subtype_assumption assumption = {
        .type = subtype,
        .rec_binding_id = rec.id
    };

    size_t old_size = dy_array_add(&ctx->subtype_assumption_cache, &assumption);

    struct dy_core_expr rec_expr = {
        .tag = DY_CORE_EXPR_RECURSION,
        .recursion = rec
    };

    struct dy_core_expr new_expr;
    if (!substitute(ctx, *rec.expr, rec.id, rec_expr, &new_expr)) {
        new_expr = dy_core_expr_retain(ctx, *rec.expr);
    }

    dy_ternary_t result = dy_is_subtype(ctx, subtype, new_expr, subtype_expr, new_subtype_expr, did_transform_subtype_expr);

    dy_core_expr_release(ctx, new_expr);

    ctx->subtype_assumption_cache.num_elems = old_size;

    return result;
}

dy_ternary_t is_subtype_of_negative_recursion(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_recursion rec, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    for (size_t i = 0; i < ctx->subtype_assumption_cache.num_elems; ++i) {
        struct dy_subtype_assumption assumption;
        dy_array_get(ctx->subtype_assumption_cache, i, &assumption);

        if (rec.id == assumption.rec_binding_id && dy_are_equal(ctx, subtype, assumption.type) == DY_YES) {
            // Treat as X <: Any
            struct dy_core_expr all = {
                .tag = DY_CORE_EXPR_END,
                .end_polarity = DY_CORE_POLARITY_NEGATIVE
            };

            return dy_is_subtype(ctx, subtype, all, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
        }
    }

    struct dy_subtype_assumption assumption = {
        .type = subtype,
        .rec_binding_id = rec.id
    };

    size_t old_size = dy_array_add(&ctx->subtype_assumption_cache, &assumption);

    struct dy_core_expr rec_expr = {
        .tag = DY_CORE_EXPR_RECURSION,
        .recursion = rec
    };

    struct dy_core_expr new_expr;
    if (!substitute(ctx, *rec.expr, rec.id, rec_expr, &new_expr)) {
        new_expr = dy_core_expr_retain(ctx, *rec.expr);
    }

    dy_ternary_t result = dy_is_subtype(ctx, subtype, new_expr, subtype_expr, new_subtype_expr, did_transform_subtype_expr);

    dy_core_expr_release(ctx, new_expr);

    ctx->subtype_assumption_cache.num_elems = old_size;

    return result;
}

dy_ternary_t dy_is_subtype_no_transformation(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_expr supertype)
{
    struct dy_core_expr e = {
        .tag = DY_CORE_EXPR_SYMBOL // arbitrary
    };

    size_t size = ctx->subtype_implicits.num_elems;

    bool did_transform_e = false;
    dy_ternary_t result = dy_is_subtype(ctx, subtype, supertype, e, &e, &did_transform_e);
    if (did_transform_e || ctx->subtype_implicits.num_elems != size) {
        if (did_transform_e) {
            dy_core_expr_release(ctx, e);
        }
        return DY_NO;
    }

    return result;
}

bool dy_is_inference_var(struct dy_core_ctx *ctx, size_t id, enum dy_core_polarity *polarity)
{
    for (size_t i = 0, size = ctx->bindings.num_elems; i < size; ++i) {
        const struct dy_core_binding *b = dy_array_pos(ctx->bindings, i);
        if (b->id == id) {
            if (b->is_inference_var) {
                *polarity = b->polarity;
                return true;
            } else {
                return false;
            }
        }
    }
    
    for (size_t i = 0, size = ctx->subtype_implicits.num_elems; i < size; ++i) {
        const struct dy_core_binding *b = dy_array_pos(ctx->subtype_implicits, i);
        if (b->id == id) {
            *polarity = DY_CORE_POLARITY_NEGATIVE;
            return true;
        }
    }
    
    for (size_t i = 0, size = ctx->bound_inference_vars.num_elems; i < size; ++i) {
        const struct dy_bound_inference_var *bc = dy_array_pos(ctx->bound_inference_vars, i);
        if (bc->id == id) {
            *polarity = bc->polarity;
            return true;
        }
    }
    
    dy_bail("Unbound variable!");
}
