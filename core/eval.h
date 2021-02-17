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

static inline bool dy_eval_application(struct dy_core_ctx *ctx, struct dy_core_application app, bool *is_value, struct dy_core_expr *result);

static inline bool dy_eval_solution(struct dy_core_ctx *ctx, struct dy_core_solution solution, bool *is_value, struct dy_core_solution *result);

static inline bool dy_eval_app_single_step(struct dy_core_ctx *ctx, struct dy_core_application app, struct dy_core_expr *result);

bool dy_eval_expr(struct dy_core_ctx *ctx, struct dy_core_expr expr, bool *is_value, struct dy_core_expr *result)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_PROBLEM:
        switch (expr.problem.tag) {
        case DY_CORE_FUNCTION: {
            struct dy_core_expr new_type;
            if (!dy_eval_expr(ctx, *expr.problem.function.type, is_value, &new_type)) {
                return false;
            }

            expr.problem.function.type = dy_core_expr_new(new_type);
            dy_core_expr_retain_ptr(ctx, expr.problem.function.expr);
            *result = expr;
            return true;
        }
        case DY_CORE_PAIR:
            *is_value = true;
            return false;
        case DY_CORE_RECURSION:
            *is_value = true;
            return false;
        }

        dy_bail("impossible");
    case DY_CORE_EXPR_SOLUTION:
        return dy_eval_solution(ctx, expr.solution, is_value, &expr.solution);
    case DY_CORE_EXPR_APPLICATION:
        return dy_eval_application(ctx, expr.application, is_value, result);
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

bool dy_eval_application(struct dy_core_ctx *ctx, struct dy_core_application app, bool *is_value, struct dy_core_expr *result)
{
    bool expr_is_value;
    struct dy_core_expr new_expr;
    bool expr_is_new = dy_eval_expr(ctx, *app.expr, &expr_is_value, &new_expr);

    bool solution_is_value;
    struct dy_core_solution new_solution;
    bool solution_is_new = dy_eval_solution(ctx, app.solution, &solution_is_value, &new_solution);

    if (!expr_is_value || !solution_is_value) {
        *is_value = false;

        if (!expr_is_new && !solution_is_new) {
            return false;
        }

        if (expr_is_new) {
            app.expr = dy_core_expr_new(new_expr);
        } else {
            dy_core_expr_retain_ptr(ctx, app.expr);
        }

        if (solution_is_new) {
            app.solution = new_solution;
        } else {
            dy_core_solution_retain(ctx, app.solution);
        }

        *result = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_APPLICATION,
            .application = app
        };
        return true;
    }

    if (!expr_is_new) {
        new_expr = dy_core_expr_retain(ctx, *app.expr);
    }

    if (!solution_is_new) {
        new_solution = dy_core_solution_retain(ctx, app.solution);
    }

    bool did_transform = false;
    bool changed_check_result = false;
    if (app.check_result == DY_MAYBE) {
        struct dy_core_expr subtype = dy_type_of(ctx, new_expr);

        struct dy_core_expr supertype = {
            .tag = DY_CORE_EXPR_SOLUTION,
            .solution = new_solution
        };

        dy_ternary_t old_check_result = app.check_result;

        struct dy_core_expr new_new_expr;
        app.check_result = dy_is_subtype(ctx, subtype, supertype, new_expr, &new_new_expr, &did_transform);

        if (did_transform) {
            dy_core_expr_release(ctx, new_expr);

            if (dy_eval_expr(ctx, new_new_expr, &expr_is_value, &new_expr)) {
                dy_core_expr_release(ctx, new_new_expr);
                new_new_expr = new_expr;
            }

            new_expr = new_new_expr;
        }

        dy_core_expr_release(ctx, subtype);

        changed_check_result = old_check_result != app.check_result;
    }

    app.expr = dy_core_expr_new(new_expr);
    app.solution = new_solution;

    struct dy_core_expr app_expr = {
        .tag = DY_CORE_EXPR_APPLICATION,
        .application = app
    };

    struct dy_core_expr res;
    if (!dy_eval_app_single_step(ctx, app, &res)) {
        *is_value = false;

        if (!expr_is_new && !solution_is_new && !did_transform && !changed_check_result) {
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

bool dy_eval_solution(struct dy_core_ctx *ctx, struct dy_core_solution solution, bool *is_value, struct dy_core_solution *result)
{
    if (solution.tag == DY_CORE_FUNCTION) {
        struct dy_core_expr expr;
        if (dy_eval_expr(ctx, *solution.expr, is_value, &expr)) {
            solution.expr = dy_core_expr_new(expr);
            dy_core_expr_retain_ptr(ctx, solution.out);
            *result = solution;
            return true;
        } else {
            return false;
        }

    } else {
        *is_value = true;
        return false;
    }
}

bool dy_eval_app_single_step(struct dy_core_ctx *ctx, struct dy_core_application app, struct dy_core_expr *result)
{
    if (app.check_result != DY_YES) {
        return false;
    }

    if (app.expr->tag == DY_CORE_EXPR_PROBLEM) {
        switch (app.expr->problem.tag) {
        case DY_CORE_FUNCTION:
            if (!dy_substitute(ctx, *app.expr->problem.function.expr, app.expr->problem.function.id, *app.solution.expr, result)) {
                *result = dy_core_expr_retain(ctx, *app.expr->problem.function.expr);
            }

            return true;
        case DY_CORE_PAIR:
            if (app.solution.direction == DY_LEFT) {
                *result = dy_core_expr_retain(ctx, *app.expr->problem.pair.left);
            } else {
                *result = dy_core_expr_retain(ctx, *app.expr->problem.pair.right);
            }

            return true;
        case DY_CORE_RECURSION:
            if (!dy_substitute(ctx, *app.expr->problem.recursion.expr, app.expr->problem.recursion.id, *app.expr, result)) {
                *result = dy_core_expr_retain(ctx, *app.expr->problem.recursion.expr);
            }

            return true;
        }

        dy_bail("impossible");
    }

    if (app.expr->tag == DY_CORE_EXPR_SOLUTION) {
        *result = dy_core_expr_retain(ctx, *app.expr->solution.out);
        return true;
    }

    return false;
}
