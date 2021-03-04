/*
 * Copyright 2017-2021 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core.h"

/**
 * Determines the type of the expression 'expr'.
 */
static inline struct dy_core_expr dy_type_of(struct dy_core_ctx *ctx, struct dy_core_expr expr);

static inline struct dy_core_expr dy_type_of_map_assumption(struct dy_core_ctx *ctx, struct dy_core_map_assumption ass, bool is_implicit);
static inline struct dy_core_expr dy_type_of_map_choice(struct dy_core_ctx *ctx, struct dy_core_map_choice choice, bool is_implicit);
static inline struct dy_core_expr dy_type_of_map_recursion(struct dy_core_ctx *ctx, struct dy_core_map_recursion rec, bool is_implicit);

struct dy_core_expr dy_type_of(struct dy_core_ctx *ctx, struct dy_core_expr expr)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_INTRO:
        expr.intro.polarity = DY_POLARITY_POSITIVE;

        switch (expr.intro.tag) {
        case DY_CORE_INTRO_COMPLEX:
            switch (expr.intro.complex.tag) {
            case DY_CORE_COMPLEX_ASSUMPTION:
                dy_core_expr_retain_ptr(ctx, expr.intro.complex.assumption.type);

                dy_array_add(&ctx->free_variables, &(struct dy_free_var){
                    .id = expr.intro.complex.assumption.id,
                    .type = *expr.intro.complex.assumption.type
                });

                expr.intro.complex.assumption.expr = dy_core_expr_new(dy_type_of(ctx, *expr.intro.complex.assumption.expr));

                ctx->free_variables.num_elems--;

                return expr;
            case DY_CORE_COMPLEX_CHOICE:
                expr.intro.complex.choice.left = dy_core_expr_new(dy_type_of(ctx, *expr.intro.complex.choice.left));
                expr.intro.complex.choice.right = dy_core_expr_new(dy_type_of(ctx, *expr.intro.complex.choice.right));
                return expr;
            case DY_CORE_COMPLEX_RECURSION:
                dy_array_add(&ctx->free_variables, &(struct dy_free_var){
                    .id = expr.intro.complex.recursion.id,
                    .type = {
                        .tag = DY_CORE_EXPR_VARIABLE,
                        .variable_id = expr.intro.complex.recursion.id
                    }
                });

                expr.intro.complex.recursion.expr = dy_core_expr_new(dy_type_of(ctx, *expr.intro.complex.recursion.expr));

                ctx->free_variables.num_elems--;

                return expr;
            }

            dy_bail("impossible");
        case DY_CORE_INTRO_SIMPLE:
            if (expr.intro.simple.tag == DY_CORE_SIMPLE_PROOF) {
                dy_core_expr_retain_ptr(ctx, expr.intro.simple.proof);
            }

            expr.intro.simple.out = dy_core_expr_new(dy_type_of(ctx, *expr.intro.simple.out));
            return expr;
        }

        dy_bail("impossible");
    case DY_CORE_EXPR_ELIM:
        return dy_core_expr_retain(ctx, *expr.elim.simple.out);
    case DY_CORE_EXPR_MAP:
        switch (expr.map.tag) {
        case DY_CORE_MAP_ASSUMPTION:
            return dy_type_of_map_assumption(ctx, expr.map.assumption, expr.map.is_implicit);
        case DY_CORE_MAP_CHOICE:
            return dy_type_of_map_choice(ctx, expr.map.choice, expr.map.is_implicit);
        case DY_CORE_MAP_RECURSION:
            return dy_type_of_map_recursion(ctx, expr.map.recursion, expr.map.is_implicit);
        }

        dy_bail("impossible");
    case DY_CORE_EXPR_VARIABLE:
        for (size_t i = ctx->free_variables.num_elems; i-- > 0;) {
            const struct dy_free_var *free_var = dy_array_pos(&ctx->free_variables, i);
            if (free_var->id == expr.variable_id) {
                return dy_core_expr_retain(ctx, free_var->type);
            }
        }

        dy_bail("impossible");
    case DY_CORE_EXPR_ANY:
    case DY_CORE_EXPR_VOID:
        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_VOID
        };
    case DY_CORE_EXPR_INFERENCE_CTX:
        dy_bail("impossible");
    case DY_CORE_EXPR_INFERENCE_VAR:
        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_ANY
        };
    case DY_CORE_EXPR_CUSTOM: {
        const struct dy_core_custom_shared *s = dy_array_pos(&ctx->custom_shared, expr.custom.id);
        return s->type_of(ctx, expr.custom.data);
    }
    }

    dy_bail("Impossible object type.");
}

