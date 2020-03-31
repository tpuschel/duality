/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <duality/syntax/ast_to_core.h>

#include <duality/support/assert.h>

static bool ast_type_map_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_type_map type_map, enum dy_core_polarity polarity, struct dy_core_expr *expr);

static bool ast_expr_map_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_expr_map expr_map, enum dy_core_polarity polarity, struct dy_core_expr *expr);

static bool ast_list_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_list list, enum dy_core_polarity polarity, struct dy_core_expr *expr);

static bool do_block_equality_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_do_block_equality equality, struct dy_core_expr *expr);

static bool do_block_let_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_do_block_let let, struct dy_core_expr *expr);

static bool do_block_ignored_expr_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_do_block_ignored_expr ignored_expr, struct dy_core_expr *expr);

static struct dy_core_unknown create_inference_var(struct dy_ast_to_core_ctx *ctx);

bool dy_ast_expr_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_expr expr, struct dy_core_expr *core_expr)
{
    switch (expr.tag) {
    case DY_AST_EXPR_POSITIVE_TYPE_MAP:
        return dy_ast_positive_type_map_to_core(ctx, expr.positive_type_map, core_expr);
    case DY_AST_EXPR_NEGATIVE_TYPE_MAP:
        return dy_ast_negative_type_map_to_core(ctx, expr.negative_type_map, core_expr);
    case DY_AST_EXPR_LIST:
        return dy_ast_list_to_core(ctx, expr.list, core_expr);
    case DY_AST_EXPR_POSITIVE_EXPR_MAP:
        return dy_ast_positive_expr_map_to_core(ctx, expr.positive_expr_map, core_expr);
    case DY_AST_EXPR_NEGATIVE_EXPR_MAP:
        return dy_ast_negative_expr_map_to_core(ctx, expr.negative_expr_map, core_expr);
    case DY_AST_EXPR_DO_BLOCK:
        return dy_ast_do_block_to_core(ctx, expr.do_block, core_expr);
    case DY_AST_EXPR_CHOICE:
        return dy_ast_choice_to_core(ctx, expr.choice, core_expr);
    case DY_AST_EXPR_TRY_BLOCK:
        return dy_ast_try_block_to_core(ctx, expr.try_block, core_expr);
    case DY_AST_EXPR_EXPR_MAP_ELIM:
        return dy_ast_expr_map_elim_to_core(ctx, expr.expr_map_elim, core_expr);
    case DY_AST_EXPR_TYPE_MAP_ELIM:
        return dy_ast_type_map_elim_to_core(ctx, expr.type_map_elim, core_expr);
    case DY_AST_EXPR_VARIABLE:
        return dy_ast_variable_to_core(ctx, expr.variable, core_expr);
    case DY_AST_EXPR_JUXTAPOSITION:
        return dy_ast_juxtaposition_to_core(ctx, expr.juxtaposition, core_expr);
    case DY_AST_EXPR_STRING:
        *core_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_STRING,
            .string = expr.string
        };

        return true;
    case DY_AST_EXPR_TYPE_STRING:
        *core_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_TYPE_OF_STRINGS
        };

        return true;
    case DY_AST_EXPR_ALL:
        *core_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_END,
            .end_polarity = DY_CORE_POLARITY_POSITIVE
        };

        return true;
    case DY_AST_EXPR_NOTHING:
        *core_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_END,
            .end_polarity = DY_CORE_POLARITY_NEGATIVE
        };

        return true;
    }

    DY_IMPOSSIBLE_ENUM();
}

bool dy_ast_variable_to_core(struct dy_ast_to_core_ctx *ctx, dy_string_t variable, struct dy_core_expr *expr)
{
    for (size_t i = dy_array_size(ctx->bound_vars); i-- > 0;) {
        struct dy_ast_to_core_bound_var bound_var;
        dy_array_get(ctx->bound_vars, i, &bound_var);

        if (dy_string_are_equal(variable, bound_var.name)) {
            *expr = (struct dy_core_expr){
                .tag = DY_CORE_EXPR_UNKNOWN,
                .unknown = {
                    .id = bound_var.replacement_id,
                    .type = dy_core_expr_new(ctx->core_expr_pool, dy_core_expr_retain(ctx->core_expr_pool, bound_var.type)),
                    .is_inference_var = false,
                }
            };

            return true;
        }
    }

    if (dy_string_are_equal(variable, DY_STR_LIT("print"))) {
        *expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_PRINT
        };

        return true;
    }

    dy_array_add(ctx->unbound_vars, &variable);

    return false;
}

