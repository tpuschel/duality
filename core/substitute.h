/*
 * Copyright 2017-2021 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core.h"

#include "../support/util.h"

/**
 * This file implements substitution for every object of Core.
 */

static inline bool dy_substitute(struct dy_core_ctx *ctx, struct dy_core_expr expr, size_t id, struct dy_core_expr sub, struct dy_core_expr *result);

static inline bool dy_substitute_assumption(struct dy_core_ctx *ctx, struct dy_core_assumption function, size_t id, struct dy_core_expr sub, struct dy_core_assumption *result);

static inline bool dy_substitute_recursion(struct dy_core_ctx *ctx, struct dy_core_recursion recursion, size_t id, struct dy_core_expr sub, struct dy_core_recursion *result);

static inline bool dy_substitute_simple(struct dy_core_ctx *ctx, struct dy_core_simple simple, size_t id, struct dy_core_expr sub, struct dy_core_simple *result);

static inline bool dy_substitute_map_assumption(struct dy_core_ctx *ctx, struct dy_core_map_assumption ass, size_t id, struct dy_core_expr sub, struct dy_core_map_assumption *result);

static inline bool dy_substitute_map_choice(struct dy_core_ctx *ctx, struct dy_core_map_choice choice, size_t id, struct dy_core_expr sub, struct dy_core_map_choice *result);

static inline bool dy_substitute_map_recursion(struct dy_core_ctx *ctx, struct dy_core_map_recursion rec, size_t id, struct dy_core_expr sub, struct dy_core_map_recursion *result);