struct dy_core_expr dy_type_of_map_assumption(struct dy_core_ctx *ctx, struct dy_core_map_assumption ass, bool is_implicit)
{
    if (ass.dependence == DY_CORE_MAP_DEPENDENCE_DEPENDENT) {
        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_ANY
        };
    }

    dy_array_add(&ctx->free_variables, &(struct dy_free_var){
        .id = ass.id,
        .type = *ass.type
    });

    dy_array_add(&ctx->free_variables, &(struct dy_free_var){
        .id = ass.assumption.id,
        .type = *ass.assumption.type
    });

    struct dy_core_expr type = dy_type_of(ctx, *ass.assumption.expr);

    ctx->free_variables.num_elems--;
    ctx->free_variables.num_elems--;

    struct dy_core_expr some_type1 = {
        .tag = DY_CORE_EXPR_INTRO,
        .intro = {
            .polarity = DY_POLARITY_NEGATIVE,
            .is_implicit = is_implicit,
            .tag = DY_CORE_INTRO_COMPLEX,
            .complex = {
                .tag = DY_CORE_COMPLEX_ASSUMPTION,
                .assumption = {
                    .id = ass.id,
                    .type = dy_core_expr_retain_ptr(ctx, ass.type),
                    .expr = dy_core_expr_retain_ptr(ctx, ass.assumption.type)
                }
            }
        }
    };

    struct dy_core_expr some_type2 = {
        .tag = DY_CORE_EXPR_INTRO,
        .intro = {
            .polarity = DY_POLARITY_NEGATIVE,
            .is_implicit = is_implicit,
            .tag = DY_CORE_INTRO_COMPLEX,
            .complex = {
                .tag = DY_CORE_COMPLEX_ASSUMPTION,
                .assumption = {
                    .id = ass.id,
                    .type = dy_core_expr_retain_ptr(ctx, ass.type),
                    .expr = dy_core_expr_new(type)
                }
            }
        }
    };

    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_INTRO,
        .intro = {
            .polarity = DY_POLARITY_POSITIVE,
            .is_implicit = false,
            .tag = DY_CORE_INTRO_COMPLEX,
            .complex = {
                .tag = DY_CORE_COMPLEX_ASSUMPTION,
                .assumption = {
                    .id = ctx->running_id++,
                    .type = dy_core_expr_new(some_type1),
                    .expr = dy_core_expr_new(some_type2)
                }
            }
        }
    };
}