bool ast_type_map_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_type_map type_map, enum dy_core_polarity polarity, struct dy_core_expr *expr)
{
    if (type_map.arg.has_type) {
        struct dy_core_expr type;
        bool arg_type_conversion_succeeded = dy_ast_expr_to_core(ctx, *type_map.arg.type, &type);

        size_t id = (*ctx->running_id)++;

        size_t old_size;
        if (type_map.arg.has_name) {
            struct dy_ast_to_core_bound_var bound_var = {
                .name = type_map.arg.name,
                .replacement_id = id,
                .type = type
            };

            old_size = dy_array_add(ctx->bound_vars, &bound_var);
        } else {
            old_size = dy_array_size(ctx->bound_vars);
        }

        struct dy_core_expr e;
        bool expr_conversion_succeeded = dy_ast_expr_to_core(ctx, *type_map.expr, &e);

        dy_array_set_size(ctx->bound_vars, old_size);

        if (!arg_type_conversion_succeeded || !expr_conversion_succeeded) {
            if (arg_type_conversion_succeeded) {
                dy_core_expr_release(ctx->core_expr_pool, type);
            }

            if (expr_conversion_succeeded) {
                dy_core_expr_release(ctx->core_expr_pool, e);
            }

            return false;
        }

        *expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_TYPE_MAP,
            .type_map = {
                .arg_id = id,
                .arg_type = dy_core_expr_new(ctx->core_expr_pool, dy_core_expr_retain(ctx->core_expr_pool, type)),
                .expr = dy_core_expr_new(ctx->core_expr_pool, e),
                .polarity = polarity,
                .is_implicit = type_map.is_implicit,
            }
        };

        return true;
    } else {
        struct dy_core_unknown unknown = create_inference_var(ctx);

        struct dy_core_expr type = {
            .tag = DY_CORE_EXPR_UNKNOWN,
            .unknown = unknown
        };

        size_t id = (*ctx->running_id)++;

        size_t old_size;
        if (type_map.arg.has_name) {
            struct dy_ast_to_core_bound_var bound_var = {
                .name = type_map.arg.name,
                .replacement_id = id,
                .type = type
            };

            old_size = dy_array_add(ctx->bound_vars, &bound_var);
        } else {
            old_size = dy_array_size(ctx->bound_vars);
        }

        struct dy_core_expr e;
        bool expr_conversion_succeeded = dy_ast_expr_to_core(ctx, *type_map.expr, &e);

        dy_array_set_size(ctx->bound_vars, old_size);

        if (!expr_conversion_succeeded) {
            dy_core_expr_release_ptr(ctx->core_expr_pool, unknown.type);
            return false;
        }

        struct dy_core_expr result_type_map = {
            .tag = DY_CORE_EXPR_TYPE_MAP,
            .type_map = {
                .arg_id = id,
                .arg_type = dy_core_expr_new(ctx->core_expr_pool, dy_core_expr_retain(ctx->core_expr_pool, type)),
                .expr = dy_core_expr_new(ctx->core_expr_pool, e),
                .polarity = polarity,
                .is_implicit = type_map.is_implicit,
            }
        };

        *expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_INFERENCE_CTX,
            .inference_ctx = {
                .id = unknown.id,
                .type = dy_core_expr_retain_ptr(ctx->core_expr_pool, unknown.type),
                .expr = dy_core_expr_new(ctx->core_expr_pool, result_type_map),
                .polarity = DY_CORE_POLARITY_POSITIVE,
            }
        };

        return true;
    }
}

bool dy_ast_positive_type_map_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_type_map type_map, struct dy_core_expr *expr)
{
    return ast_type_map_to_core(ctx, type_map, DY_CORE_POLARITY_POSITIVE, expr);
}

bool dy_ast_negative_type_map_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_type_map type_map, struct dy_core_expr *expr)
{
    return ast_type_map_to_core(ctx, type_map, DY_CORE_POLARITY_NEGATIVE, expr);
}