bool dy_substitute(struct dy_core_ctx *ctx, struct dy_core_expr expr, size_t id, struct dy_core_expr sub, struct dy_core_expr *result)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_INTRO:
        switch (expr.intro.tag) {
        case DY_CORE_INTRO_COMPLEX:
            switch (expr.intro.complex.tag) {
            case DY_CORE_COMPLEX_ASSUMPTION:
                if (dy_substitute_assumption(ctx, expr.intro.complex.assumption, id, sub, &expr.intro.complex.assumption)) {
                    *result = expr;
                    return true;
                } else {
                    return false;
                }
            case DY_CORE_COMPLEX_CHOICE: {
                struct dy_core_expr left;
                bool left_is_new = dy_substitute(ctx, *expr.intro.complex.choice.left, id, sub, &left);

                struct dy_core_expr right;
                bool right_is_new = dy_substitute(ctx, *expr.intro.complex.choice.right, id, sub, &right);

                if (!left_is_new && !right_is_new) {
                    return false;
                }

                if (left_is_new) {
                    expr.intro.complex.choice.left = dy_core_expr_new(left);
                } else {
                    dy_core_expr_retain_ptr(ctx, expr.intro.complex.choice.left);
                }

                if (right_is_new) {
                    expr.intro.complex.choice.right = dy_core_expr_new(right);
                } else {
                    dy_core_expr_retain_ptr(ctx, expr.intro.complex.choice.right);
                }

                *result = expr;
                return true;
            }
            case DY_CORE_COMPLEX_RECURSION:
                if (dy_substitute_recursion(ctx, expr.intro.complex.recursion, id, sub, &expr.intro.complex.recursion)) {
                    *result = expr;
                    return true;
                } else {
                    return false;
                }
            }

            dy_bail("Impossible");
        case DY_CORE_INTRO_SIMPLE:
            if (dy_substitute_simple(ctx, expr.intro.simple, id, sub, &expr.intro.simple)) {
                *result = expr;
                return true;
            } else {
                return false;
            }
        }

        dy_bail("impossible");
    case DY_CORE_EXPR_VARIABLE:
        if (expr.variable_id == id) {
            *result = dy_core_expr_retain(ctx, sub);
            return true;
        }

        for (size_t i = 0, size = ctx->equal_variables.num_elems; i < size; ++i) {
            const struct dy_equal_variables *v = dy_array_pos(&ctx->equal_variables, i);

            if (v->id1 == expr.variable_id) {
                expr.variable_id = v->id2;
                *result = expr;
                return true;
            }
        }

        return false;
    case DY_CORE_EXPR_ELIM: {
        struct dy_core_expr new_expr;
        bool expr_is_new = dy_substitute(ctx, *expr.elim.expr, id, sub, &new_expr);

        struct dy_core_simple new_simple;
        bool simple_is_new = dy_substitute_simple(ctx, expr.elim.simple, id, sub, &new_simple);

        if (!expr_is_new && !simple_is_new) {
            return false;
        }

        if (expr_is_new) {
            expr.elim.expr = dy_core_expr_new(new_expr);
        } else {
            dy_core_expr_retain_ptr(ctx, expr.elim.expr);
        }

        if (simple_is_new) {
            expr.elim.simple = new_simple;
        } else {
            dy_core_simple_retain(ctx, expr.elim.simple);
        }

        *result = expr;
        return true;
    }
    case DY_CORE_EXPR_MAP:
        switch (expr.map.tag) {
        case DY_CORE_MAP_ASSUMPTION:
            if (dy_substitute_map_assumption(ctx, expr.map.assumption, id, sub, &expr.map.assumption)) {
                *result = expr;
                return true;
            } else {
                return false;
            }
        case DY_CORE_MAP_CHOICE:
            if (dy_substitute_map_choice(ctx, expr.map.choice, id, sub, &expr.map.choice)) {
                *result = expr;
                return true;
            } else {
                return false;
            }
        case DY_CORE_MAP_RECURSION:
            if (dy_substitute_map_recursion(ctx, expr.map.recursion, id, sub, &expr.map.recursion)) {
                *result = expr;
                return true;
            } else {
                return false;
            }
        }

        dy_bail("impossible");
    case DY_CORE_EXPR_ANY:
    case DY_CORE_EXPR_VOID:
        return false;
    case DY_CORE_EXPR_INFERENCE_CTX: {
        if (id == expr.inference_ctx.id) {
            return false;
        }

        struct dy_core_expr new_expr;
        if (!dy_substitute(ctx, *expr.inference_ctx.expr, id, sub, &new_expr)) {
            return false;
        }

        expr.inference_ctx.expr = dy_core_expr_new(new_expr);

        *result = expr;

        return true;
    }
    case DY_CORE_EXPR_INFERENCE_VAR:
        if (expr.inference_var_id == id) {
            *result = dy_core_expr_retain(ctx, sub);
            return true;
        }

        for (size_t i = 0, size = ctx->equal_variables.num_elems; i < size; ++i) {
            const struct dy_equal_variables *v = dy_array_pos(&ctx->equal_variables, i);

            if (v->id1 == expr.inference_var_id) {
                expr.inference_var_id = v->id2;
                *result = expr;
                return true;
            }
        }

        return false;
    case DY_CORE_EXPR_CUSTOM: {
        const struct dy_core_custom_shared *s = dy_array_pos(&ctx->custom_shared, expr.custom.id);
        return s->substitute(ctx, expr.custom.data, id, sub, result);
    }
    }

    dy_bail("Impossible object type.");
}

