/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core.h"

#include "../support/util.h"

/**
 * This file implements substitution for every object of Core.
 */

static inline struct dy_core_expr substitute(struct dy_core_ctx *ctx, struct dy_core_expr expr, size_t id, struct dy_core_expr sub);

static inline struct dy_core_equality_map substitute_equality_map(struct dy_core_ctx *ctx, struct dy_core_equality_map equality_map, size_t id, struct dy_core_expr sub);

static inline struct dy_core_type_map substitute_type_map(struct dy_core_ctx *ctx, struct dy_core_type_map inference_type_map, size_t id, struct dy_core_expr sub);

static inline struct dy_core_inference_type_map substitute_inference_type_map(struct dy_core_ctx *ctx, struct dy_core_inference_type_map type_map, size_t id, struct dy_core_expr sub);

struct dy_core_expr substitute(struct dy_core_ctx *ctx, struct dy_core_expr expr, size_t id, struct dy_core_expr sub)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_CUSTOM:
        return expr.custom.substitute(expr.custom.data, ctx, id, sub);
    case DY_CORE_EXPR_END:
        // fallthrough
    case DY_CORE_EXPR_SYMBOL:
        return dy_core_expr_retain(expr);
    case DY_CORE_EXPR_EQUALITY_MAP:
        expr.equality_map = substitute_equality_map(ctx, expr.equality_map, id, sub);
        return expr;
    case DY_CORE_EXPR_TYPE_MAP:
        expr.type_map = substitute_type_map(ctx, expr.type_map, id, sub);
        return expr;
    case DY_CORE_EXPR_VARIABLE:
        if (expr.variable.id == id) {
            return dy_core_expr_retain(sub);
        } else {
            expr.variable.type = dy_core_expr_new(substitute(ctx, *expr.variable.type, id, sub));
            return expr;
        }
    case DY_CORE_EXPR_INFERENCE_VARIABLE:
        if (expr.variable.id == id) {
            return dy_core_expr_retain(sub);
        } else {
            expr.inference_variable.type = dy_core_expr_new(substitute(ctx, *expr.inference_variable.type, id, sub));
            return expr;
        }
    case DY_CORE_EXPR_JUNCTION:
        expr.junction.e1 = dy_core_expr_new(substitute(ctx, *expr.junction.e1, id, sub));
        expr.junction.e2 = dy_core_expr_new(substitute(ctx, *expr.junction.e2, id, sub));
        return expr;
    case DY_CORE_EXPR_ALTERNATIVE:
        expr.alternative.first = dy_core_expr_new(substitute(ctx, *expr.alternative.first, id, sub));
        expr.alternative.second = dy_core_expr_new(substitute(ctx, *expr.alternative.second, id, sub));
        return expr;
    case DY_CORE_EXPR_EQUALITY_MAP_ELIM:
        expr.equality_map_elim.expr = dy_core_expr_new(substitute(ctx, *expr.equality_map_elim.expr, id, sub));
        expr.equality_map_elim.map = substitute_equality_map(ctx, expr.equality_map_elim.map, id, sub);
        return expr;
    case DY_CORE_EXPR_TYPE_MAP_ELIM:
        expr.type_map_elim.expr = dy_core_expr_new(substitute(ctx, *expr.type_map_elim.expr, id, sub));
        expr.type_map_elim.map = substitute_type_map(ctx, expr.type_map_elim.map, id, sub);
        return expr;
    case DY_CORE_EXPR_INFERENCE_TYPE_MAP:
        expr.inference_type_map = substitute_inference_type_map(ctx, expr.inference_type_map, id, sub);
        return expr;
    case DY_CORE_EXPR_RECURSION:
        if (expr.recursion.id == id) {
            return dy_core_expr_retain(expr);
        } else {
            if (dy_core_expr_is_bound(expr.recursion.id, sub)) {
                dy_bail("Recursion binding appears in substitution!\n");
            }

            expr.recursion.expr = dy_core_expr_new(substitute(ctx, *expr.recursion.expr, id, sub));
            return expr;
        }
    }

    dy_bail("Impossible object type.");
}

static struct dy_core_equality_map substitute_equality_map(struct dy_core_ctx *ctx, struct dy_core_equality_map equality_map, size_t id, struct dy_core_expr sub)
{
    equality_map.e1 = dy_core_expr_new(substitute(ctx, *equality_map.e1, id, sub));
    equality_map.e2 = dy_core_expr_new(substitute(ctx, *equality_map.e2, id, sub));
    return equality_map;
}

static struct dy_core_type_map substitute_type_map(struct dy_core_ctx *ctx, struct dy_core_type_map type_map, size_t id, struct dy_core_expr sub)
{
    type_map.binding.type = dy_core_expr_new(substitute(ctx, *type_map.binding.type, id, sub));
    if (id != type_map.binding.id) {
        if (dy_core_expr_is_bound(type_map.binding.id, sub)) {
            size_t new_id = ctx->running_id++;

            struct dy_core_expr var_expr = {
                .tag = DY_CORE_EXPR_VARIABLE,
                .variable = {
                    .id = new_id,
                    .type = type_map.binding.type,
                }
            };

            struct dy_core_expr new_body = substitute(ctx, *type_map.expr, type_map.binding.id, var_expr);

            type_map.binding.id = new_id;
            type_map.expr = dy_core_expr_new(new_body);
        }

        type_map.expr = dy_core_expr_new(substitute(ctx, *type_map.expr, id, sub));
    } else {
        type_map.expr = dy_core_expr_retain_ptr(type_map.expr);
    }
    return type_map;
}

struct dy_core_inference_type_map substitute_inference_type_map(struct dy_core_ctx *ctx, struct dy_core_inference_type_map inference_type_map, size_t id, struct dy_core_expr sub)
{
    inference_type_map.binding.type = dy_core_expr_new(substitute(ctx, *inference_type_map.binding.type, id, sub));
    if (id != inference_type_map.binding.id) {
        if (dy_core_expr_is_bound(inference_type_map.binding.id, sub)) {
            size_t new_id = ctx->running_id++;

            struct dy_core_expr var_expr = {
                .tag = DY_CORE_EXPR_INFERENCE_VARIABLE,
                .inference_variable = {
                    .id = new_id,
                    .type = inference_type_map.binding.type,
                    .polarity = inference_type_map.polarity,
                }
            };

            struct dy_core_expr new_body = substitute(ctx, *inference_type_map.expr, inference_type_map.binding.id, var_expr);

            inference_type_map.binding.id = new_id;
            inference_type_map.expr = dy_core_expr_new(new_body);
        }

        inference_type_map.expr = dy_core_expr_new(substitute(ctx, *inference_type_map.expr, id, sub));
    } else {
        inference_type_map.expr = dy_core_expr_retain_ptr(inference_type_map.expr);
    }

    return inference_type_map;
}