bool ast_list_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_list list, enum dy_core_polarity polarity, struct dy_core_expr *expr)
{
    struct dy_core_expr e1;
    bool b1 = dy_ast_expr_to_core(ctx, *list.expr, &e1);

    if (!list.next_or_null) {
        if (b1) {
            *expr = e1;
        }

        return b1;
    }

    struct dy_core_expr e2;
    bool b2 = dy_ast_list_to_core(ctx, *list.next_or_null, &e2);

    if (!b1 || !b2) {
        if (b1) {
            dy_core_expr_release(ctx->core_expr_pool, e1);
        }

        if (b2) {
            dy_core_expr_release(ctx->core_expr_pool, e2);
        }

        return false;
    }

    *expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_BOTH,
        .both = {
            .e1 = dy_core_expr_new(ctx->core_expr_pool, e1),
            .e2 = dy_core_expr_new(ctx->core_expr_pool, e2),
            .polarity = polarity,
        }
    };

    return true;
}

bool dy_ast_list_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_list list, struct dy_core_expr *expr)
{
    return ast_list_to_core(ctx, list, DY_CORE_POLARITY_POSITIVE, expr);
}

bool dy_ast_choice_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_list choice, struct dy_core_expr *expr)
{
    return ast_list_to_core(ctx, choice, DY_CORE_POLARITY_NEGATIVE, expr);
}

bool dy_ast_try_block_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_list try_block, struct dy_core_expr *expr)
{
    struct dy_core_expr e1;
    bool b1 = dy_ast_expr_to_core(ctx, *try_block.expr, &e1);

    if (!try_block.next_or_null) {
        if (b1) {
            *expr = e1;
        }

        return b1;
    }

    struct dy_core_expr e2;
    bool b2 = dy_ast_try_block_to_core(ctx, *try_block.next_or_null, &e2);

    if (!b1 || !b2) {
        if (b1) {
            dy_core_expr_release(ctx->core_expr_pool, e1);
        }

        if (b2) {
            dy_core_expr_release(ctx->core_expr_pool, e2);
        }

        return false;
    }

    *expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_ONE_OF,
        .one_of = {
            .first = dy_core_expr_new(ctx->core_expr_pool, e1),
            .second = dy_core_expr_new(ctx->core_expr_pool, e2),
        }
    };

    return true;
}

bool ast_expr_map_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_expr_map expr_map, enum dy_core_polarity polarity, struct dy_core_expr *expr)
{
    struct dy_core_expr e1;
    bool b1 = dy_ast_expr_to_core(ctx, *expr_map.e1, &e1);

    struct dy_core_expr e2;
    bool b2 = dy_ast_expr_to_core(ctx, *expr_map.e2, &e2);

    if (!b1 || !b2) {
        if (b1) {
            dy_core_expr_release(ctx->core_expr_pool, e1);
        }

        if (b2) {
            dy_core_expr_release(ctx->core_expr_pool, e2);
        }

        return false;
    }

    *expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_EXPR_MAP,
        .expr_map = {
            .e1 = dy_core_expr_new(ctx->core_expr_pool, e1),
            .e2 = dy_core_expr_new(ctx->core_expr_pool, e2),
            .polarity = polarity,
            .is_implicit = expr_map.is_implicit,
        }
    };

    return true;
}

bool dy_ast_positive_expr_map_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_expr_map positive_expr_map, struct dy_core_expr *expr)
{
    return ast_expr_map_to_core(ctx, positive_expr_map, DY_CORE_POLARITY_POSITIVE, expr);
}

bool dy_ast_negative_expr_map_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_expr_map negative_expr_map, struct dy_core_expr *expr)
{
    return ast_expr_map_to_core(ctx, negative_expr_map, DY_CORE_POLARITY_NEGATIVE, expr);
}

bool dy_ast_do_block_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_do_block do_block, struct dy_core_expr *expr)
{
    switch (do_block.tag) {
    case DY_AST_DO_BLOCK_END_EXPR:
        return dy_ast_expr_to_core(ctx, *do_block.end_expr, expr);
    case DY_AST_DO_BLOCK_LET:
        return do_block_let_to_core(ctx, do_block.let, expr);
    case DY_AST_DO_BLOCK_IGNORED_EXPR:
        return do_block_ignored_expr_to_core(ctx, do_block.ignored_expr, expr);
    case DY_AST_DO_BLOCK_EQUALITY:
        return do_block_equality_to_core(ctx, do_block.equality, expr);
    }

    DY_IMPOSSIBLE_ENUM();
}