bool dy_substitute_assumption(struct dy_core_ctx *ctx, struct dy_core_assumption function, size_t id, struct dy_core_expr sub, struct dy_core_assumption *result)
{
    struct dy_core_expr type;
    bool type_is_new = dy_substitute(ctx, *function.type, id, sub, &type);

    if (id != function.id) {
        if (dy_core_expr_contains_this_variable(ctx, function.id, sub)) {
            size_t new_id = ctx->running_id++;

            dy_array_add(&ctx->equal_variables, &(struct dy_equal_variables){
                .id1 = function.id,
                .id2 = new_id
            });

            struct dy_core_expr expr;
            bool expr_is_new = dy_substitute(ctx, *function.expr, id, sub, &expr);

            --ctx->equal_variables.num_elems;

            if (!type_is_new && !expr_is_new) {
                return false;
            }

            if (type_is_new) {
                function.type = dy_core_expr_new(type);
            } else {
                dy_core_expr_retain_ptr(ctx, function.type);
            }

            if (expr_is_new) {
                function.expr = dy_core_expr_new(expr);
            } else {
                dy_core_expr_retain_ptr(ctx, function.expr);
            }

            function.id = new_id;

            *result = function;
            return true;
        } else {
            struct dy_core_expr expr;
            bool expr_is_new = dy_substitute(ctx, *function.expr, id, sub, &expr);

            if (!type_is_new && !expr_is_new) {
                return false;
            }

            if (type_is_new) {
                function.type = dy_core_expr_new(type);
            } else {
                dy_core_expr_retain_ptr(ctx, function.type);
            }

            if (expr_is_new) {
                function.expr = dy_core_expr_new(expr);
            } else {
                dy_core_expr_retain_ptr(ctx, function.expr);
            }

            *result = function;
            return true;
        }
    } else {
        if (!type_is_new) {
            return false;
        }

        if (type_is_new) {
            function.type = dy_core_expr_new(type);
        } else {
            dy_core_expr_retain_ptr(ctx, function.type);
        }

        function.expr = dy_core_expr_retain_ptr(ctx, function.expr);

        *result = function;
        return true;
    }
}

bool dy_substitute_recursion(struct dy_core_ctx *ctx, struct dy_core_recursion recursion, size_t id, struct dy_core_expr sub, struct dy_core_recursion *result)
{
    if (id == recursion.id) {
        return false;
    }

    if (dy_core_expr_contains_this_variable(ctx, recursion.id, sub)) {
        size_t new_id = ctx->running_id++;

        dy_array_add(&ctx->equal_variables, &(struct dy_equal_variables){
            .id1 = recursion.id,
            .id2 = new_id
        });

        struct dy_core_expr expr;
        bool expr_is_new = dy_substitute(ctx, *recursion.expr, id, sub, &expr);

        --ctx->equal_variables.num_elems;

        if (!expr_is_new) {
            return false;
        }

        recursion.expr = dy_core_expr_new(expr);
        recursion.id = new_id;

        *result = recursion;
        return true;
    } else {
        struct dy_core_expr expr;
        bool expr_is_new = dy_substitute(ctx, *recursion.expr, id, sub, &expr);

        if (!expr_is_new) {
            return false;
        }

        recursion.expr = dy_core_expr_new(expr);

        *result = recursion;
        return true;
    }
}

bool dy_substitute_simple(struct dy_core_ctx *ctx, struct dy_core_simple simple, size_t id, struct dy_core_expr sub, struct dy_core_simple *result)
{
    struct dy_core_expr out;
    bool out_is_new = dy_substitute(ctx, *simple.out, id, sub, &out);

    if (simple.tag == DY_CORE_SIMPLE_PROOF) {
        struct dy_core_expr expr;
        bool expr_is_new = dy_substitute(ctx, *simple.proof, id, sub, &expr);

        if (!expr_is_new && !out_is_new) {
            return false;
        }

        if (expr_is_new) {
            simple.proof = dy_core_expr_new(expr);
        } else {
            dy_core_expr_retain_ptr(ctx, simple.proof);
        }

        if (out_is_new) {
            simple.out = dy_core_expr_new(out);
        } else {
            dy_core_expr_retain_ptr(ctx, simple.out);
        }

        *result = simple;
        return true;
    } else {
        if (out_is_new) {
            simple.out = dy_core_expr_new(out);
            *result = simple;
            return true;
        } else {
            return false;
        }
    }
}

