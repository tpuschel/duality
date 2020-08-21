/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core.h"
#include "constraint.h"
#include "type_of.h"

/**
 * Implementation of the subtype check.
 *
 * A subtype check does three things:
 *   (1) Check whether the supposed subtype actually is a subtype of the supposed supertype.
 *       This results in a ternary result.
 *   (2) Collect constraints generated while doing (1).
 *   (3) Transform the expression that is of type 'subtype' according to the information recovered during (2).
 */

static inline dy_ternary_t dy_is_subtype(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, dy_array_t *subtype_implicits);

static inline dy_ternary_t dy_is_subtype_no_transformation(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint);

static inline dy_ternary_t dy_is_subtype_sub(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr, dy_array_t *subtype_implicits);

static inline dy_ternary_t equality_map_is_subtype(struct dy_core_ctx *ctx, struct dy_core_equality_map equality_map, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_equality_map, dy_array_t *subtype_implicits);

static inline dy_ternary_t positive_equality_map_is_subtype(struct dy_core_ctx *ctx, struct dy_core_equality_map equality_map, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr, dy_array_t *subtype_implicits);
static inline dy_ternary_t negative_equality_map_is_subtype(struct dy_core_ctx *ctx, struct dy_core_equality_map equality_map, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr, dy_array_t *subtype_implicits);

static inline dy_ternary_t type_map_is_subtype(struct dy_core_ctx *ctx, struct dy_core_type_map type_map, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr, dy_array_t *subtype_implicits);
static inline dy_ternary_t positive_type_map_is_subtype(struct dy_core_ctx *ctx, struct dy_core_type_map type_map, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr, dy_array_t *subtype_implicits);
static inline dy_ternary_t negative_type_map_is_subtype(struct dy_core_ctx *ctx, struct dy_core_type_map type_map, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr, dy_array_t *subtype_implicits);

static inline dy_ternary_t positive_junction_is_subtype(struct dy_core_ctx *ctx, struct dy_core_junction junction, struct dy_core_expr expr, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr, dy_array_t *subtype_implicits);
static inline dy_ternary_t negative_junction_is_subtype(struct dy_core_ctx *ctx, struct dy_core_junction junction, struct dy_core_expr expr, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr, dy_array_t *subtype_implicits);

static inline dy_ternary_t is_subtype_of_positive_junction(struct dy_core_ctx *ctx, struct dy_core_expr expr, struct dy_core_junction junction, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr, dy_array_t *subtype_implicits);
static inline dy_ternary_t is_subtype_of_negative_junction(struct dy_core_ctx *ctx, struct dy_core_expr expr, struct dy_core_junction junction, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr, dy_array_t *subtype_implicits);

static inline dy_ternary_t junction_is_subtype_of_junction(struct dy_core_ctx *ctx, struct dy_core_junction p1, struct dy_core_junction p2, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr, dy_array_t *subtype_implicits);

static inline dy_ternary_t both_are_subtype_respectively(struct dy_core_ctx *ctx, struct dy_core_expr sub1, struct dy_core_expr sup1, struct dy_core_expr sub2, struct dy_core_expr sup2, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr, dy_array_t *subtype_implicits);

static inline dy_ternary_t positive_recursion_is_subtype(struct dy_core_ctx *ctx, struct dy_core_recursion rec, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr, dy_array_t *subtype_implicits);

static inline dy_ternary_t negative_recursion_is_subtype(struct dy_core_ctx *ctx, struct dy_core_recursion rec, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr, dy_array_t *subtype_implicits);

static inline dy_ternary_t is_subtype_of_positive_recursion(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_recursion rec, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr, dy_array_t *subtype_implicits);

static inline dy_ternary_t is_subtype_of_negative_recursion(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_recursion rec, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr, dy_array_t *subtype_implicits);

static inline const struct dy_constraint *alloc_constraint(struct dy_constraint constraint);

static inline bool dy_is_vacuous(size_t id, struct dy_core_expr expr);

dy_ternary_t dy_is_subtype(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, dy_array_t *subtype_implicits)
{
    bool did_transform_expr = false;
    dy_ternary_t result = dy_is_subtype_sub(ctx, subtype, supertype, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, &did_transform_expr, subtype_implicits);

    if (!did_transform_expr) {
        *new_subtype_expr = dy_core_expr_retain(subtype_expr);
    }

    return result;
}

dy_ternary_t dy_is_subtype_no_transformation(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint)
{
    struct dy_core_expr e = {
        .tag = DY_CORE_EXPR_SYMBOL // arbitrary
    };

    dy_array_t subtype_implicits = dy_array_create(sizeof(struct dy_core_variable), DY_ALIGNOF(struct dy_core_variable), 8);

    bool did_transform_e = false;
    dy_ternary_t result = dy_is_subtype_sub(ctx, subtype, supertype, constraint, did_generate_constraint, e, &e, &did_transform_e, &subtype_implicits);
    if (did_transform_e || subtype_implicits.num_elems != 0) {
        dy_core_expr_release(e);
        return DY_NO;
    }

    return result;
}