bool do_block_equality_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_do_block_equality equality, struct dy_core_expr *expr)
{
    struct dy_core_expr e1;
    bool e1_conversion_succeeded = dy_ast_expr_to_core(ctx, *equality.e1, &e1);

    struct dy_core_expr e2;
    bool e2_conversion_succeeded = dy_ast_expr_to_core(ctx, *equality.e2, &e2);

    struct dy_core_expr rest;
    bool rest_conversion_succeeded = dy_ast_do_block_to_core(ctx, *equality.rest, &rest);

    if (!e1_conversion_succeeded || !e2_conversion_succeeded || !rest_conversion_succeeded) {
        if (e1_conversion_succeeded) {
            dy_core_expr_release(ctx->core_expr_pool, e1);
        }

        if (e2_conversion_succeeded) {
            dy_core_expr_release(ctx->core_expr_pool, e2);
        }

        if (rest_conversion_succeeded) {
            dy_core_expr_release(ctx->core_expr_pool, rest);
        }

        return false;
    }

    struct dy_core_expr positive_expr_map = {
        .tag = DY_CORE_EXPR_EXPR_MAP,
        .expr_map = {
            .e1 = dy_core_expr_new(ctx->core_expr_pool, e1),
            .e2 = dy_core_expr_new(ctx->core_expr_pool, rest),
            .polarity = DY_CORE_POLARITY_POSITIVE,
            .is_implicit = false,
        }
    };

    struct dy_core_expr result_type = {
        .tag = DY_CORE_EXPR_UNKNOWN,
        .unknown = {
            .id = (*ctx->running_id)++,
            .type = dy_core_expr_new(ctx->core_expr_pool, (struct dy_core_expr){ .tag = DY_CORE_EXPR_END, .end_polarity = DY_CORE_POLARITY_POSITIVE }),
            .is_inference_var = true,
        }
    };

    struct dy_core_expr elim = {
        .tag = DY_CORE_EXPR_EXPR_MAP_ELIM,
        .expr_map_elim = {
            .expr = dy_core_expr_new(ctx->core_expr_pool, positive_expr_map),
            .expr_map = {
                .e1 = dy_core_expr_new(ctx->core_expr_pool, e2),
                .e2 = dy_core_expr_new(ctx->core_expr_pool, dy_core_expr_retain(ctx->core_expr_pool, result_type)),
                .polarity = DY_CORE_POLARITY_NEGATIVE,
                .is_implicit = false,
            },
        }
    };

    *expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_INFERENCE_CTX,
        .inference_ctx = {
            .id = result_type.unknown.id,
            .type = result_type.unknown.type,
            .expr = dy_core_expr_new(ctx->core_expr_pool, elim),
            .polarity = DY_CORE_POLARITY_NEGATIVE,
        }
    };

    return true;
}

bool do_block_let_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_do_block_let let, struct dy_core_expr *expr)
{
    struct dy_core_expr e;
    bool expr_conversion_succeeded = dy_ast_expr_to_core(ctx, *let.expr, &e);

    size_t id = (*ctx->running_id)++;

    struct dy_core_unknown unknown = create_inference_var(ctx);

    struct dy_core_expr arg_type = {
        .tag = DY_CORE_EXPR_UNKNOWN,
        .unknown = unknown
    };

    struct dy_ast_to_core_bound_var bound_var = {
        .name = let.arg_name,
        .replacement_id = id,
        .type = arg_type
    };

    size_t old_size = dy_array_add(ctx->bound_vars, &bound_var);

    struct dy_core_expr rest;
    bool rest_conversion_succeeded = dy_ast_do_block_to_core(ctx, *let.rest, &rest);

    dy_array_set_size(ctx->bound_vars, old_size);

    if (!expr_conversion_succeeded || !rest_conversion_succeeded) {
        dy_core_expr_release(ctx->core_expr_pool, arg_type);

        if (expr_conversion_succeeded) {
            dy_core_expr_release(ctx->core_expr_pool, e);
        }

        if (rest_conversion_succeeded) {
            dy_core_expr_release(ctx->core_expr_pool, rest);
        }

        return false;
    }

    struct dy_core_expr positive_type_map = {
        .tag = DY_CORE_EXPR_TYPE_MAP,
        .type_map = {
            .arg_id = id,
            .arg_type = dy_core_expr_new(ctx->core_expr_pool, dy_core_expr_retain(ctx->core_expr_pool, arg_type)),
            .expr = dy_core_expr_new(ctx->core_expr_pool, rest),
            .polarity = DY_CORE_POLARITY_POSITIVE,
            .is_implicit = false,
        }
    };

    struct dy_core_expr result_inference_ctx = {
        .tag = DY_CORE_EXPR_INFERENCE_CTX,
        .inference_ctx = {
            .id = unknown.id,
            .type = dy_core_expr_retain_ptr(ctx->core_expr_pool, unknown.type),
            .expr = dy_core_expr_new(ctx->core_expr_pool, positive_type_map),
            .polarity = DY_CORE_POLARITY_POSITIVE,
        }
    };

    struct dy_core_expr result_type = {
        .tag = DY_CORE_EXPR_UNKNOWN,
        .unknown = {
            .id = (*ctx->running_id)++,
            .type = dy_core_expr_new(ctx->core_expr_pool, (struct dy_core_expr){ .tag = DY_CORE_EXPR_END, .end_polarity = DY_CORE_POLARITY_POSITIVE }),
            .is_inference_var = true,
        }
    };

    struct dy_core_expr elim = {
        .tag = DY_CORE_EXPR_EXPR_MAP_ELIM,
        .expr_map_elim = {
            .expr = dy_core_expr_new(ctx->core_expr_pool, result_inference_ctx),
            .expr_map = {
                .e1 = dy_core_expr_new(ctx->core_expr_pool, e),
                .e2 = dy_core_expr_new(ctx->core_expr_pool, dy_core_expr_retain(ctx->core_expr_pool, result_type)),
                .polarity = DY_CORE_POLARITY_NEGATIVE,
                .is_implicit = false,
            },
        }
    };

    *expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_INFERENCE_CTX,
        .inference_ctx = {
            .id = result_type.unknown.id,
            .type = result_type.unknown.type,
            .expr = dy_core_expr_new(ctx->core_expr_pool, elim),
            .polarity = DY_CORE_POLARITY_NEGATIVE,
        }
    };

    return true;
}