struct dy_core_expr dy_type_of_map_choice(struct dy_core_ctx *ctx, struct dy_core_map_choice choice, bool is_implicit)
{
    if (choice.left_dependence == DY_CORE_MAP_DEPENDENCE_DEPENDENT || choice.right_dependence == DY_CORE_MAP_DEPENDENCE_DEPENDENT) {
        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_ANY
        };
    }

    dy_array_add(&ctx->free_variables, &(struct dy_free_var){
        .id = choice.assumption_left.id,
        .type = *choice.assumption_left.type
    });

    struct dy_core_expr type_left = dy_type_of(ctx, *choice.assumption_left.expr);

    ctx->free_variables.num_elems--;

    dy_array_add(&ctx->free_variables, &(struct dy_free_var){
        .id = choice.assumption_right.id,
        .type = *choice.assumption_right.type
    });

    struct dy_core_expr type_right = dy_type_of(ctx, *choice.assumption_right.expr);

    ctx->free_variables.num_elems--;

    struct dy_core_expr type = {
        .tag = DY_CORE_EXPR_INTRO,
        .intro = {
            .polarity = DY_POLARITY_NEGATIVE,
            .is_implicit = is_implicit,
            .tag = DY_CORE_INTRO_COMPLEX,
            .complex = {
                .tag = DY_CORE_COMPLEX_CHOICE,
                .choice = {
                    .left = dy_core_expr_retain_ptr(ctx, choice.assumption_left.type),
                    .right = dy_core_expr_retain_ptr(ctx, choice.assumption_right.type)
                }
            }
        }
    };

    struct dy_core_expr expr = {
        .tag = DY_CORE_EXPR_INTRO,
        .intro = {
            .polarity = DY_POLARITY_NEGATIVE,
            .is_implicit = is_implicit,
            .tag = DY_CORE_INTRO_COMPLEX,
            .complex = {
                .tag = DY_CORE_COMPLEX_CHOICE,
                .choice = {
                    .left = dy_core_expr_new(type_left),
                    .right = dy_core_expr_new(type_right)
                }
            }
        }
    };

    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_INTRO,
        .intro = {
            .polarity = DY_POLARITY_POSITIVE,
            .is_implicit = false,
            .tag = DY_CORE_INTRO_COMPLEX,
            .complex = {
                .tag = DY_CORE_COMPLEX_ASSUMPTION,
                .assumption = {
                    .id = ctx->running_id++,
                    .type = dy_core_expr_new(type),
                    .expr = dy_core_expr_new(expr)
                }
            }
        }
    };
}

struct dy_core_expr dy_type_of_map_recursion(struct dy_core_ctx *ctx, struct dy_core_map_recursion rec, bool is_implicit)
{
    if (rec.dependence == DY_CORE_MAP_DEPENDENCE_DEPENDENT) {
        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_ANY
        };
    }

    dy_array_add(&ctx->free_variables, &(struct dy_free_var){
        .id = rec.id,
        .type = {
            .tag = DY_CORE_EXPR_VARIABLE,
            .variable_id = rec.id
        }
    });

    dy_array_add(&ctx->free_variables, &(struct dy_free_var){
        .id = rec.assumption.id,
        .type = *rec.assumption.type
    });

    struct dy_core_expr type = dy_type_of(ctx, *rec.assumption.expr);

    ctx->free_variables.num_elems--;
    ctx->free_variables.num_elems--;

    struct dy_core_expr rec_type1 = {
        .tag = DY_CORE_EXPR_INTRO,
        .intro = {
            .polarity = DY_POLARITY_NEGATIVE,
            .is_implicit = is_implicit,
            .tag = DY_CORE_INTRO_COMPLEX,
            .complex = {
                .tag = DY_CORE_COMPLEX_RECURSION,
                .recursion = {
                    .id = rec.id,
                    .expr = dy_core_expr_retain_ptr(ctx, rec.assumption.type)
                }
            }
        }
    };

    struct dy_core_expr rec_type2 = {
        .tag = DY_CORE_EXPR_INTRO,
        .intro = {
            .polarity = DY_POLARITY_NEGATIVE,
            .is_implicit = is_implicit,
            .tag = DY_CORE_INTRO_COMPLEX,
            .complex = {
                .tag = DY_CORE_COMPLEX_RECURSION,
                .recursion = {
                    .id = rec.id,
                    .expr = dy_core_expr_new(type)
                }
            }
        }
    };

    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_INTRO,
        .intro = {
            .polarity = DY_POLARITY_POSITIVE,
            .is_implicit = false,
            .tag = DY_CORE_INTRO_COMPLEX,
            .complex = {
                .tag = DY_CORE_COMPLEX_ASSUMPTION,
                .assumption = {
                    .id = ctx->running_id++,
                    .type = dy_core_expr_new(rec_type1),
                    .expr = dy_core_expr_new(rec_type2)
                }
            }
        }
    };
}