dy_ternary_t dy_is_subtype_sub(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr, dy_array_t *subtype_implicits)
{
    if (subtype.tag == DY_CORE_EXPR_INFERENCE_VARIABLE && subtype.inference_variable.polarity == DY_CORE_POLARITY_NEGATIVE) {
        if (dy_is_vacuous(subtype.inference_variable.id, supertype)) {
            return DY_YES;
        }

        *constraint = (struct dy_constraint){
            .tag = DY_CONSTRAINT_SINGLE,
            .single = {
                .id = subtype.inference_variable.id,
                .expr = dy_core_expr_retain(supertype),
                .polarity = DY_CORE_POLARITY_NEGATIVE,
            }
        };

        *did_generate_constraint = true;

        return DY_MAYBE;
    }

    if (supertype.tag == DY_CORE_EXPR_INFERENCE_VARIABLE && supertype.inference_variable.polarity == DY_CORE_POLARITY_POSITIVE) {
        if (dy_is_vacuous(supertype.inference_variable.id, subtype)) {
            return DY_YES;
        }

        *constraint = (struct dy_constraint){
            .tag = DY_CONSTRAINT_SINGLE,
            .single = {
                .id = supertype.inference_variable.id,
                .expr = dy_core_expr_retain(subtype),
                .polarity = DY_CORE_POLARITY_POSITIVE,
            }
        };

        *did_generate_constraint = true;

        return DY_MAYBE;
    }

    if (subtype.tag == DY_CORE_EXPR_INFERENCE_VARIABLE || supertype.tag == DY_CORE_EXPR_INFERENCE_VARIABLE) {
        return DY_MAYBE;
    }

    if (supertype.tag == DY_CORE_EXPR_END && supertype.end_polarity == DY_CORE_POLARITY_NEGATIVE) {
        return DY_YES;
    }

    if (subtype.tag == DY_CORE_EXPR_END && subtype.end_polarity == DY_CORE_POLARITY_POSITIVE) {
        return DY_YES;
    }

    if (subtype.tag == DY_CORE_EXPR_RECURSION && supertype.tag == DY_CORE_EXPR_RECURSION && subtype.recursion.polarity == supertype.recursion.polarity && subtype.recursion.id == supertype.recursion.id) {
        return dy_is_subtype_sub(ctx, *subtype.recursion.expr, *supertype.recursion.expr, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, did_transform_subtype_expr, subtype_implicits);
    }

    if (subtype.tag == DY_CORE_EXPR_RECURSION && subtype.recursion.polarity == DY_CORE_POLARITY_POSITIVE) {
        return positive_recursion_is_subtype(ctx, subtype.recursion, supertype, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, did_transform_subtype_expr, subtype_implicits);
    }

    if (subtype.tag == DY_CORE_EXPR_RECURSION && subtype.recursion.polarity == DY_CORE_POLARITY_NEGATIVE) {
        return negative_recursion_is_subtype(ctx, subtype.recursion, supertype, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, did_transform_subtype_expr, subtype_implicits);
    }

    if (supertype.tag == DY_CORE_EXPR_RECURSION && supertype.recursion.polarity == DY_CORE_POLARITY_POSITIVE) {
        return is_subtype_of_positive_recursion(ctx, subtype, supertype.recursion, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, did_transform_subtype_expr, subtype_implicits);
    }

    if (supertype.tag == DY_CORE_EXPR_RECURSION && supertype.recursion.polarity == DY_CORE_POLARITY_NEGATIVE) {
        return is_subtype_of_negative_recursion(ctx, subtype, supertype.recursion, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, did_transform_subtype_expr, subtype_implicits);
    }

    if (supertype.tag == DY_CORE_EXPR_JUNCTION && supertype.junction.polarity == DY_CORE_POLARITY_POSITIVE) {
        return is_subtype_of_positive_junction(ctx, subtype, supertype.junction, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, did_transform_subtype_expr, subtype_implicits);
    }

    if (supertype.tag == DY_CORE_EXPR_JUNCTION && supertype.junction.polarity == DY_CORE_POLARITY_NEGATIVE) {
        return is_subtype_of_negative_junction(ctx, subtype, supertype.junction, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, did_transform_subtype_expr, subtype_implicits);
    }

    if (subtype.tag == DY_CORE_EXPR_JUNCTION && subtype.junction.polarity == DY_CORE_POLARITY_POSITIVE) {
        return positive_junction_is_subtype(ctx, subtype.junction, supertype, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, did_transform_subtype_expr, subtype_implicits);
    }

    if (subtype.tag == DY_CORE_EXPR_JUNCTION && subtype.junction.polarity == DY_CORE_POLARITY_NEGATIVE) {
        return negative_junction_is_subtype(ctx, subtype.junction, supertype, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, did_transform_subtype_expr, subtype_implicits);
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
        return equality_map_is_subtype(ctx, subtype.equality_map, supertype, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, did_transform_subtype_expr, subtype_implicits);
    }

    if (subtype.tag == DY_CORE_EXPR_TYPE_MAP) {
        return type_map_is_subtype(ctx, subtype.type_map, supertype, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, did_transform_subtype_expr, subtype_implicits);
    }

    if (subtype.tag == DY_CORE_EXPR_CUSTOM) {
        return subtype.custom.is_subtype(subtype.custom.data, ctx, supertype, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, subtype_implicits);
    }

    if (supertype.tag == DY_CORE_EXPR_CUSTOM) {
        return supertype.custom.is_supertype(supertype.custom.data, ctx, subtype, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, subtype_implicits);
    }

    dy_bail("Should be unreachable!");
}

dy_ternary_t equality_map_is_subtype(struct dy_core_ctx *ctx, struct dy_core_equality_map equality_map, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr, dy_array_t *subtype_implicits)
{
    switch (equality_map.polarity) {
    case DY_CORE_POLARITY_POSITIVE:
        return positive_equality_map_is_subtype(ctx, equality_map, supertype, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, did_transform_subtype_expr, subtype_implicits);
    case DY_CORE_POLARITY_NEGATIVE:
        return negative_equality_map_is_subtype(ctx, equality_map, supertype, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, did_transform_subtype_expr, subtype_implicits);
    }

    dy_bail("Impossible polarity.");
}