bool do_block_ignored_expr_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_do_block_ignored_expr ignored_expr, struct dy_core_expr *expr)
{
    struct dy_core_expr e;
    bool expr_conversion_succeeded = dy_ast_expr_to_core(ctx, *ignored_expr.expr, &e);

    size_t id = (*ctx->running_id)++;

    struct dy_core_expr rest;
    bool rest_conversion_succeeded = dy_ast_do_block_to_core(ctx, *ignored_expr.rest, &rest);

    if (!expr_conversion_succeeded || !rest_conversion_succeeded) {
        if (expr_conversion_succeeded) {
            dy_core_expr_release(ctx->core_expr_pool, e);
        }

        if (rest_conversion_succeeded) {
            dy_core_expr_release(ctx->core_expr_pool, rest);
        }

        return false;
    }

    struct dy_core_unknown unknown = create_inference_var(ctx);

    struct dy_core_expr unknown_expr = {
        .tag = DY_CORE_EXPR_UNKNOWN,
        .unknown = unknown
    };

    struct dy_core_expr positive_type_map = {
        .tag = DY_CORE_EXPR_TYPE_MAP,
        .type_map = {
            .arg_id = id,
            .arg_type = dy_core_expr_new(ctx->core_expr_pool, unknown_expr),
            .expr = dy_core_expr_new(ctx->core_expr_pool, rest),
            .polarity = DY_CORE_POLARITY_POSITIVE,
            .is_implicit = false,
        }
    };

    struct dy_core_expr result_inference_ctx = {
        .tag = DY_CORE_EXPR_INFERENCE_CTX,
        .inference_ctx = {
            .id = unknown.id,
            .type = dy_core_expr_retain_ptr(ctx->core_expr_pool, unknown.type),
            .expr = dy_core_expr_new(ctx->core_expr_pool, positive_type_map),
            .polarity = DY_CORE_POLARITY_POSITIVE,
        }
    };

    struct dy_core_expr result_type = {
        .tag = DY_CORE_EXPR_UNKNOWN,
        .unknown = {
            .id = (*ctx->running_id)++,
            .type = dy_core_expr_new(ctx->core_expr_pool, (struct dy_core_expr){ .tag = DY_CORE_EXPR_END, .end_polarity = DY_CORE_POLARITY_POSITIVE }),
            .is_inference_var = true,
        }
    };

    struct dy_core_expr elim = {
        .tag = DY_CORE_EXPR_EXPR_MAP_ELIM,
        .expr_map_elim = {
            .expr = dy_core_expr_new(ctx->core_expr_pool, result_inference_ctx),
            .expr_map = {
                .e1 = dy_core_expr_new(ctx->core_expr_pool, e),
                .e2 = dy_core_expr_new(ctx->core_expr_pool, dy_core_expr_retain(ctx->core_expr_pool, result_type)),
                .polarity = DY_CORE_POLARITY_NEGATIVE,
                .is_implicit = false,
            },
        }
    };

    *expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_INFERENCE_CTX,
        .inference_ctx = {
            .id = result_type.unknown.id,
            .type = dy_core_expr_retain_ptr(ctx->core_expr_pool, result_type.unknown.type),
            .expr = dy_core_expr_new(ctx->core_expr_pool, elim),
            .polarity = DY_CORE_POLARITY_NEGATIVE,
        }
    };

    return true;
}