bool dy_substitute_map_assumption(struct dy_core_ctx *ctx, struct dy_core_map_assumption ass, size_t id, struct dy_core_expr sub, struct dy_core_map_assumption *result)
{
    struct dy_core_expr type;
    bool type_is_new = dy_substitute(ctx, *ass.type, id, sub, &type);

    if (id != ass.id) {
        if (dy_core_expr_contains_this_variable(ctx, ass.id, sub)) {
            size_t new_id = ctx->running_id++;

            dy_array_add(&ctx->equal_variables, &(struct dy_equal_variables){
                .id1 = ass.id,
                .id2 = new_id
            });

            struct dy_core_assumption new_ass;
            bool ass_is_new = dy_substitute_assumption(ctx, ass.assumption, id, sub, &new_ass);

            --ctx->equal_variables.num_elems;

            if (!type_is_new && !ass_is_new) {
                return false;
            }

            if (type_is_new) {
                ass.type = dy_core_expr_new(type);
            } else {
                dy_core_expr_retain_ptr(ctx, ass.type);
            }

            if (ass_is_new) {
                ass.assumption = new_ass;
            } else {
                dy_core_assumption_retain(ctx, ass.assumption);
            }

            ass.id = new_id;

            *result = ass;
            return true;
        } else {
            struct dy_core_assumption new_ass;
            bool ass_is_new = dy_substitute_assumption(ctx, ass.assumption, id, sub, &new_ass);

            if (!type_is_new && !ass_is_new) {
                return false;
            }

            if (type_is_new) {
                ass.type = dy_core_expr_new(type);
            } else {
                dy_core_expr_retain_ptr(ctx, ass.type);
            }

            if (ass_is_new) {
                ass.assumption = new_ass;
            } else {
                dy_core_assumption_retain(ctx, ass.assumption);
            }

            *result = ass;
            return true;
        }
    } else {
        if (!type_is_new) {
            return false;
        }

        if (type_is_new) {
            ass.type = dy_core_expr_new(type);
        } else {
            dy_core_expr_retain_ptr(ctx, ass.type);
        }

        ass.assumption = dy_core_assumption_retain(ctx, ass.assumption);

        *result = ass;
        return true;
    }
}

bool dy_substitute_map_choice(struct dy_core_ctx *ctx, struct dy_core_map_choice choice, size_t id, struct dy_core_expr sub, struct dy_core_map_choice *result)
{
    struct dy_core_assumption new_ass_left;
    bool ass_left_is_new = dy_substitute_assumption(ctx, choice.assumption_left, id, sub, &new_ass_left);

    struct dy_core_assumption new_ass_right;
    bool ass_right_is_new = dy_substitute_assumption(ctx, choice.assumption_right, id, sub, &new_ass_right);

    if (!ass_left_is_new && !ass_right_is_new) {
        return false;
    }

    if (ass_left_is_new) {
        choice.assumption_left = new_ass_left;
    } else {
        dy_core_assumption_retain(ctx, choice.assumption_left);
    }

    if (ass_right_is_new) {
        choice.assumption_right = new_ass_right;
    } else {
        dy_core_assumption_retain(ctx, choice.assumption_right);
    }

    *result = choice;
    return true;
}

bool dy_substitute_map_recursion(struct dy_core_ctx *ctx, struct dy_core_map_recursion rec, size_t id, struct dy_core_expr sub, struct dy_core_map_recursion *result)
{
    if (id == rec.id) {
        return false;
    }

    if (dy_core_expr_contains_this_variable(ctx, rec.id, sub)) {
        size_t new_id = ctx->running_id++;

        dy_array_add(&ctx->equal_variables, &(struct dy_equal_variables){
            .id1 = rec.id,
            .id2 = new_id
        });

        struct dy_core_assumption new_ass;
        bool ass_is_new = dy_substitute_assumption(ctx, rec.assumption, id, sub, &new_ass);

        --ctx->equal_variables.num_elems;

        if (!ass_is_new) {
            return false;
        }

        rec.assumption = new_ass;
        rec.id = new_id;

        *result = rec;
        return true;
    } else {
        struct dy_core_assumption new_ass;
        bool ass_is_new = dy_substitute_assumption(ctx, rec.assumption, id, sub, &new_ass);

        if (!ass_is_new) {
            return false;
        }

        rec.assumption = new_ass;

        *result = rec;
        return true;
    }
}