dy_ternary_t positive_equality_map_is_subtype(struct dy_core_ctx *ctx, struct dy_core_equality_map equality_map, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr, dy_array_t *subtype_implicits)
{
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
    if (supertype.tag == DY_CORE_EXPR_EQUALITY_MAP && equality_map.is_implicit == supertype.equality_map.is_implicit) {
        dy_ternary_t are_equal = dy_are_equal(ctx, *equality_map.e1, *supertype.equality_map.e1);
        if (are_equal == DY_NO) {
            return DY_NO;
        }

        struct dy_core_expr subtype_expr_emap_e2 = {
            .tag = DY_CORE_EXPR_EQUALITY_MAP_ELIM,
            .equality_map_elim = {
                .expr = dy_core_expr_new(dy_core_expr_retain(subtype_expr)),
                .map = {
                    .e1 = dy_core_expr_retain_ptr(equality_map.e1),
                    .e2 = dy_core_expr_retain_ptr(equality_map.e2),
                    .polarity = DY_CORE_POLARITY_NEGATIVE,
                    .is_implicit = equality_map.is_implicit,
                },
                .check_result = DY_MAYBE,
            }
        };

        struct dy_core_expr new_subtype_expr_emap_e2;
        bool did_transform_subtype_expr_emap_e2 = false;
        dy_ternary_t is_subtype = dy_is_subtype_sub(ctx, *equality_map.e2, *supertype.equality_map.e2, constraint, did_generate_constraint, subtype_expr_emap_e2, &new_subtype_expr_emap_e2, &did_transform_subtype_expr_emap_e2, subtype_implicits);

        if (did_transform_subtype_expr_emap_e2) {
            dy_core_expr_release(subtype_expr_emap_e2);
        } else {
            new_subtype_expr_emap_e2 = subtype_expr_emap_e2;
        }

        if (is_subtype == DY_NO) {
            dy_core_expr_release(new_subtype_expr_emap_e2);
            return DY_NO;
        }

        if (did_transform_subtype_expr_emap_e2) {
            *new_subtype_expr = (struct dy_core_expr){
                .tag = DY_CORE_EXPR_EQUALITY_MAP,
                .equality_map = {
                    .e1 = dy_core_expr_retain_ptr(equality_map.e1),
                    .e2 = dy_core_expr_new(new_subtype_expr_emap_e2),
                    .polarity = DY_CORE_POLARITY_POSITIVE,
                    .is_implicit = equality_map.is_implicit,
                }
            };

            *did_transform_subtype_expr = true;
        } else {
            dy_core_expr_release(new_subtype_expr_emap_e2);
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
    if (supertype.tag == DY_CORE_EXPR_TYPE_MAP && supertype.type_map.polarity == DY_CORE_POLARITY_NEGATIVE && equality_map.is_implicit == supertype.type_map.is_implicit) {
        struct dy_core_expr type_of_equality_map_e1 = dy_type_of(ctx, *equality_map.e1);

        struct dy_constraint c1;
        bool have_c1 = false;
        struct dy_core_expr emap_e1 = *equality_map.e1;
        bool did_transform_emap_e1 = false;
        struct dy_core_expr new_emap_e1;
        dy_ternary_t is_subtype_in = dy_is_subtype_sub(ctx, type_of_equality_map_e1, *supertype.type_map.binding.type, &c1, &have_c1, emap_e1, &new_emap_e1, &did_transform_emap_e1, subtype_implicits);

        dy_core_expr_release(type_of_equality_map_e1);

        if (is_subtype_in == DY_NO) {
            return DY_NO;
        }

        if (!did_transform_emap_e1) {
            new_emap_e1 = dy_core_expr_retain(emap_e1);
        }

        struct dy_core_expr emap_e2 = {
            .tag = DY_CORE_EXPR_EQUALITY_MAP_ELIM,
            .equality_map_elim = {
                .expr = dy_core_expr_new(dy_core_expr_retain(subtype_expr)),
                .map = {
                    .e1 = dy_core_expr_retain_ptr(equality_map.e1),
                    .e2 = dy_core_expr_retain_ptr(equality_map.e2),
                    .polarity = DY_CORE_POLARITY_NEGATIVE,
                    .is_implicit = equality_map.is_implicit,
                },
                .check_result = DY_MAYBE,
            }
        };

        struct dy_constraint c2;
        bool have_c2 = false;
        struct dy_core_expr new_emap_e2;
        bool did_transform_emap_e2 = false;
        dy_ternary_t is_subtype_out = dy_is_subtype_sub(ctx, *equality_map.e2, *supertype.type_map.expr, &c2, &have_c2, emap_e2, &new_emap_e2, &did_transform_emap_e2, subtype_implicits);
        if (is_subtype_out == DY_NO) {
            dy_core_expr_release(new_emap_e1);
            dy_core_expr_release(emap_e2);
            return DY_NO;
        }

        if (did_transform_emap_e2) {
            dy_core_expr_release(emap_e2);
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
            dy_core_expr_release(new_emap_e1);
            dy_core_expr_release(new_emap_e2);
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

        if (is_subtype_in == DY_MAYBE || is_subtype_out == DY_MAYBE) {
            return DY_MAYBE;
        }

        return DY_YES;
    }

    return DY_NO;
}

dy_ternary_t negative_equality_map_is_subtype(struct dy_core_ctx *ctx, struct dy_core_equality_map equality_map, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr, dy_array_t *subtype_implicits)
{
    /*
     * Implements the following:
     *
     * (x : e1 ~> e2) <: e3 ~> e4
     *
     * e1 = e3
     *
     * e2 <: e4, f
     *
     * x => e1 -> f (x e1)
     */
    if (supertype.tag == DY_CORE_EXPR_EQUALITY_MAP && supertype.equality_map.polarity == DY_CORE_POLARITY_NEGATIVE && equality_map.is_implicit == supertype.equality_map.is_implicit) {
        dy_ternary_t are_equal = dy_are_equal(ctx, *equality_map.e1, *supertype.equality_map.e1);
        if (are_equal == DY_NO) {
            return DY_NO;
        }

        struct dy_core_expr emap_e2 = {
            .tag = DY_CORE_EXPR_EQUALITY_MAP_ELIM,
            .equality_map_elim = {
                .expr = dy_core_expr_new(dy_core_expr_retain(subtype_expr)),
                .map = {
                    .e1 = dy_core_expr_retain_ptr(equality_map.e1),
                    .e2 = dy_core_expr_retain_ptr(equality_map.e2),
                    .polarity = DY_CORE_POLARITY_NEGATIVE,
                    .is_implicit = equality_map.is_implicit,
                },
                .check_result = DY_MAYBE,
            }
        };

        struct dy_core_expr new_emap_e2;
        bool did_transform_emap_e2 = false;
        dy_ternary_t is_subtype = dy_is_subtype_sub(ctx, *equality_map.e2, *supertype.equality_map.e2, constraint, did_generate_constraint, emap_e2, &new_emap_e2, &did_transform_emap_e2, subtype_implicits);

        if (is_subtype == DY_NO) {
            dy_core_expr_release(emap_e2);
            return DY_NO;
        }

        if (did_transform_emap_e2) {
            dy_core_expr_release(emap_e2);
        } else {
            new_emap_e2 = emap_e2;
        }

        if (did_transform_emap_e2) {
            *new_subtype_expr = (struct dy_core_expr){
                .tag = DY_CORE_EXPR_EQUALITY_MAP,
                .equality_map = {
                    .e1 = dy_core_expr_retain_ptr(equality_map.e1),
                    .e2 = dy_core_expr_new(new_emap_e2),
                    .polarity = DY_CORE_POLARITY_POSITIVE,
                    .is_implicit = equality_map.is_implicit,
                }
            };

            *did_transform_subtype_expr = true;
        } else {
            dy_core_expr_release(new_emap_e2);
        }

        if (are_equal == DY_MAYBE || is_subtype == DY_MAYBE) {
            return DY_MAYBE;
        }

        return DY_YES;
    }

    return DY_NO;
}

dy_ternary_t type_map_is_subtype(struct dy_core_ctx *ctx, struct dy_core_type_map type_map, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr, dy_array_t *subtype_implicits)
{
    switch (type_map.polarity) {
    case DY_CORE_POLARITY_POSITIVE:
        return positive_type_map_is_subtype(ctx, type_map, supertype, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, did_transform_subtype_expr, subtype_implicits);
    case DY_CORE_POLARITY_NEGATIVE:
        return negative_type_map_is_subtype(ctx, type_map, supertype, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, did_transform_subtype_expr, subtype_implicits);
    }

    dy_bail("Impossible polarity.");
}

dy_ternary_t positive_type_map_is_subtype(struct dy_core_ctx *ctx, struct dy_core_type_map type_map, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr, dy_array_t *subtype_implicits)
{
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
    if (supertype.tag == DY_CORE_EXPR_EQUALITY_MAP && supertype.equality_map.polarity == DY_CORE_POLARITY_NEGATIVE && type_map.is_implicit == supertype.equality_map.is_implicit) {
        struct dy_core_expr type_of_supertype_e1 = dy_type_of(ctx, *supertype.equality_map.e1);

        struct dy_constraint c1;
        bool have_c1 = false;
        struct dy_core_expr new_supertype_e1;
        bool did_transform_supertype_e1 = false;
        dy_ternary_t is_subtype_in = dy_is_subtype_sub(ctx, type_of_supertype_e1, *type_map.binding.type, &c1, &have_c1, *supertype.equality_map.e1, &new_supertype_e1, &did_transform_supertype_e1, subtype_implicits);

        dy_core_expr_release(type_of_supertype_e1);

        if (is_subtype_in == DY_NO) {
            return DY_NO;
        }

        if (!did_transform_supertype_e1) {
            new_supertype_e1 = dy_core_expr_retain(*supertype.equality_map.e1);
        }

        struct dy_core_expr new_type_map_expr = substitute(ctx, *type_map.expr, type_map.binding.id, new_supertype_e1);

        struct dy_core_expr emap_e2 = {
            .tag = DY_CORE_EXPR_EQUALITY_MAP_ELIM,
            .equality_map_elim = {
                .expr = dy_core_expr_new(dy_core_expr_retain(subtype_expr)),
                .map = {
                    .e1 = dy_core_expr_new(new_supertype_e1),
                    .e2 = dy_core_expr_new(new_type_map_expr),
                    .polarity = DY_CORE_POLARITY_NEGATIVE,
                    .is_implicit = type_map.is_implicit,
                },
                .check_result = DY_MAYBE,
            }
        };

        struct dy_constraint c2;
        bool have_c2 = false;
        struct dy_core_expr new_emap_e2;
        bool did_transform_emap_e2 = false;
        dy_ternary_t is_subtype_out = dy_is_subtype_sub(ctx, new_type_map_expr, *supertype.equality_map.e2, &c2, &have_c2, emap_e2, &new_emap_e2, &did_transform_emap_e2, subtype_implicits);
        if (is_subtype_out == DY_NO) {
            dy_core_expr_release(emap_e2);
            return DY_NO;
        }

        if (did_transform_emap_e2) {
            dy_core_expr_release(emap_e2);
        } else {
            new_emap_e2 = emap_e2;
        }

        if (did_transform_supertype_e1 || did_transform_emap_e2) {
            *new_subtype_expr = (struct dy_core_expr){
                .tag = DY_CORE_EXPR_EQUALITY_MAP,
                .equality_map = {
                    .e1 = dy_core_expr_retain_ptr(supertype.equality_map.e1),
                    .e2 = dy_core_expr_new(new_emap_e2),
                    .polarity = DY_CORE_POLARITY_POSITIVE,
                    .is_implicit = type_map.is_implicit,
                }
            };

            *did_transform_subtype_expr = true;
        } else {
            dy_core_expr_release(new_emap_e2);
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
    if (supertype.tag == DY_CORE_EXPR_TYPE_MAP && supertype.type_map.polarity == DY_CORE_POLARITY_POSITIVE && type_map.is_implicit == supertype.type_map.is_implicit) {
        struct dy_core_variable var = {
            .id = supertype.type_map.binding.id,
            .type = supertype.type_map.binding.type,
        };

        struct dy_core_expr var_expr = {
            .tag = DY_CORE_EXPR_VARIABLE,
            .variable = var
        };

        struct dy_constraint c1;
        bool have_c1 = false;
        struct dy_core_expr new_var_expr;
        bool did_transform_var_expr = false;
        dy_ternary_t is_subtype_in = dy_is_subtype_sub(ctx, *supertype.type_map.binding.type, *type_map.binding.type, &c1, &have_c1, var_expr, &new_var_expr, &did_transform_var_expr, subtype_implicits);
        if (is_subtype_in == DY_NO) {
            return DY_NO;
        }

        if (!did_transform_var_expr) {
            new_var_expr = dy_core_expr_retain(var_expr);
        }

        struct dy_core_expr new_type_map_expr = substitute(ctx, *type_map.expr, type_map.binding.id, new_var_expr);

        struct dy_core_expr tmap_e2 = {
            .tag = DY_CORE_EXPR_EQUALITY_MAP_ELIM,
            .equality_map_elim = {
                .expr = dy_core_expr_new(dy_core_expr_retain(subtype_expr)),
                .map = {
                    .e1 = dy_core_expr_new(new_var_expr),
                    .e2 = dy_core_expr_new(new_type_map_expr),
                    .polarity = DY_CORE_POLARITY_NEGATIVE,
                    .is_implicit = type_map.is_implicit,
                },
                .check_result = DY_MAYBE,
            }
        };

        struct dy_constraint c2;
        bool have_c2 = false;
        struct dy_core_expr new_tmap_e2;
        bool did_transform_tmap_e2 = false;
        dy_ternary_t is_subtype_out = dy_is_subtype_sub(ctx, new_type_map_expr, *supertype.type_map.expr, &c2, &have_c2, tmap_e2, &new_tmap_e2, &did_transform_tmap_e2, subtype_implicits);
        if (is_subtype_out == DY_NO) {
            dy_core_expr_release(tmap_e2);
            return DY_NO;
        }

        if (did_transform_tmap_e2) {
            dy_core_expr_release(tmap_e2);
        } else {
            new_tmap_e2 = tmap_e2;
        }

        if (did_transform_var_expr || did_transform_tmap_e2) {
            *new_subtype_expr = (struct dy_core_expr){
                .tag = DY_CORE_EXPR_TYPE_MAP,
                .type_map = {
                    .binding = {
                        .id = var.id,
                        .type = dy_core_expr_retain_ptr(var.type),
                    },
                    .expr = dy_core_expr_new(new_tmap_e2),
                    .polarity = DY_CORE_POLARITY_POSITIVE,
                    .is_implicit = type_map.is_implicit,
                }
            };

            *did_transform_subtype_expr = true;
        } else {
            dy_core_expr_release(new_tmap_e2);
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

        if (is_subtype_in == DY_MAYBE || is_subtype_out == DY_MAYBE) {
            return DY_MAYBE;
        }

        return DY_YES;
    }

    if (type_map.is_implicit) {
        size_t id = ctx->running_id++;

        struct dy_core_expr unknown = {
            .tag = DY_CORE_EXPR_INFERENCE_VARIABLE,
            .inference_variable = {
                .id = id,
                .type = type_map.binding.type,
                .polarity = DY_CORE_POLARITY_NEGATIVE,
            }
        };

        struct dy_core_expr e = {
            .tag = DY_CORE_EXPR_EQUALITY_MAP_ELIM,
            .equality_map_elim = {
                .expr = dy_core_expr_new(dy_core_expr_retain(subtype_expr)),
                .map = {
                    .e1 = dy_core_expr_new(dy_core_expr_retain(unknown)),
                    .e2 = dy_core_expr_new(dy_core_expr_retain(supertype)),
                    .polarity = DY_CORE_POLARITY_NEGATIVE,
                    .is_implicit = true,
                },
                .check_result = DY_MAYBE,
            }
        };

        struct dy_core_variable v = {
            .id = id,
            .type = type_map.binding.type
        };

        dy_array_add(subtype_implicits, &v);

        *new_subtype_expr = e;
        *did_transform_subtype_expr = true;

        return DY_MAYBE;
    }

    return DY_NO;
}

dy_ternary_t negative_type_map_is_subtype(struct dy_core_ctx *ctx, struct dy_core_type_map type_map, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr, dy_array_t *subtype_implicits)
{
    if (supertype.tag == DY_CORE_EXPR_TYPE_MAP && supertype.type_map.polarity == DY_CORE_POLARITY_NEGATIVE && type_map.is_implicit == supertype.type_map.is_implicit) {
        // Will revisit later.
        dy_bail("Not yet implemented.");
    }

    return DY_NO;
}

dy_ternary_t positive_junction_is_subtype(struct dy_core_ctx *ctx, struct dy_core_junction junction, struct dy_core_expr expr, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr, dy_array_t *subtype_implicits)
{
    struct dy_constraint c1;
    bool have_c1 = false;
    struct dy_core_expr e1;
    bool did_transform_e1 = false;
    dy_ternary_t first_res = dy_is_subtype_sub(ctx, *junction.e1, expr, &c1, &have_c1, subtype_expr, &e1, &did_transform_e1, subtype_implicits);
    if (first_res == DY_YES) {
        *constraint = c1;
        *did_generate_constraint = have_c1;
        *new_subtype_expr = e1;
        *did_transform_subtype_expr = did_transform_e1;
        return DY_YES;
    }

    struct dy_constraint c2;
    bool have_c2 = false;
    struct dy_core_expr e2;
    bool did_transform_e2 = false;
    dy_ternary_t second_res = dy_is_subtype_sub(ctx, *junction.e2, expr, &c2, &have_c2, subtype_expr, &e2, &did_transform_e2, subtype_implicits);
    if (second_res == DY_YES) {
        *constraint = c2;
        *did_generate_constraint = have_c2;
        *new_subtype_expr = e2;
        *did_transform_subtype_expr = did_transform_e2;
        return DY_YES;
    }

    if (first_res == DY_NO && second_res == DY_NO) {
        return DY_NO;
    }

    if (second_res == DY_NO) {
        *constraint = c1;
        *did_generate_constraint = have_c1;
        *new_subtype_expr = e1;
        *did_transform_subtype_expr = did_transform_e1;
        return first_res;
    }

    if (first_res == DY_NO) {
        *constraint = c2;
        *did_generate_constraint = have_c2;
        *new_subtype_expr = e2;
        *did_transform_subtype_expr = did_transform_e2;
        return second_res;
    }

    if (!did_transform_e1) {
        e1 = dy_core_expr_retain(subtype_expr);
    }

    if (!did_transform_e2) {
        e2 = dy_core_expr_retain(subtype_expr);
    }

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
    } else if (have_c1) {
        *constraint = c1;
        *did_generate_constraint = true;
    } else if (have_c2) {
        *constraint = c2;
        *did_generate_constraint = true;
    }

    if (did_transform_e1 && did_transform_e2) {
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
        dy_core_expr_release(e1);
        dy_core_expr_release(e2);
    }

    return DY_MAYBE;
}

dy_ternary_t negative_junction_is_subtype(struct dy_core_ctx *ctx, struct dy_core_junction junction, struct dy_core_expr expr, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr, dy_array_t *subtype_implicits)
{
    if (expr.tag == DY_CORE_EXPR_JUNCTION && expr.junction.polarity == DY_CORE_POLARITY_NEGATIVE) {
        return junction_is_subtype_of_junction(ctx, junction, expr.junction, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, did_transform_subtype_expr, subtype_implicits);
    }

    struct dy_constraint c1;
    bool have_c1 = false;
    struct dy_core_expr e1;
    bool did_transform_e1 = false;
    dy_ternary_t first_result = dy_is_subtype_sub(ctx, *junction.e1, expr, &c1, &have_c1, subtype_expr, &e1, &did_transform_e1, subtype_implicits);

    struct dy_constraint c2;
    bool have_c2 = false;
    struct dy_core_expr e2;
    bool did_transform_e2 = false;
    dy_ternary_t second_result = dy_is_subtype_sub(ctx, *junction.e2, expr, &c2, &have_c2, subtype_expr, &e2, &did_transform_e2, subtype_implicits);

    if (first_result == DY_NO && second_result == DY_NO) {
        return DY_NO;
    }

    if (second_result == DY_NO) {
        *constraint = c1;
        *did_generate_constraint = have_c1;
        *new_subtype_expr = e1;
        *did_transform_subtype_expr = did_transform_e1;
        return DY_MAYBE;
    }

    if (first_result == DY_NO) {
        *constraint = c2;
        *did_generate_constraint = have_c2;
        *new_subtype_expr = e2;
        *did_transform_subtype_expr = did_transform_e2;
        return DY_MAYBE;
    }

    if (!did_transform_e1) {
        e1 = dy_core_expr_retain(subtype_expr);
    }

    if (!did_transform_e2) {
        e2 = dy_core_expr_retain(subtype_expr);
    }

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
    } else if (have_c1) {
        *constraint = c1;
        *did_generate_constraint = true;
    } else if (have_c2) {
        *constraint = c2;
        *did_generate_constraint = true;
    }

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
        dy_core_expr_release(e1);
        dy_core_expr_release(e2);
    }

    if (first_result == DY_YES && second_result == DY_YES) {
        return DY_YES;
    }

    return DY_MAYBE;
}

dy_ternary_t is_subtype_of_positive_junction(struct dy_core_ctx *ctx, struct dy_core_expr expr, struct dy_core_junction junction, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr, dy_array_t *subtype_implicits)
{
    struct dy_constraint c1;
    bool have_c1 = false;
    struct dy_core_expr e1;
    bool did_transform_e1 = false;
    dy_ternary_t first_result = dy_is_subtype_sub(ctx, expr, *junction.e1, &c1, &have_c1, subtype_expr, &e1, &did_transform_e1, subtype_implicits);
    if (first_result == DY_NO) {
        return DY_NO;
    }

    struct dy_constraint c2;
    bool have_c2 = false;
    struct dy_core_expr e2;
    bool did_transform_e2 = false;
    dy_ternary_t second_result = dy_is_subtype_sub(ctx, expr, *junction.e2, &c2, &have_c2, subtype_expr, &e2, &did_transform_e2, subtype_implicits);
    if (second_result == DY_NO) {
        if (did_transform_e1) {
            dy_core_expr_release(e1);
        }

        return DY_NO;
    }

    if (!did_transform_e1) {
        e1 = dy_core_expr_retain(subtype_expr);
    }

    if (!did_transform_e2) {
        e2 = dy_core_expr_retain(subtype_expr);
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
        dy_core_expr_release(e1);
        dy_core_expr_release(e2);
    }

    if (first_result == DY_MAYBE || second_result == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

dy_ternary_t is_subtype_of_negative_junction(struct dy_core_ctx *ctx, struct dy_core_expr expr, struct dy_core_junction junction, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr, dy_array_t *subtype_implicits)
{
    if (expr.tag == DY_CORE_EXPR_JUNCTION && expr.junction.polarity == DY_CORE_POLARITY_NEGATIVE) {
        return junction_is_subtype_of_junction(ctx, expr.junction, junction, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, did_transform_subtype_expr, subtype_implicits);
    }

    struct dy_constraint c1;
    bool have_c1 = false;
    struct dy_core_expr e1;
    bool did_transform_e1 = false;
    dy_ternary_t first_res = dy_is_subtype_sub(ctx, expr, *junction.e1, &c1, &have_c1, subtype_expr, &e1, &did_transform_e1, subtype_implicits);
    if (first_res == DY_YES) {
        *constraint = c1;
        *did_generate_constraint = have_c1;
        *new_subtype_expr = e1;
        *did_transform_subtype_expr = did_transform_e1;
        return DY_YES;
    }

    struct dy_constraint c2;
    bool have_c2 = false;
    struct dy_core_expr e2;
    bool did_transform_e2 = false;
    dy_ternary_t second_res = dy_is_subtype_sub(ctx, expr, *junction.e2, &c2, &have_c2, subtype_expr, &e2, &did_transform_e2, subtype_implicits);
    if (second_res == DY_YES) {
        *constraint = c2;
        *did_generate_constraint = have_c2;
        *new_subtype_expr = e2;
        *did_transform_subtype_expr = did_transform_e2;
        return DY_YES;
    }

    if (first_res == DY_NO && second_res == DY_NO) {
        return DY_NO;
    }

    if (second_res == DY_NO) {
        *constraint = c1;
        *did_generate_constraint = have_c1;
        *new_subtype_expr = e1;
        *did_transform_subtype_expr = did_transform_e1;
        return first_res;
    }

    if (first_res == DY_NO) {
        *constraint = c2;
        *did_generate_constraint = have_c2;
        *new_subtype_expr = e2;
        *did_transform_subtype_expr = did_transform_e2;
        return second_res;
    }

    if (!did_transform_e1) {
        e1 = dy_core_expr_retain(subtype_expr);
    }

    if (!did_transform_e2) {
        e2 = dy_core_expr_retain(subtype_expr);
    }

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
    } else if (have_c1) {
        *constraint = c1;
        *did_generate_constraint = true;
    } else if (have_c2) {
        *constraint = c2;
        *did_generate_constraint = true;
    }

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
        dy_core_expr_release(e1);
        dy_core_expr_release(e2);
    }

    return DY_MAYBE;
}

dy_ternary_t both_are_subtype_respectively(struct dy_core_ctx *ctx, struct dy_core_expr sub1, struct dy_core_expr sup1, struct dy_core_expr sub2, struct dy_core_expr sup2, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr, dy_array_t *subtype_implicits)
{
    struct dy_constraint c1;
    bool have_c1 = false;
    struct dy_core_expr e1;
    bool did_transform_e1 = false;
    dy_ternary_t res1 = dy_is_subtype_sub(ctx, sub1, sup1, &c1, &have_c1, subtype_expr, &e1, &did_transform_e1, subtype_implicits);
    if (res1 == DY_NO) {
        return DY_NO;
    }

    if (!did_transform_e1) {
        e1 = dy_core_expr_retain(subtype_expr);
    }

    struct dy_constraint c2;
    bool have_c2 = false;
    struct dy_core_expr e2;
    bool did_transform_e2 = false;
    dy_ternary_t res2 = dy_is_subtype_sub(ctx, sub2, sup2, &c2, &have_c2, subtype_expr, &e2, &did_transform_e2, subtype_implicits);
    if (res2 == DY_NO) {
        dy_core_expr_release(e1);
        return DY_NO;
    }

    if (!did_transform_e2) {
        e2 = dy_core_expr_retain(subtype_expr);
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
        dy_core_expr_release(e1);
        dy_core_expr_release(e2);
    }

    if (res1 == DY_MAYBE || res2 == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

dy_ternary_t junction_is_subtype_of_junction(struct dy_core_ctx *ctx, struct dy_core_junction p1, struct dy_core_junction p2, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr, dy_array_t *subtype_implicits)
{
    struct dy_constraint c1;
    bool have_c1 = false;
    struct dy_core_expr e1;
    bool did_transform_e1 = false;
    dy_ternary_t res1 = both_are_subtype_respectively(ctx, *p1.e1, *p2.e1, *p1.e2, *p2.e2, &c1, &have_c1, subtype_expr, &e1, &did_transform_e1, subtype_implicits);
    if (res1 == DY_YES) {
        *constraint = c1;
        *did_generate_constraint = have_c1;
        *new_subtype_expr = e1;
        *did_transform_subtype_expr = did_transform_e1;
        return DY_YES;
    }

    struct dy_constraint c2;
    bool have_c2 = false;
    struct dy_core_expr e2;
    bool did_transform_e2 = false;
    dy_ternary_t res2 = both_are_subtype_respectively(ctx, *p1.e1, *p2.e2, *p1.e2, *p2.e1, &c2, &have_c2, subtype_expr, &e2, &did_transform_e2, subtype_implicits);
    if (res2 == DY_YES) {
        *constraint = c2;
        *did_generate_constraint = have_c2;
        *new_subtype_expr = e2;
        *did_transform_subtype_expr = did_transform_e2;
        return DY_YES;
    }

    if (res1 == DY_MAYBE) {
        *constraint = c1;
        *did_generate_constraint = have_c1;
        *new_subtype_expr = e1;
        *did_transform_subtype_expr = did_transform_e1;
        return DY_MAYBE;
    }

    if (res2 == DY_MAYBE) {
        *constraint = c2;
        *did_generate_constraint = have_c2;
        *new_subtype_expr = e2;
        *did_transform_subtype_expr = did_transform_e2;
        return DY_MAYBE;
    }

    return DY_NO;
}

dy_ternary_t positive_recursion_is_subtype(struct dy_core_ctx *ctx, struct dy_core_recursion rec, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr, dy_array_t *subtype_implicits)
{
    for (size_t i = 0; i < ctx->supertype_assumption_cache.num_elems; ++i) {
        struct dy_subtype_assumption assumption;
        dy_array_get(ctx->supertype_assumption_cache, i, &assumption);

        if (rec.id == assumption.rec_binding_id && dy_are_equal(ctx, supertype, assumption.type) == DY_YES) {
            // Treat as Any <: X
            struct dy_core_expr any = {
                .tag = DY_CORE_EXPR_END,
                .end_polarity = DY_CORE_POLARITY_NEGATIVE
            };

            return dy_is_subtype_sub(ctx, any, supertype, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, did_transform_subtype_expr, subtype_implicits);
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

    struct dy_core_expr new_subtype = substitute(ctx, *rec.expr, rec.id, rec_expr);

    dy_ternary_t result = dy_is_subtype_sub(ctx, new_subtype, supertype, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, did_transform_subtype_expr, subtype_implicits);

    dy_core_expr_release(new_subtype);

    ctx->supertype_assumption_cache.num_elems = old_size;

    return result;
}

dy_ternary_t negative_recursion_is_subtype(struct dy_core_ctx *ctx, struct dy_core_recursion rec, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr, dy_array_t *subtype_implicits)
{
    for (size_t i = 0; i < ctx->supertype_assumption_cache.num_elems; ++i) {
        struct dy_subtype_assumption assumption;
        dy_array_get(ctx->supertype_assumption_cache, i, &assumption);

        if (rec.id == assumption.rec_binding_id && dy_are_equal(ctx, supertype, assumption.type) == DY_YES) {
            // Treat as All <: X
            struct dy_core_expr all = {
                .tag = DY_CORE_EXPR_END,
                .end_polarity = DY_CORE_POLARITY_POSITIVE
            };

            return dy_is_subtype_sub(ctx, all, supertype, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, did_transform_subtype_expr, subtype_implicits);
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

    struct dy_core_expr new_expr = substitute(ctx, *rec.expr, rec.id, rec_expr);

    dy_ternary_t result = dy_is_subtype_sub(ctx, new_expr, supertype, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, did_transform_subtype_expr, subtype_implicits);

    dy_core_expr_release(new_expr);

    ctx->supertype_assumption_cache.num_elems = old_size;

    return result;
}

dy_ternary_t is_subtype_of_positive_recursion(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_recursion rec, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr, dy_array_t *subtype_implicits)
{
    for (size_t i = 0; i < ctx->subtype_assumption_cache.num_elems; ++i) {
        struct dy_subtype_assumption assumption;
        dy_array_get(ctx->subtype_assumption_cache, i, &assumption);

        if (rec.id == assumption.rec_binding_id && dy_are_equal(ctx, subtype, assumption.type) == DY_YES) {
            // Treat as X <: Any
            struct dy_core_expr any = {
                .tag = DY_CORE_EXPR_END,
                .end_polarity = DY_CORE_POLARITY_NEGATIVE
            };

            return dy_is_subtype_sub(ctx, subtype, any, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, did_transform_subtype_expr, subtype_implicits);
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

    struct dy_core_expr new_expr = substitute(ctx, *rec.expr, rec.id, rec_expr);

    dy_ternary_t result = dy_is_subtype_sub(ctx, subtype, new_expr, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, did_transform_subtype_expr, subtype_implicits);

    dy_core_expr_release(new_expr);

    ctx->subtype_assumption_cache.num_elems = old_size;

    return result;
}

dy_ternary_t is_subtype_of_negative_recursion(struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_recursion rec, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr, dy_array_t *subtype_implicits)
{
    for (size_t i = 0; i < ctx->subtype_assumption_cache.num_elems; ++i) {
        struct dy_subtype_assumption assumption;
        dy_array_get(ctx->subtype_assumption_cache, i, &assumption);

        if (rec.id == assumption.rec_binding_id && dy_are_equal(ctx, subtype, assumption.type) == DY_YES) {
            // Treat as X <: All
            struct dy_core_expr all = {
                .tag = DY_CORE_EXPR_END,
                .end_polarity = DY_CORE_POLARITY_POSITIVE
            };

            return dy_is_subtype_sub(ctx, subtype, all, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, did_transform_subtype_expr, subtype_implicits);
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

    struct dy_core_expr new_expr = substitute(ctx, *rec.expr, rec.id, rec_expr);

    dy_ternary_t result = dy_is_subtype_sub(ctx, subtype, new_expr, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, did_transform_subtype_expr, subtype_implicits);

    dy_core_expr_release(new_expr);

    ctx->subtype_assumption_cache.num_elems = old_size;

    return result;
}

bool dy_is_vacuous(size_t id, struct dy_core_expr expr)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_EQUALITY_MAP:
        return false;
    case DY_CORE_EXPR_TYPE_MAP:
        return false;
    case DY_CORE_EXPR_EQUALITY_MAP_ELIM:
        return false;
    case DY_CORE_EXPR_TYPE_MAP_ELIM:
        return false;
    case DY_CORE_EXPR_JUNCTION:
        return dy_is_vacuous(id, *expr.junction.e1) && dy_is_vacuous(id, *expr.junction.e2);
    case DY_CORE_EXPR_ALTERNATIVE:
        return false;
    case DY_CORE_EXPR_VARIABLE:
        return expr.variable.id == id;
    case DY_CORE_EXPR_INFERENCE_VARIABLE:
        return expr.inference_variable.id == id;
    case DY_CORE_EXPR_CUSTOM:
        return false;
    case DY_CORE_EXPR_INFERENCE_TYPE_MAP:
        dy_bail("Should not happen\n");
    case DY_CORE_EXPR_RECURSION:
        return dy_is_vacuous(id, *expr.recursion.expr);
    case DY_CORE_EXPR_END:
        return false;
    case DY_CORE_EXPR_SYMBOL:
        return false;
    }

    dy_bail("Impossible object type.");
}