bool dy_ast_expr_map_elim_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_expr_map_elim elim, struct dy_core_expr *expr)
{
    struct dy_core_expr e1;
    bool b1 = dy_ast_expr_to_core(ctx, *elim.expr, &e1);

    struct dy_core_expr e2;
    bool b2 = dy_ast_negative_expr_map_to_core(ctx, elim.expr_map, &e2);

    if (!b1 || !b2) {
        if (b1) {
            dy_core_expr_release(ctx->core_expr_pool, e1);
        }

        if (b2) {
            dy_core_expr_release(ctx->core_expr_pool, e2);
        }

        return false;
    }

    dy_assert(e2.tag == DY_CORE_EXPR_EXPR_MAP);

    *expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_EXPR_MAP_ELIM,
        .expr_map_elim = {
            .expr = dy_core_expr_new(ctx->core_expr_pool, e1),
            .expr_map = e2.expr_map,
        }
    };

    return true;
}

bool dy_ast_type_map_elim_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_type_map_elim elim, struct dy_core_expr *expr)
{
    struct dy_core_expr e1;
    bool b1 = dy_ast_expr_to_core(ctx, *elim.expr, &e1);

    struct dy_core_expr e2;
    bool b2 = dy_ast_negative_type_map_to_core(ctx, elim.type_map, &e2);

    if (!b1 || !b2) {
        if (b1) {
            dy_core_expr_release(ctx->core_expr_pool, e1);
        }

        if (b2) {
            dy_core_expr_release(ctx->core_expr_pool, e2);
        }

        return false;
    }

    dy_assert(e2.tag == DY_CORE_EXPR_TYPE_MAP);

    *expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_TYPE_MAP_ELIM,
        .type_map_elim = {
            .expr = dy_core_expr_new(ctx->core_expr_pool, e1),
            .type_map = e2.type_map,
        }
    };

    return true;
}

bool dy_ast_juxtaposition_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_juxtaposition juxtaposition, struct dy_core_expr *expr)
{
    struct dy_core_expr left;
    bool b1 = dy_ast_expr_to_core(ctx, *juxtaposition.left, &left);

    struct dy_core_expr right;
    bool b2 = dy_ast_expr_to_core(ctx, *juxtaposition.right, &right);

    if (!b1 || !b2) {
        if (b1) {
            dy_core_expr_release(ctx->core_expr_pool, left);
        }

        if (b2) {
            dy_core_expr_release(ctx->core_expr_pool, right);
        }

        return false;
    }

    struct dy_core_unknown unknown = create_inference_var(ctx);

    struct dy_core_expr unknown_expr = {
        .tag = DY_CORE_EXPR_UNKNOWN,
        .unknown = unknown
    };

    struct dy_core_expr elim = {
        .tag = DY_CORE_EXPR_EXPR_MAP_ELIM,
        .expr_map_elim = {
            .expr = dy_core_expr_new(ctx->core_expr_pool, left),
            .expr_map = {
                .e1 = dy_core_expr_new(ctx->core_expr_pool, right),
                .e2 = dy_core_expr_new(ctx->core_expr_pool, unknown_expr),
                .polarity = DY_CORE_POLARITY_NEGATIVE,
                .is_implicit = false,
            },
        }
    };

    *expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_INFERENCE_CTX,
        .inference_ctx = {
            .id = unknown.id,
            .type = dy_core_expr_retain_ptr(ctx->core_expr_pool, unknown.type),
            .expr = dy_core_expr_new(ctx->core_expr_pool, elim),
            .polarity = DY_CORE_POLARITY_NEGATIVE,
        }
    };

    return true;
}

struct dy_core_unknown create_inference_var(struct dy_ast_to_core_ctx *ctx)
{
    struct dy_core_expr type_all = {
        .tag = DY_CORE_EXPR_END,
        .end_polarity = DY_CORE_POLARITY_POSITIVE
    };

    return (struct dy_core_unknown){
        .id = (*ctx->running_id)++,
        .type = dy_core_expr_new(ctx->core_expr_pool, type_all),
        .is_inference_var = true
    };
}
