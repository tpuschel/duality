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

static inline bool dy_substitute_function(struct dy_core_ctx *ctx, struct dy_core_function function, size_t id, struct dy_core_expr sub, struct dy_core_function *result);

static inline bool dy_substitute_recursion(struct dy_core_ctx *ctx, struct dy_core_recursion recursion, size_t id, struct dy_core_expr sub, struct dy_core_recursion *result);

static inline bool dy_substitute_solution(struct dy_core_ctx *ctx, struct dy_core_solution solution, size_t id, struct dy_core_expr sub, struct dy_core_solution *result);

bool dy_substitute(struct dy_core_ctx *ctx, struct dy_core_expr expr, size_t id, struct dy_core_expr sub, struct dy_core_expr *result)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_PROBLEM:
        switch (expr.problem.tag) {
        case DY_CORE_FUNCTION:
            if (dy_substitute_function(ctx, expr.problem.function, id, sub, &expr.problem.function)) {
                *result = expr;
                return true;
            } else {
                return false;
            }
        case DY_CORE_PAIR: {
            struct dy_core_expr left;
            bool left_is_new = dy_substitute(ctx, *expr.problem.pair.left, id, sub, &left);

            struct dy_core_expr right;
            bool right_is_new = dy_substitute(ctx, *expr.problem.pair.right, id, sub, &right);

            if (!left_is_new && !right_is_new) {
                return false;
            }

            if (left_is_new) {
                expr.problem.pair.left = dy_core_expr_new(left);
            } else {
                dy_core_expr_retain_ptr(ctx, expr.problem.pair.left);
            }

            if (right_is_new) {
                expr.problem.pair.right = dy_core_expr_new(right);
            } else {
                dy_core_expr_retain_ptr(ctx, expr.problem.pair.right);
            }

            *result = expr;
            return true;
        }
        case DY_CORE_RECURSION:
            if (dy_substitute_recursion(ctx, expr.problem.recursion, id, sub, &expr.problem.recursion)) {
                *result = expr;
                return true;
            } else {
                return false;
            }
        }

        dy_bail("Impossible");
    case DY_CORE_EXPR_SOLUTION:
        if (dy_substitute_solution(ctx, expr.solution, id, sub, &expr.solution)) {
            *result = expr;
            return true;
        } else {
            return false;
        }
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
    case DY_CORE_EXPR_APPLICATION: {
        struct dy_core_expr new_expr;
        bool expr_is_new = dy_substitute(ctx, *expr.application.expr, id, sub, &new_expr);

        struct dy_core_solution new_solution;
        bool solution_is_new = dy_substitute_solution(ctx, expr.application.solution, id, sub, &new_solution);

        if (!expr_is_new && !solution_is_new) {
            return false;
        }

        if (expr_is_new) {
            expr.application.expr = dy_core_expr_new(new_expr);
        } else {
            dy_core_expr_retain_ptr(ctx, expr.application.expr);
        }

        if (solution_is_new) {
            expr.application.solution = new_solution;
        } else {
            dy_core_solution_retain(ctx, expr.application.solution);
        }

        *result = expr;
        return true;
    }
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

bool dy_substitute_function(struct dy_core_ctx *ctx, struct dy_core_function function, size_t id, struct dy_core_expr sub, struct dy_core_function *result)
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

bool dy_substitute_solution(struct dy_core_ctx *ctx, struct dy_core_solution solution, size_t id, struct dy_core_expr sub, struct dy_core_solution *result)
{
    struct dy_core_expr out;
    bool out_is_new = dy_substitute(ctx, *solution.out, id, sub, &out);

    if (solution.tag == DY_CORE_FUNCTION) {
        struct dy_core_expr expr;
        bool expr_is_new = dy_substitute(ctx, *solution.expr, id, sub, &expr);

        if (!expr_is_new && !out_is_new) {
            return false;
        }

        if (expr_is_new) {
            solution.expr = dy_core_expr_new(expr);
        } else {
            dy_core_expr_retain_ptr(ctx, solution.expr);
        }

        if (out_is_new) {
            solution.out = dy_core_expr_new(out);
        } else {
            dy_core_expr_retain_ptr(ctx, solution.out);
        }

        *result = solution;
        return true;
    } else {
        if (out_is_new) {
            solution.out = dy_core_expr_new(out);
            *result = solution;
            return true;
        } else {
            return false;
        }
    }
}
