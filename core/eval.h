/*
 * Copyright 2017-2021 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core.h"
#include "type_of.h"
#include "is_subtype.h"

static inline bool dy_eval_expr(struct dy_core_ctx *ctx, struct dy_core_expr expr, bool *is_value, struct dy_core_expr *result);

static inline bool dy_eval_elim(struct dy_core_ctx *ctx, struct dy_core_elim elim, bool *is_value, struct dy_core_expr *result);

static inline bool dy_eval_simple(struct dy_core_ctx *ctx, struct dy_core_simple simple, bool *is_value, struct dy_core_simple *result);

static inline bool dy_eval_elim_single_step(struct dy_core_ctx *ctx, struct dy_core_elim elim, struct dy_core_expr *result);

static inline bool dy_eval_map_assumption_elim(struct dy_core_ctx *ctx, struct dy_core_map_assumption ass, struct dy_core_expr proof, struct dy_core_expr out, bool is_implicit, enum dy_polarity polarity, struct dy_core_expr *result);

static inline bool dy_eval_map_choice_elim(struct dy_core_ctx *ctx, struct dy_core_map_choice choice, enum dy_direction direction, struct dy_core_expr out, bool is_implicit, enum dy_polarity polarity, struct dy_core_expr *result);

static inline bool dy_eval_map_recursion_elim(struct dy_core_ctx *ctx, struct dy_core_map_recursion rec, struct dy_core_expr out, bool is_implicit, enum dy_polarity polarity, struct dy_core_expr *result);

bool dy_eval_expr(struct dy_core_ctx *ctx, struct dy_core_expr expr, bool *is_value, struct dy_core_expr *result)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_INTRO:
        switch (expr.intro.tag) {
        case DY_CORE_INTRO_COMPLEX:
            switch (expr.intro.complex.tag) {
            case DY_CORE_COMPLEX_ASSUMPTION: {
                struct dy_core_expr new_type;
                if (!dy_eval_expr(ctx, *expr.intro.complex.assumption.type, is_value, &new_type)) {
                    return false;
                }

                expr.intro.complex.assumption.type = dy_core_expr_new(new_type);
                dy_core_expr_retain_ptr(ctx, expr.intro.complex.assumption.expr);
                *result = expr;
                return true;
            }
            case DY_CORE_COMPLEX_CHOICE:
                *is_value = true;
                return false;
            case DY_CORE_COMPLEX_RECURSION:
                *is_value = true;
                return false;
            }

            dy_bail("impossible");
        case DY_CORE_INTRO_SIMPLE:
            if (dy_eval_simple(ctx, expr.intro.simple, is_value, &expr.intro.simple)) {
                *result = expr;
                return true;
            } else {
                return false;
            }
        }

        dy_bail("impossible");
    case DY_CORE_EXPR_ELIM:
        return dy_eval_elim(ctx, expr.elim, is_value, result);
    case DY_CORE_EXPR_MAP:
        // TODO: Actually do stuff with maps.
    case DY_CORE_EXPR_VARIABLE:
    case DY_CORE_EXPR_ANY:
    case DY_CORE_EXPR_VOID:
    case DY_CORE_EXPR_INFERENCE_VAR:
        *is_value = true;
        return false;
    case DY_CORE_EXPR_INFERENCE_CTX:
        dy_bail("impossible");
    case DY_CORE_EXPR_CUSTOM: {
        const struct dy_core_custom_shared *s = dy_array_pos(&ctx->custom_shared, expr.custom.id);
        return s->eval(ctx, expr.custom.data, is_value, result);
    }
    }

    dy_bail("Impossible object type.");
}

bool dy_eval_elim(struct dy_core_ctx *ctx, struct dy_core_elim elim, bool *is_value, struct dy_core_expr *result)
{
    bool expr_is_value;
    struct dy_core_expr new_expr;
    bool expr_is_new = dy_eval_expr(ctx, *elim.expr, &expr_is_value, &new_expr);

    bool simple_is_value;
    struct dy_core_simple new_simple;
    bool simple_is_new = dy_eval_simple(ctx, elim.simple, &simple_is_value, &new_simple);

    if (!expr_is_value || !simple_is_value) {
        *is_value = false;

        if (!expr_is_new && !simple_is_new) {
            return false;
        }

        if (expr_is_new) {
            elim.expr = dy_core_expr_new(new_expr);
        } else {
            dy_core_expr_retain_ptr(ctx, elim.expr);
        }

        if (simple_is_new) {
            elim.simple = new_simple;
        } else {
            dy_core_simple_retain(ctx, elim.simple);
        }

        *result = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_ELIM,
            .elim = elim
        };
        return true;
    }

    if (!expr_is_new) {
        new_expr = dy_core_expr_retain(ctx, *elim.expr);
    }

    if (!simple_is_new) {
        new_simple = dy_core_simple_retain(ctx, elim.simple);
    }

    bool did_transform = false;
    bool changed_check_result = false;
    if (elim.check_result == DY_MAYBE) {
        struct dy_core_expr subtype = dy_type_of(ctx, new_expr);

        struct dy_core_expr supertype = {
            .tag = DY_CORE_EXPR_INTRO,
            .intro = {
                .polarity = DY_POLARITY_NEGATIVE,
                .is_implicit = elim.is_implicit,
                .tag = DY_CORE_INTRO_SIMPLE,
                .simple = new_simple
            }
        };

        dy_ternary_t old_check_result = elim.check_result;

        struct dy_core_expr new_new_expr;
        elim.check_result = dy_is_subtype(ctx, subtype, supertype, new_expr, &new_new_expr, &did_transform);

        if (did_transform) {
            dy_core_expr_release(ctx, new_expr);

            if (dy_eval_expr(ctx, new_new_expr, &expr_is_value, &new_expr)) {
                dy_core_expr_release(ctx, new_new_expr);
                new_new_expr = new_expr;
            }

            new_expr = new_new_expr;
        }

        dy_core_expr_release(ctx, subtype);

        changed_check_result = old_check_result != elim.check_result;
    }

    elim.expr = dy_core_expr_new(new_expr);
    elim.simple = new_simple;

    struct dy_core_expr app_expr = {
        .tag = DY_CORE_EXPR_ELIM,
        .elim = elim
    };

    struct dy_core_expr res;
    if (!dy_eval_elim_single_step(ctx, elim, &res)) {
        *is_value = false;

        if (!expr_is_new && !simple_is_new && !did_transform && !changed_check_result) {
            dy_core_expr_release(ctx, app_expr);
            return false;
        } else {
            *result = app_expr;
            return true;
        }
    }

    dy_core_expr_release(ctx, app_expr);

    if (dy_eval_expr(ctx, res, is_value, result)) {
        dy_core_expr_release(ctx, res);
    } else {
        *result = res;
    }

    return true;
}

bool dy_eval_simple(struct dy_core_ctx *ctx, struct dy_core_simple simple, bool *is_value, struct dy_core_simple *result)
{
    if (simple.tag == DY_CORE_SIMPLE_PROOF) {
        struct dy_core_expr proof;
        if (dy_eval_expr(ctx, *simple.proof, is_value, &proof)) {
            simple.proof = dy_core_expr_new(proof);
            dy_core_expr_retain_ptr(ctx, simple.out);
            *result = simple;
            return true;
        } else {
            return false;
        }

    } else {
        *is_value = true;
        return false;
    }
}

bool dy_eval_elim_single_step(struct dy_core_ctx *ctx, struct dy_core_elim elim, struct dy_core_expr *result)
{
    if (elim.check_result != DY_YES) {
        return false;
    }

    if (elim.expr->tag == DY_CORE_EXPR_INTRO) {
        if (elim.expr->intro.tag == DY_CORE_INTRO_COMPLEX) {
            switch (elim.expr->intro.complex.tag) {
            case DY_CORE_COMPLEX_ASSUMPTION:
                if (!dy_substitute(ctx, *elim.expr->intro.complex.assumption.expr, elim.expr->intro.complex.assumption.id, *elim.simple.proof, result)) {
                    *result = dy_core_expr_retain(ctx, *elim.expr->intro.complex.assumption.expr);
                }

                return true;
            case DY_CORE_COMPLEX_CHOICE:
                if (elim.simple.direction == DY_LEFT) {
                    *result = dy_core_expr_retain(ctx, *elim.expr->intro.complex.choice.left);
                } else {
                    *result = dy_core_expr_retain(ctx, *elim.expr->intro.complex.choice.right);
                }

                return true;
            case DY_CORE_COMPLEX_RECURSION:
                if (!dy_substitute(ctx, *elim.expr->intro.complex.recursion.expr, elim.expr->intro.complex.recursion.id, *elim.expr, result)) {
                    *result = dy_core_expr_retain(ctx, *elim.expr->intro.complex.recursion.expr);
                }

                return true;
            }

            dy_bail("impossible");
        } else {
            *result = dy_core_expr_retain(ctx, *elim.expr->intro.simple.out);
            return true;
        }
    }

    if (elim.expr->tag == DY_CORE_EXPR_MAP) {
        if (elim.simple.proof->tag != DY_CORE_EXPR_INTRO) {
            return false;
        }

        switch (elim.expr->map.tag) {
        case DY_CORE_MAP_ASSUMPTION:
            return dy_eval_map_assumption_elim(ctx, elim.expr->map.assumption, *elim.simple.proof->intro.simple.proof, *elim.simple.proof->intro.simple.out, elim.simple.proof->intro.is_implicit, elim.simple.proof->intro.polarity, result);
        case DY_CORE_MAP_CHOICE:
            return dy_eval_map_choice_elim(ctx, elim.expr->map.choice, elim.simple.proof->intro.simple.direction, *elim.simple.proof->intro.simple.out, elim.simple.proof->intro.is_implicit, elim.simple.proof->intro.polarity, result);
        case DY_CORE_MAP_RECURSION:
            return dy_eval_map_recursion_elim(ctx, elim.expr->map.recursion, *elim.simple.proof->intro.simple.out, elim.simple.proof->intro.is_implicit, elim.simple.proof->intro.polarity, result);
        }

        dy_bail("impossible");
    }

    return false;
}

bool dy_eval_map_assumption_elim(struct dy_core_ctx *ctx, struct dy_core_map_assumption ass, struct dy_core_expr proof, struct dy_core_expr out, bool is_implicit, enum dy_polarity polarity, struct dy_core_expr *result)
{
    struct dy_core_assumption subst_ass;
    if (!dy_substitute_assumption(ctx, ass.assumption, ass.id, proof, &subst_ass)) {
        subst_ass = dy_core_assumption_retain(ctx, ass.assumption);
    }

    dy_array_add(&ctx->free_variables, &(struct dy_free_var){
        .id = subst_ass.id,
        .type = *subst_ass.type
    });

    struct dy_core_expr type = dy_type_of(ctx, *subst_ass.expr);

    ctx->free_variables.num_elems--;

    struct dy_core_expr expr = {
        .tag = DY_CORE_EXPR_INTRO,
        .intro = {
            .polarity = DY_POLARITY_POSITIVE,
            .is_implicit = false,
            .tag = DY_CORE_INTRO_COMPLEX,
            .complex = {
                .tag = DY_CORE_COMPLEX_ASSUMPTION,
                .assumption = subst_ass
            }
        }
    };

    struct dy_core_expr new_out = {
        .tag = DY_CORE_EXPR_ELIM,
        .elim = {
            .expr = dy_core_expr_new(expr),
            .simple = {
                .tag = DY_CORE_SIMPLE_PROOF,
                .proof = dy_core_expr_new(dy_core_expr_retain(ctx, out)),
                .out = dy_core_expr_new(type)
            },
            .is_implicit = false,
            .check_result = DY_YES,
            .eval_immediately = true
        }
    };

    *result = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_INTRO,
        .intro = {
            .is_implicit = is_implicit,
            .polarity = polarity,
            .tag = DY_CORE_INTRO_SIMPLE,
            .simple = {
                .tag = DY_CORE_SIMPLE_PROOF,
                .proof = dy_core_expr_new(dy_core_expr_retain(ctx, proof)),
                .out = dy_core_expr_new(new_out)
            }
        }
    };

    return true;
}

bool dy_eval_map_choice_elim(struct dy_core_ctx *ctx, struct dy_core_map_choice choice, enum dy_direction direction, struct dy_core_expr out, bool is_implicit, enum dy_polarity polarity, struct dy_core_expr *result)
{
    struct dy_core_assumption ass;
    if (direction == DY_LEFT) {
        ass = dy_core_assumption_retain(ctx, choice.assumption_left);
    } else {
        ass = dy_core_assumption_retain(ctx, choice.assumption_right);
    }

    dy_array_add(&ctx->free_variables, &(struct dy_free_var){
        .id = ass.id,
        .type = *ass.type
    });

    struct dy_core_expr type = dy_type_of(ctx, *ass.expr);

    ctx->free_variables.num_elems--;

    struct dy_core_expr expr = {
        .tag = DY_CORE_EXPR_INTRO,
        .intro = {
            .polarity = DY_POLARITY_POSITIVE,
            .is_implicit = false,
            .tag = DY_CORE_INTRO_COMPLEX,
            .complex = {
                .tag = DY_CORE_COMPLEX_ASSUMPTION,
                .assumption = ass
            }
        }
    };

    struct dy_core_expr new_out = {
        .tag = DY_CORE_EXPR_ELIM,
        .elim = {
            .expr = dy_core_expr_new(expr),
            .simple = {
                .tag = DY_CORE_SIMPLE_PROOF,
                .proof = dy_core_expr_new(dy_core_expr_retain(ctx, out)),
                .out = dy_core_expr_new(type)
            },
            .is_implicit = false,
            .check_result = DY_YES,
            .eval_immediately = true
        }
    };

    *result = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_INTRO,
        .intro = {
            .is_implicit = is_implicit,
            .polarity = polarity,
            .tag = DY_CORE_INTRO_SIMPLE,
            .simple = {
                .tag = DY_CORE_SIMPLE_DECISION,
                .direction = direction,
                .out = dy_core_expr_new(new_out)
            }
        }
    };

    return true;
}

bool dy_eval_map_recursion_elim(struct dy_core_ctx *ctx, struct dy_core_map_recursion rec, struct dy_core_expr out, bool is_implicit, enum dy_polarity polarity, struct dy_core_expr *result)
{
    dy_bail("not yet");
}
