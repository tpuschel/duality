/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <duality/core/is_subtype.h>

#include <duality/core/are_equal.h>
#include <duality/core/type_of.h>
#include <duality/core/constraint.h>

#include <duality/support/assert.h>

#include "substitute.h"

#include <stdio.h>

static dy_ternary_t dy_is_subtype_sub(struct dy_check_ctx ctx, struct dy_core_expr subtype, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static dy_ternary_t value_map_is_subtype(struct dy_check_ctx ctx, struct dy_core_value_map value_map, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_value_map);

static dy_ternary_t positive_value_map_is_subtype(struct dy_check_ctx ctx, struct dy_core_value_map value_map, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);
static dy_ternary_t negative_value_map_is_subtype(struct dy_check_ctx ctx, struct dy_core_value_map value_map, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static dy_ternary_t type_map_is_subtype(struct dy_check_ctx ctx, struct dy_core_type_map type_map, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);
static dy_ternary_t positive_type_map_is_subtype(struct dy_check_ctx ctx, struct dy_core_type_map type_map, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);
static dy_ternary_t negative_type_map_is_subtype(struct dy_check_ctx ctx, struct dy_core_type_map type_map, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static dy_ternary_t positive_both_is_subtype(struct dy_check_ctx ctx, struct dy_core_both both, struct dy_core_expr expr, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);
static dy_ternary_t negative_both_is_subtype(struct dy_check_ctx ctx, struct dy_core_both both, struct dy_core_expr expr, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static dy_ternary_t is_subtype_of_positive_both(struct dy_check_ctx ctx, struct dy_core_expr expr, struct dy_core_both both, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);
static dy_ternary_t is_subtype_of_negative_both(struct dy_check_ctx ctx, struct dy_core_expr expr, struct dy_core_both both, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static dy_ternary_t both_is_subtype_of_both(struct dy_check_ctx ctx, struct dy_core_both p1, struct dy_core_both p2, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

static struct dy_core_expr *alloc_expr(struct dy_check_ctx ctx, struct dy_core_expr expr);
static struct dy_constraint *alloc_constraint(struct dy_check_ctx ctx, struct dy_constraint constraint);

dy_ternary_t dy_is_subtype(struct dy_check_ctx ctx, struct dy_core_expr subtype, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr)
{
    bool did_transform_expr = false;
    dy_ternary_t result = dy_is_subtype_sub(ctx, subtype, supertype, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, &did_transform_expr);

    if (!did_transform_expr) {
        *new_subtype_expr = subtype_expr;
    }

    return result;
}

dy_ternary_t dy_is_subtype_no_transformation(struct dy_check_ctx ctx, struct dy_core_expr subtype, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint)
{
    struct dy_core_expr e;
    bool did_transform_e = false;
    dy_ternary_t result = dy_is_subtype_sub(ctx, subtype, supertype, constraint, did_generate_constraint, e, &e, &did_transform_e);
    if (did_transform_e) {
        return DY_NO;
    }

    return result;
}

dy_ternary_t dy_is_subtype_sub(struct dy_check_ctx ctx, struct dy_core_expr subtype, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    if (subtype.tag == DY_CORE_EXPR_UNKNOWN && subtype.unknown.is_inference_var && supertype.tag == DY_CORE_EXPR_UNKNOWN && supertype.unknown.is_inference_var) {
        if (subtype.unknown.id == supertype.unknown.id) {
            return DY_YES;
        }
    }

    if (subtype.tag == DY_CORE_EXPR_UNKNOWN && subtype.unknown.is_inference_var) {
        *constraint = (struct dy_constraint){
            .tag = DY_CONSTRAINT_SINGLE,
            .single = {
                .id = subtype.unknown.id,
                .range = {
                    .have_subtype = false,
                    .have_supertype = true,
                    .supertype = supertype,
                },
            }
        };

        *did_generate_constraint = true;

        return DY_YES;
    }

    if (supertype.tag == DY_CORE_EXPR_UNKNOWN && supertype.unknown.is_inference_var) {
        *constraint = (struct dy_constraint){
            .tag = DY_CONSTRAINT_SINGLE,
            .single = {
                .id = supertype.unknown.id,
                .range = {
                    .have_supertype = false,
                    .have_subtype = true,
                    .subtype = subtype,
                },
            }
        };

        *did_generate_constraint = true;

        return DY_YES;
    }

    if (subtype.tag == DY_CORE_EXPR_END && subtype.end_polarity == DY_CORE_POLARITY_NEGATIVE) {
        return DY_YES;
    }

    if (supertype.tag == DY_CORE_EXPR_END && supertype.end_polarity == DY_CORE_POLARITY_NEGATIVE) {
        return DY_MAYBE;
    }

    if (supertype.tag == DY_CORE_EXPR_END && supertype.end_polarity == DY_CORE_POLARITY_POSITIVE) {
        return DY_YES;
    }

    if (subtype.tag == DY_CORE_EXPR_END && subtype.end_polarity == DY_CORE_POLARITY_POSITIVE) {
        return DY_MAYBE;
    }

    if (subtype.tag == DY_CORE_EXPR_BOTH && subtype.both.polarity == DY_CORE_POLARITY_POSITIVE) {
        return positive_both_is_subtype(ctx, subtype.both, supertype, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
    }

    if (supertype.tag == DY_CORE_EXPR_BOTH && supertype.both.polarity == DY_CORE_POLARITY_POSITIVE) {
        return is_subtype_of_positive_both(ctx, subtype, supertype.both, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
    }

    if (subtype.tag == DY_CORE_EXPR_BOTH && subtype.both.polarity == DY_CORE_POLARITY_NEGATIVE) {
        return negative_both_is_subtype(ctx, subtype.both, supertype, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
    }

    if (supertype.tag == DY_CORE_EXPR_BOTH && supertype.both.polarity == DY_CORE_POLARITY_NEGATIVE) {
        return is_subtype_of_negative_both(ctx, subtype, supertype.both, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
    }

    if (supertype.tag == DY_CORE_EXPR_VALUE_MAP_ELIM || supertype.tag == DY_CORE_EXPR_TYPE_MAP_ELIM || supertype.tag == DY_CORE_EXPR_UNKNOWN || supertype.tag == DY_CORE_EXPR_ONE_OF) {
        return dy_are_equal(ctx, subtype, supertype);
    }

    switch (subtype.tag) {
    case DY_CORE_EXPR_VALUE_MAP:
        return value_map_is_subtype(ctx, subtype.value_map, supertype, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
    case DY_CORE_EXPR_TYPE_MAP:
        return type_map_is_subtype(ctx, subtype.type_map, supertype, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
    case DY_CORE_EXPR_END:
        dy_bail("should not be reached");
    case DY_CORE_EXPR_VALUE_MAP_ELIM:
        // fallthrough
    case DY_CORE_EXPR_TYPE_MAP_ELIM:
        // fallthrough
    case DY_CORE_EXPR_ONE_OF:
        // fallthrough
    case DY_CORE_EXPR_UNKNOWN:
        // fallthrough
    case DY_CORE_EXPR_TYPE_OF_STRINGS:
        // fallthrough
    case DY_CORE_EXPR_BOTH:
        // fallthrough
    case DY_CORE_EXPR_PRINT:
        // fallthrough
    case DY_CORE_EXPR_STRING:
        return dy_are_equal(ctx, subtype, supertype);
    }

    DY_IMPOSSIBLE_ENUM();
}

dy_ternary_t value_map_is_subtype(struct dy_check_ctx ctx, struct dy_core_value_map value_map, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    switch (value_map.polarity) {
    case DY_CORE_POLARITY_POSITIVE:
        return positive_value_map_is_subtype(ctx, value_map, supertype, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
    case DY_CORE_POLARITY_NEGATIVE:
        return negative_value_map_is_subtype(ctx, value_map, supertype, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
    }

    DY_IMPOSSIBLE_ENUM();
}

dy_ternary_t positive_value_map_is_subtype(struct dy_check_ctx ctx, struct dy_core_value_map value_map, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    if (supertype.tag == DY_CORE_EXPR_VALUE_MAP && value_map.is_implicit == supertype.value_map.is_implicit) {
        dy_ternary_t are_equal = dy_are_equal(ctx, *value_map.e1, *supertype.value_map.e1);
        if (are_equal == DY_NO) {
            return DY_NO;
        }

        struct dy_core_expr subtype_expr_vmap_e2 = {
            .tag = DY_CORE_EXPR_VALUE_MAP_ELIM,
            .value_map_elim = {
                .id = (*ctx.running_id)++,
                .expr = alloc_expr(ctx, subtype_expr),
                .value_map = {
                    .e1 = value_map.e1,
                    .e2 = value_map.e2,
                    .polarity = DY_CORE_POLARITY_NEGATIVE,
                    .is_implicit = false,
                },
            }
        };

        struct dy_core_expr transformed_subtype_expr_vmap_e2;
        bool did_transform_subtype_expr_vmap_e2 = false;
        dy_ternary_t is_subtype = dy_is_subtype_sub(ctx, *value_map.e2, *supertype.value_map.e2, constraint, did_generate_constraint, subtype_expr_vmap_e2, &transformed_subtype_expr_vmap_e2, &did_transform_subtype_expr_vmap_e2);
        if (is_subtype == DY_NO) {
            return DY_NO;
        }

        if (did_transform_subtype_expr_vmap_e2) {
            *new_subtype_expr = (struct dy_core_expr){
                .tag = DY_CORE_EXPR_VALUE_MAP,
                .value_map = {
                    .e1 = value_map.e1,
                    .e2 = alloc_expr(ctx, transformed_subtype_expr_vmap_e2),
                    .polarity = DY_CORE_POLARITY_POSITIVE,
                    .is_implicit = false,
                }
            };

            *did_transform_subtype_expr = true;
        }

        if (are_equal == DY_MAYBE || is_subtype == DY_MAYBE) {
            return DY_MAYBE;
        }

        return DY_YES;
    }

    if (supertype.tag == DY_CORE_EXPR_TYPE_MAP && supertype.type_map.polarity == DY_CORE_POLARITY_NEGATIVE && value_map.is_implicit == supertype.type_map.is_implicit) {
        struct dy_constraint c1;
        bool have_c1 = false;
        struct dy_core_expr vmap_e1 = *value_map.e1;
        bool did_transform_vmap_e1 = false;
        dy_ternary_t is_subtype_in = dy_is_subtype_sub(ctx, dy_type_of(ctx, *value_map.e1), *supertype.type_map.arg_type, &c1, &have_c1, vmap_e1, &vmap_e1, &did_transform_vmap_e1);
        if (is_subtype_in == DY_NO) {
            return DY_NO;
        }

        struct dy_core_expr vmap_e2 = {
            .tag = DY_CORE_EXPR_VALUE_MAP_ELIM,
            .value_map_elim = {
                .id = (*ctx.running_id)++,
                .expr = alloc_expr(ctx, subtype_expr),
                .value_map = {
                    .e1 = value_map.e1,
                    .e2 = value_map.e2,
                    .polarity = DY_CORE_POLARITY_NEGATIVE,
                    .is_implicit = false,
                },
            }
        };

        struct dy_constraint c2;
        bool have_c2 = false;
        bool did_transform_vmap_e2 = false;
        dy_ternary_t is_subtype_out = dy_is_subtype_sub(ctx, *value_map.e2, *supertype.type_map.expr, &c2, &have_c2, vmap_e2, &vmap_e2, &did_transform_vmap_e2);
        if (is_subtype_out == DY_NO) {
            return DY_NO;
        }

        if (did_transform_vmap_e1 || did_transform_vmap_e2) {
            *new_subtype_expr = (struct dy_core_expr){
                .tag = DY_CORE_EXPR_VALUE_MAP,
                .value_map = {
                    .e1 = alloc_expr(ctx, vmap_e1),
                    .e2 = alloc_expr(ctx, vmap_e2),
                    .polarity = value_map.polarity,
                    .is_implicit = value_map.is_implicit,
                }
            };

            *did_transform_subtype_expr = true;
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

        if (is_subtype_in == DY_MAYBE || is_subtype_out == DY_MAYBE) {
            return DY_MAYBE;
        }

        return DY_YES;
    }

    return DY_NO;
}

dy_ternary_t negative_value_map_is_subtype(struct dy_check_ctx ctx, struct dy_core_value_map value_map, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    if (supertype.tag == DY_CORE_EXPR_VALUE_MAP && supertype.value_map.polarity == DY_CORE_POLARITY_NEGATIVE && value_map.is_implicit == supertype.value_map.is_implicit) {
        dy_ternary_t are_equal = dy_are_equal(ctx, *value_map.e1, *supertype.value_map.e1);
        if (are_equal == DY_NO) {
            return DY_NO;
        }

        struct dy_core_expr vmap_e2 = {
            .tag = DY_CORE_EXPR_VALUE_MAP_ELIM,
            .value_map_elim = {
                .id = (*ctx.running_id)++,
                .expr = alloc_expr(ctx, subtype_expr),
                .value_map = {
                    .e1 = value_map.e1,
                    .e2 = value_map.e2,
                    .polarity = DY_CORE_POLARITY_NEGATIVE,
                    .is_implicit = false,
                },
            }
        };

        bool did_transform_vmap_e2 = false;
        dy_ternary_t is_subtype = dy_is_subtype_sub(ctx, *value_map.e2, *supertype.value_map.e2, constraint, did_generate_constraint, vmap_e2, &vmap_e2, &did_transform_vmap_e2);
        if (is_subtype == DY_NO) {
            return DY_NO;
        }

        if (did_transform_vmap_e2) {
            *new_subtype_expr = (struct dy_core_expr){
                .tag = DY_CORE_EXPR_VALUE_MAP,
                .value_map = {
                    .e1 = value_map.e1,
                    .e2 = alloc_expr(ctx, vmap_e2),
                    .polarity = value_map.polarity,
                    .is_implicit = value_map.is_implicit,
                }
            };

            *did_transform_subtype_expr = true;
        }

        if (are_equal == DY_MAYBE || is_subtype == DY_MAYBE) {
            return DY_MAYBE;
        }

        return DY_YES;
    }

    return DY_NO;
}

dy_ternary_t type_map_is_subtype(struct dy_check_ctx ctx, struct dy_core_type_map type_map, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    switch (type_map.polarity) {
    case DY_CORE_POLARITY_POSITIVE:
        return positive_type_map_is_subtype(ctx, type_map, supertype, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
    case DY_CORE_POLARITY_NEGATIVE:
        return negative_type_map_is_subtype(ctx, type_map, supertype, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
    }

    DY_IMPOSSIBLE_ENUM();
}

dy_ternary_t positive_type_map_is_subtype(struct dy_check_ctx ctx, struct dy_core_type_map type_map, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    if (supertype.tag == DY_CORE_EXPR_VALUE_MAP && supertype.value_map.polarity == DY_CORE_POLARITY_NEGATIVE && type_map.is_implicit == supertype.value_map.is_implicit) {
        struct dy_core_unknown var = {
            .id = (*ctx.running_id)++,
            .type = alloc_expr(ctx, dy_type_of(ctx, *supertype.value_map.e1)),
            .is_inference_var = false
        };

        struct dy_core_expr var_expr = {
            .tag = DY_CORE_EXPR_UNKNOWN,
            .unknown = var
        };

        struct dy_constraint c1;
        bool have_c1 = false;
        bool did_transform_var_expr = false;
        dy_ternary_t is_subtype_in = dy_is_subtype_sub(ctx, *var.type, *type_map.arg_type, &c1, &have_c1, var_expr, &var_expr, &did_transform_var_expr);
        if (is_subtype_in == DY_NO) {
            return DY_NO;
        }

        struct dy_core_expr vmap_e1 = substitute(ctx, var.id, *supertype.value_map.e1, var_expr);
        struct dy_core_expr new_type_map_expr = substitute(ctx, type_map.arg_id, vmap_e1, *type_map.expr);

        struct dy_core_expr vmap_e2 = {
            .tag = DY_CORE_EXPR_VALUE_MAP_ELIM,
            .value_map_elim = {
                .id = (*ctx.running_id)++,
                .expr = alloc_expr(ctx, subtype_expr),
                .value_map = {
                    .e1 = alloc_expr(ctx, var_expr),
                    .e2 = alloc_expr(ctx, new_type_map_expr),
                    .polarity = DY_CORE_POLARITY_NEGATIVE,
                    .is_implicit = false,
                },
            }
        };

        struct dy_constraint c2;
        bool have_c2 = false;
        bool did_transform_vmap_e2 = false;
        dy_ternary_t is_subtype_out = dy_is_subtype_sub(ctx, new_type_map_expr, *supertype.value_map.e2, &c2, &have_c2, vmap_e2, &vmap_e2, &did_transform_vmap_e2);
        if (is_subtype_out == DY_NO) {
            return DY_NO;
        }

        if (did_transform_var_expr || did_transform_vmap_e2) {
            *new_subtype_expr = (struct dy_core_expr){
                .tag = DY_CORE_EXPR_TYPE_MAP,
                .type_map = {
                    .arg_id = var.id,
                    .arg_type = var.type,
                    .expr = alloc_expr(ctx, vmap_e2),
                    .polarity = DY_CORE_POLARITY_POSITIVE,
                    .is_implicit = false,
                }
            };

            *did_transform_subtype_expr = true;
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

        if (is_subtype_in == DY_MAYBE || is_subtype_out == DY_MAYBE) {
            return DY_MAYBE;
        }

        return DY_YES;
    }

    if (supertype.tag == DY_CORE_EXPR_TYPE_MAP && supertype.type_map.polarity == DY_CORE_POLARITY_POSITIVE && type_map.is_implicit == supertype.type_map.is_implicit) {
        struct dy_core_unknown var = {
            .id = (*ctx.running_id)++,
            .type = supertype.type_map.arg_type,
            .is_inference_var = false
        };

        struct dy_core_expr var_expr = {
            .tag = DY_CORE_EXPR_UNKNOWN,
            .unknown = var
        };

        struct dy_constraint c1;
        bool have_c1 = false;
        bool did_transform_var_expr = false;
        dy_ternary_t is_subtype_in = dy_is_subtype_sub(ctx, *supertype.type_map.arg_type, *type_map.arg_type, &c1, &have_c1, var_expr, &var_expr, &did_transform_var_expr);
        if (is_subtype_in == DY_NO) {
            return DY_NO;
        }

        struct dy_core_expr supertype_arg_expr = {
            .tag = DY_CORE_EXPR_UNKNOWN,
            .unknown = {
                .id = supertype.type_map.arg_id,
                .type = supertype.type_map.arg_type,
                .is_inference_var = false,
            }
        };

        struct dy_core_expr transformed_arg_expr = substitute(ctx, var.id, supertype_arg_expr, var_expr);
        struct dy_core_expr new_type_map_expr = substitute(ctx, type_map.arg_id, transformed_arg_expr, *type_map.expr);

        struct dy_core_expr type_map_expr_value = {
            .tag = DY_CORE_EXPR_VALUE_MAP_ELIM,
            .value_map_elim = {
                .id = (*ctx.running_id)++,
                .expr = alloc_expr(ctx, subtype_expr),
                .value_map = {
                    .e1 = alloc_expr(ctx, var_expr),
                    .e2 = alloc_expr(ctx, new_type_map_expr),
                    .polarity = DY_CORE_POLARITY_NEGATIVE,
                    .is_implicit = false,
                },
            }
        };

        struct dy_constraint c2;
        bool have_c2 = false;
        bool did_transform_type_map_expr_value = false;
        dy_ternary_t is_subtype_out = dy_is_subtype_sub(ctx, new_type_map_expr, *supertype.type_map.expr, &c2, &have_c2, type_map_expr_value, &type_map_expr_value, &did_transform_type_map_expr_value);
        if (is_subtype_out == DY_NO) {
            return DY_NO;
        }

        if (did_transform_var_expr || did_transform_type_map_expr_value) {
            *new_subtype_expr = (struct dy_core_expr){
                .tag = DY_CORE_EXPR_TYPE_MAP,
                .type_map = {
                    .arg_id = var.id,
                    .arg_type = var.type,
                    .expr = alloc_expr(ctx, type_map_expr_value),
                    .polarity = DY_CORE_POLARITY_POSITIVE,
                    .is_implicit = false,
                }
            };

            *did_transform_subtype_expr = true;
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

        if (is_subtype_in == DY_MAYBE || is_subtype_out == DY_MAYBE) {
            return DY_MAYBE;
        }

        return DY_YES;
    }

    if (type_map.is_implicit) {
        size_t id = (*ctx.running_id)++;

        struct dy_core_expr unknown = {
            .tag = DY_CORE_EXPR_UNKNOWN,
            .unknown = {
                .id = id,
                .type = type_map.arg_type,
                .is_inference_var = true,
            }
        };

        struct dy_core_expr new_type = substitute(ctx, type_map.arg_id, unknown, *type_map.expr);

        struct dy_core_expr e = {
            .tag = DY_CORE_EXPR_VALUE_MAP_ELIM,
            .value_map_elim = {
                .id = (*ctx.running_id)++,
                .expr = alloc_expr(ctx, subtype_expr),
                .value_map = {
                    .e1 = alloc_expr(ctx, unknown),
                    .e2 = alloc_expr(ctx, new_type),
                    .polarity = DY_CORE_POLARITY_NEGATIVE,
                    .is_implicit = true,
                },
            }
        };

        struct dy_constraint c;
        bool have_c = false;
        bool did_transform_e = false;
        dy_ternary_t result = dy_is_subtype_sub(ctx, new_type, supertype, &c, &have_c, e, &e, &did_transform_e);
        if (result == DY_NO) {
            return DY_NO;
        }

        if (!have_c) {
            fprintf(stderr, "[is_subtype] No constraints, assigning All.\n");

            struct dy_core_expr all = {
                .tag = DY_CORE_EXPR_END,
                .end_polarity = DY_CORE_POLARITY_POSITIVE
            };

            *new_subtype_expr = substitute(ctx, id, all, e);
            *did_transform_subtype_expr = true;

            return result;
        }

        dy_array_t *ids = dy_array_create(ctx.allocator, sizeof(size_t), 4);
        dy_binding_contraints(ctx, id, c, have_c, ids);

        if (dy_array_size(ids) != 0) {
            struct dy_bound_constraint bound_constraint = {
                .id = id,
                .type = *type_map.arg_type,
                .binding_ids = ids
            };

            dy_array_add(ctx.bound_constraints, &bound_constraint);

            *new_subtype_expr = e;
            *did_transform_subtype_expr = true;

            if (have_c) {
                *constraint = c;
                *did_generate_constraint = true;
            }

            return result;
        }

        dy_array_destroy(ids);

        struct dy_constraint_range solution = dy_constraint_collect(ctx, c, id);

        if (solution.have_supertype) {
            e = substitute(ctx, id, solution.supertype, e);
        } else {
            struct dy_core_expr all = {
                .tag = DY_CORE_EXPR_END,
                .end_polarity = DY_CORE_POLARITY_POSITIVE
            };

            e = substitute(ctx, id, all, e);
        }

        have_c = false;
        result = dy_is_subtype(ctx, dy_type_of(ctx, e), supertype, &c, &have_c, e, &e);
        if (result == DY_NO) {
            return DY_NO;
        }

        *new_subtype_expr = e;
        *did_transform_subtype_expr = true;

        if (have_c) {
            *constraint = c;
            *did_generate_constraint = true;
        }

        return result;
    }

    return DY_NO;
}

dy_ternary_t negative_type_map_is_subtype(struct dy_check_ctx ctx, struct dy_core_type_map type_map, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    if (supertype.tag == DY_CORE_EXPR_TYPE_MAP && supertype.type_map.polarity == DY_CORE_POLARITY_NEGATIVE && type_map.is_implicit == supertype.type_map.is_implicit) {
        struct dy_core_unknown var = {
            .id = (*ctx.running_id)++,
            .type = supertype.type_map.arg_type,
            .is_inference_var = false
        };

        struct dy_core_expr var_expr = {
            .tag = DY_CORE_EXPR_UNKNOWN,
            .unknown = var
        };

        struct dy_constraint c1;
        bool have_c1 = false;
        bool did_transform_var_expr = false;
        dy_ternary_t is_subtype_in = dy_is_subtype_sub(ctx, *supertype.type_map.arg_type, *type_map.arg_type, &c1, &have_c1, var_expr, &var_expr, &did_transform_var_expr);
        if (is_subtype_in == DY_NO) {
            return DY_NO;
        }

        struct dy_core_expr supertype_arg_expr = {
            .tag = DY_CORE_EXPR_UNKNOWN,
            .unknown = {
                .id = supertype.type_map.arg_id,
                .type = supertype.type_map.arg_type,
                .is_inference_var = false,
            }
        };

        struct dy_core_expr transformed_arg_expr = substitute(ctx, var.id, supertype_arg_expr, var_expr);
        struct dy_core_expr new_type_map_expr = substitute(ctx, type_map.arg_id, transformed_arg_expr, *type_map.expr);

        struct dy_core_expr type_map_expr_value = {
            .tag = DY_CORE_EXPR_VALUE_MAP_ELIM,
            .value_map_elim = {
                .id = (*ctx.running_id)++,
                .expr = alloc_expr(ctx, subtype_expr),
                .value_map = {
                    .e1 = alloc_expr(ctx, var_expr),
                    .e2 = alloc_expr(ctx, new_type_map_expr),
                    .polarity = DY_CORE_POLARITY_NEGATIVE,
                    .is_implicit = false,
                },
            }
        };

        struct dy_constraint c2;
        bool have_c2 = false;
        bool did_transform_type_map_expr_value = false;
        dy_ternary_t is_subtype_out = dy_is_subtype_sub(ctx, new_type_map_expr, *supertype.type_map.expr, &c2, &have_c2, type_map_expr_value, &type_map_expr_value, &did_transform_type_map_expr_value);
        if (is_subtype_out == DY_NO) {
            return DY_NO;
        }

        if (did_transform_var_expr || did_transform_type_map_expr_value) {
            *new_subtype_expr = (struct dy_core_expr){
                .tag = DY_CORE_EXPR_TYPE_MAP,
                .type_map = {
                    .arg_id = var.id,
                    .arg_type = var.type,
                    .expr = alloc_expr(ctx, type_map_expr_value),
                    .polarity = DY_CORE_POLARITY_NEGATIVE,
                    .is_implicit = false,
                }
            };

            *did_transform_subtype_expr = true;
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

        if (is_subtype_in == DY_MAYBE || is_subtype_out == DY_MAYBE) {
            return DY_MAYBE;
        }

        return DY_YES;
    }

    return DY_NO;
}

dy_ternary_t positive_both_is_subtype(struct dy_check_ctx ctx, struct dy_core_both both, struct dy_core_expr expr, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    if (expr.tag == DY_CORE_EXPR_BOTH && expr.both.polarity == DY_CORE_POLARITY_POSITIVE) {
        return both_is_subtype_of_both(ctx, both, expr.both, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
    }

    struct dy_constraint c1;
    bool have_c1 = false;
    struct dy_core_expr e1 = subtype_expr;
    bool did_transform_e1 = false;
    dy_ternary_t first_res = dy_is_subtype_sub(ctx, *both.e1, expr, &c1, &have_c1, e1, &e1, &did_transform_e1);

    struct dy_constraint c2;
    bool have_c2 = false;
    struct dy_core_expr e2 = subtype_expr;
    bool did_transform_e2 = false;
    dy_ternary_t second_res = dy_is_subtype_sub(ctx, *both.e2, expr, &c2, &have_c2, e2, &e2, &did_transform_e2);

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
    }

    if (did_transform_e1 || did_transform_e2) {
        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_BOTH,
            .both = {
                .e1 = alloc_expr(ctx, e1),
                .e2 = alloc_expr(ctx, e2),
                .polarity = DY_CORE_POLARITY_POSITIVE,
            }
        };

        *did_transform_subtype_expr = true;
    }

    return first_res;
}

dy_ternary_t negative_both_is_subtype(struct dy_check_ctx ctx, struct dy_core_both both, struct dy_core_expr expr, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    if (expr.tag == DY_CORE_EXPR_BOTH && expr.both.polarity == DY_CORE_POLARITY_NEGATIVE) {
        return both_is_subtype_of_both(ctx, both, expr.both, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
    }

    struct dy_constraint c1;
    bool have_c1 = false;
    struct dy_core_expr e1 = subtype_expr;
    bool did_transform_e1 = false;
    dy_ternary_t first_result = dy_is_subtype_sub(ctx, *both.e1, expr, &c1, &have_c1, e1, &e1, &did_transform_e1);

    struct dy_constraint c2;
    bool have_c2 = false;
    struct dy_core_expr e2 = subtype_expr;
    bool did_transform_e2 = false;
    dy_ternary_t second_result = dy_is_subtype_sub(ctx, *both.e2, expr, &c2, &have_c2, e2, &e2, &did_transform_e2);

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
    }

    if (did_transform_e1 || did_transform_e2) {
        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_ONE_OF,
            .one_of = {
                .first = alloc_expr(ctx, e1),
                .second = alloc_expr(ctx, e2),
            }
        };

        *did_transform_subtype_expr = true;
    }

    if (first_result == DY_YES && second_result == DY_YES) {
        return DY_YES;
    }

    return DY_MAYBE;
}

dy_ternary_t is_subtype_of_positive_both(struct dy_check_ctx ctx, struct dy_core_expr expr, struct dy_core_both both, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    if (expr.tag == DY_CORE_EXPR_BOTH && expr.both.polarity == DY_CORE_POLARITY_POSITIVE) {
        return both_is_subtype_of_both(ctx, expr.both, both, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
    }

    struct dy_constraint c1;
    bool have_c1 = false;
    struct dy_core_expr e1 = subtype_expr;
    bool did_transform_e1 = false;
    dy_ternary_t first_result = dy_is_subtype_sub(ctx, expr, *both.e1, &c1, &have_c1, e1, &e1, &did_transform_e1);
    if (first_result == DY_NO) {
        return DY_NO;
    }

    struct dy_constraint c2;
    bool have_c2 = false;
    struct dy_core_expr e2 = subtype_expr;
    bool did_transform_e2 = false;
    dy_ternary_t second_result = dy_is_subtype_sub(ctx, expr, *both.e2, &c2, &have_c2, e2, &e2, &did_transform_e2);
    if (second_result == DY_NO) {
        return DY_NO;
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

    if (did_transform_e1 || did_transform_e2) {
        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_BOTH,
            .both = {
                .e1 = alloc_expr(ctx, e1),
                .e2 = alloc_expr(ctx, e2),
                .polarity = DY_CORE_POLARITY_POSITIVE,
            }
        };

        *did_transform_subtype_expr = true;
    }

    if (first_result == DY_MAYBE || second_result == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

dy_ternary_t is_subtype_of_negative_both(struct dy_check_ctx ctx, struct dy_core_expr expr, struct dy_core_both both, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    if (expr.tag == DY_CORE_EXPR_BOTH && expr.both.polarity == DY_CORE_POLARITY_NEGATIVE) {
        return both_is_subtype_of_both(ctx, expr.both, both, constraint, did_generate_constraint, subtype_expr, new_subtype_expr, did_transform_subtype_expr);
    }

    struct dy_constraint c1;
    bool have_c1 = false;
    struct dy_core_expr e1 = subtype_expr;
    bool did_transform_e1 = false;
    dy_ternary_t first_res = dy_is_subtype_sub(ctx, expr, *both.e1, &c1, &have_c1, e1, &e1, &did_transform_e1);

    struct dy_constraint c2;
    bool have_c2 = false;
    struct dy_core_expr e2 = subtype_expr;
    bool did_transform_e2 = false;
    dy_ternary_t second_res = dy_is_subtype_sub(ctx, expr, *both.e2, &c2, &have_c2, e2, &e2, &did_transform_e2);

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
    }

    if (did_transform_e1 || did_transform_e2) {
        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_BOTH,
            .both = {
                .e1 = alloc_expr(ctx, e1),
                .e2 = alloc_expr(ctx, e2),
                .polarity = DY_CORE_POLARITY_POSITIVE,
            }
        };

        *did_transform_subtype_expr = true;
    }

    if (first_res == DY_MAYBE && second_res == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

dy_ternary_t both_is_subtype_of_both(struct dy_check_ctx ctx, struct dy_core_both p1, struct dy_core_both p2, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr)
{
    struct dy_constraint c1;
    bool have_c1 = false;
    struct dy_core_expr e1 = subtype_expr;
    bool did_transform_e1 = false;
    dy_ternary_t first_result = dy_is_subtype_sub(ctx, *p1.e1, *p2.e1, &c1, &have_c1, e1, &e1, &did_transform_e1);
    if (first_result == DY_NO) {
        return DY_NO;
    }

    struct dy_constraint c2;
    bool have_c2 = false;
    struct dy_core_expr e2 = subtype_expr;
    bool did_transform_e2 = false;
    dy_ternary_t second_result = dy_is_subtype_sub(ctx, *p1.e2, *p2.e2, &c2, &have_c2, e2, &e2, &did_transform_e2);
    if (second_result == DY_NO) {
        return DY_NO;
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

    if (did_transform_e1 || did_transform_e2) {
        *new_subtype_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_BOTH,
            .both = {
                .e1 = alloc_expr(ctx, e1),
                .e2 = alloc_expr(ctx, e2),
                .polarity = DY_CORE_POLARITY_POSITIVE,
            }
        };

        *did_transform_subtype_expr = true;
    }

    if (first_result == DY_MAYBE || second_result == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

struct dy_core_expr *alloc_expr(struct dy_check_ctx ctx, struct dy_core_expr expr)
{
    return dy_alloc(&expr, sizeof expr, ctx.allocator);
}

struct dy_constraint *alloc_constraint(struct dy_check_ctx ctx, struct dy_constraint constraint)
{
    return dy_alloc(&constraint, sizeof constraint, ctx.allocator);
}
