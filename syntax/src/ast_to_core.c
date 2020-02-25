/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <duality/syntax/ast_to_core.h>

#include <duality/support/assert.h>

static struct dy_core_expr *alloc_expr(struct dy_ast_to_core_ctx *ctx, struct dy_core_expr expr);

static bool ast_type_map_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_type_map type_map, enum dy_core_polarity polarity, struct dy_core_expr *expr, dy_array_t *sub_maps);

static bool ast_value_map_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_value_map value_map, enum dy_core_polarity polarity, struct dy_core_expr *expr, dy_array_t *sub_maps);

static bool ast_list_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_list list, enum dy_core_polarity polarity, struct dy_core_expr *expr, dy_array_t *sub_maps);

static bool do_block_equality_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_do_block_equality equality, struct dy_core_expr *expr, dy_array_t *sub_maps);

static bool do_block_let_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_do_block_let let, struct dy_core_expr *expr, dy_array_t *sub_maps);

static bool do_block_ignored_expr_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_do_block_ignored_expr ignored_expr, struct dy_core_expr *expr, dy_array_t *sub_maps);

static struct dy_core_unknown create_inference_var(struct dy_ast_to_core_ctx *ctx);

bool dy_ast_expr_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_expr expr, struct dy_core_expr *core_expr, dy_array_t *sub_maps)
{
    switch (expr.tag) {
    case DY_AST_EXPR_POSITIVE_TYPE_MAP:
        return dy_ast_positive_type_map_to_core(ctx, expr.positive_type_map, core_expr, sub_maps);
    case DY_AST_EXPR_NEGATIVE_TYPE_MAP:
        return dy_ast_negative_type_map_to_core(ctx, expr.negative_type_map, core_expr, sub_maps);
    case DY_AST_EXPR_LIST:
        return dy_ast_list_to_core(ctx, expr.list, core_expr, sub_maps);
    case DY_AST_EXPR_POSITIVE_VALUE_MAP:
        return dy_ast_positive_value_map_to_core(ctx, expr.positive_value_map, core_expr, sub_maps);
    case DY_AST_EXPR_NEGATIVE_VALUE_MAP:
        return dy_ast_negative_value_map_to_core(ctx, expr.negative_value_map, core_expr, sub_maps);
    case DY_AST_EXPR_DO_BLOCK:
        return dy_ast_do_block_to_core(ctx, expr.do_block, core_expr, sub_maps);
    case DY_AST_EXPR_CHOICE:
        return dy_ast_choice_to_core(ctx, expr.choice, core_expr, sub_maps);
    case DY_AST_EXPR_TRY_BLOCK:
        return dy_ast_try_block_to_core(ctx, expr.try_block, core_expr, sub_maps);
    case DY_AST_EXPR_VALUE_MAP_ELIM:
        return dy_ast_value_map_elim_to_core(ctx, expr.value_map_elim, core_expr, sub_maps);
    case DY_AST_EXPR_TYPE_MAP_ELIM:
        return dy_ast_type_map_elim_to_core(ctx, expr.type_map_elim, core_expr, sub_maps);
    case DY_AST_EXPR_VARIABLE:
        return dy_ast_variable_to_core(ctx, expr.variable, core_expr);
    case DY_AST_EXPR_JUXTAPOSITION:
        return dy_ast_juxtaposition_to_core(ctx, expr.juxtaposition, core_expr, sub_maps);
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
                    .type = bound_var.type,
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

bool ast_type_map_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_type_map type_map, enum dy_core_polarity polarity, struct dy_core_expr *expr, dy_array_t *sub_maps)
{
    if (type_map.arg.has_type) {
        dy_array_t *sub_maps1 = dy_array_create(dy_allocator_stdlib(), sizeof(struct dy_text_range_core_map), 2);

        struct dy_core_expr *type = ctx->allocator.alloc(sizeof *type, ctx->allocator.env);
        bool arg_type_conversion_succeeded = dy_ast_expr_to_core(ctx, *type_map.arg.type, type, sub_maps1);

        struct dy_text_range_core_map map1 = {
            .text_range = type_map.arg.type->text_range,
            .expr = *type,
            .sub_maps = sub_maps1
        };

        dy_array_add(sub_maps, &map1);

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
        dy_array_t *sub_maps2 = dy_array_create(dy_allocator_stdlib(), sizeof(struct dy_text_range_core_map), 2);
        bool expr_conversion_succeeded = dy_ast_expr_to_core(ctx, *type_map.expr, &e, sub_maps2);

        struct dy_text_range_core_map map2 = {
            .text_range = type_map.expr->text_range,
            .expr = e,
            .sub_maps = sub_maps2
        };

        dy_array_set_size(ctx->bound_vars, old_size);

        if (!arg_type_conversion_succeeded || !expr_conversion_succeeded) {
            return false;
        }

        dy_array_add(sub_maps, &map2);

        *expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_TYPE_MAP,
            .type_map = {
                .arg_id = id,
                .arg_type = type,
                .expr = alloc_expr(ctx, e),
                .polarity = polarity,
                .is_implicit = type_map.is_implicit,
            }
        };

        return true;
    } else {
        struct dy_core_unknown unknown = create_inference_var(ctx);

        struct dy_core_expr *type = ctx->allocator.alloc(sizeof *type, ctx->allocator.env);
        *type = (struct dy_core_expr){
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
        dy_array_t *sub_maps2 = dy_array_create(dy_allocator_stdlib(), sizeof(struct dy_text_range_core_map), 2);
        bool expr_conversion_succeeded = dy_ast_expr_to_core(ctx, *type_map.expr, &e, sub_maps2);

        struct dy_text_range_core_map map2 = {
            .text_range = type_map.expr->text_range,
            .expr = e,
            .sub_maps = sub_maps2
        };

        dy_array_set_size(ctx->bound_vars, old_size);

        if (!expr_conversion_succeeded) {
            return false;
        }

        dy_array_add(sub_maps, &map2);

        struct dy_core_expr result_type_map = {
            .tag = DY_CORE_EXPR_TYPE_MAP,
            .type_map = {
                .arg_id = id,
                .arg_type = type,
                .expr = alloc_expr(ctx, e),
                .polarity = polarity,
                .is_implicit = type_map.is_implicit,
            }
        };

        *expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_TYPE_MAP,
            .type_map = {
                .arg_id = unknown.id,
                .arg_type = unknown.type,
                .expr = alloc_expr(ctx, result_type_map),
                .polarity = DY_CORE_POLARITY_POSITIVE,
                .is_implicit = true,
            }
        };

        return true;
    }
}

bool dy_ast_positive_type_map_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_type_map type_map, struct dy_core_expr *expr, dy_array_t *sub_maps)
{
    return ast_type_map_to_core(ctx, type_map, DY_CORE_POLARITY_POSITIVE, expr, sub_maps);
}

bool dy_ast_negative_type_map_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_type_map type_map, struct dy_core_expr *expr, dy_array_t *sub_maps)
{
    return ast_type_map_to_core(ctx, type_map, DY_CORE_POLARITY_NEGATIVE, expr, sub_maps);
}

bool ast_list_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_list list, enum dy_core_polarity polarity, struct dy_core_expr *expr, dy_array_t *sub_maps)
{
    dy_assert(list.num_exprs != 0);

    struct dy_core_expr e1;
    dy_array_t *sub_maps1 = dy_array_create(dy_allocator_stdlib(), sizeof(struct dy_text_range_core_map), 2);
    bool b1 = dy_ast_expr_to_core(ctx, list.exprs[0], &e1, sub_maps1);

    struct dy_text_range_core_map map1 = {
        .text_range = list.exprs[0].text_range,
        .expr = e1,
        .sub_maps = sub_maps1
    };

    dy_array_add(sub_maps, &map1);

    if (list.num_exprs == 1) {
        if (b1) {
            *expr = e1;

            return true;
        } else {
            return false;
        }
    }

    struct dy_ast_list new_list = {
        .exprs = list.exprs + 1,
        .num_exprs = list.num_exprs - 1
    };

    struct dy_core_expr e2;
    bool b2 = dy_ast_list_to_core(ctx, new_list, &e2, sub_maps);

    if (!b1 || !b2) {
        return false;
    }

    *expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_BOTH,
        .both = {
            .e1 = alloc_expr(ctx, e1),
            .e2 = alloc_expr(ctx, e2),
            .polarity = polarity,
        }
    };

    return true;
}

bool dy_ast_list_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_list list, struct dy_core_expr *expr, dy_array_t *sub_maps)
{
    return ast_list_to_core(ctx, list, DY_CORE_POLARITY_POSITIVE, expr, sub_maps);
}

bool dy_ast_choice_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_list choice, struct dy_core_expr *expr, dy_array_t *sub_maps)
{
    return ast_list_to_core(ctx, choice, DY_CORE_POLARITY_NEGATIVE, expr, sub_maps);
}

bool dy_ast_try_block_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_list try_block, struct dy_core_expr *expr, dy_array_t *sub_maps)
{
    dy_assert(try_block.num_exprs != 0);

    struct dy_core_expr e1;
    dy_array_t *sub_maps1 = dy_array_create(dy_allocator_stdlib(), sizeof(struct dy_text_range_core_map), 2);
    bool b1 = dy_ast_expr_to_core(ctx, try_block.exprs[0], &e1, sub_maps1);

    struct dy_text_range_core_map map1 = {
        .text_range = try_block.exprs[0].text_range,
        .expr = e1,
        .sub_maps = sub_maps1
    };

    dy_array_add(sub_maps, &map1);

    if (try_block.num_exprs == 1) {
        if (b1) {
            *expr = e1;
            return true;
        } else {
            return false;
        }
    }

    struct dy_ast_list new_try_block = {
        .exprs = try_block.exprs + 1,
        .num_exprs = try_block.num_exprs - 1
    };

    struct dy_core_expr e2;
    bool b2 = dy_ast_try_block_to_core(ctx, new_try_block, &e2, sub_maps);

    if (!b1 || !b2) {
        return false;
    }

    *expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_ONE_OF,
        .one_of = {
            .first = alloc_expr(ctx, e1),
            .second = alloc_expr(ctx, e2),
        }
    };

    return true;
}

bool ast_value_map_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_value_map value_map, enum dy_core_polarity polarity, struct dy_core_expr *expr, dy_array_t *sub_maps)
{
    struct dy_core_expr e1;
    dy_array_t *sub_maps1 = dy_array_create(dy_allocator_stdlib(), sizeof(struct dy_text_range_core_map), 2);
    bool b1 = dy_ast_expr_to_core(ctx, *value_map.e1, &e1, sub_maps1);

    struct dy_text_range_core_map map1 = {
        .text_range = value_map.e1->text_range,
        .expr = e1,
        .sub_maps = sub_maps1
    };

    struct dy_core_expr e2;
    dy_array_t *sub_maps2 = dy_array_create(dy_allocator_stdlib(), sizeof(struct dy_text_range_core_map), 2);
    bool b2 = dy_ast_expr_to_core(ctx, *value_map.e2, &e2, sub_maps2);

    struct dy_text_range_core_map map2 = {
        .text_range = value_map.e2->text_range,
        .expr = e2,
        .sub_maps = sub_maps2
    };

    if (!b1 || !b2) {
        return false;
    }

    dy_array_add(sub_maps, &map1);
    dy_array_add(sub_maps, &map2);

    *expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_VALUE_MAP,
        .value_map = {
            .e1 = alloc_expr(ctx, e1),
            .e2 = alloc_expr(ctx, e2),
            .polarity = polarity,
            .is_implicit = value_map.is_implicit,
        }
    };

    return true;
}

bool dy_ast_positive_value_map_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_value_map positive_value_map, struct dy_core_expr *expr, dy_array_t *sub_maps)
{
    return ast_value_map_to_core(ctx, positive_value_map, DY_CORE_POLARITY_POSITIVE, expr, sub_maps);
}

bool dy_ast_negative_value_map_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_value_map negative_value_map, struct dy_core_expr *expr, dy_array_t *sub_maps)
{
    return ast_value_map_to_core(ctx, negative_value_map, DY_CORE_POLARITY_NEGATIVE, expr, sub_maps);
}

bool dy_ast_do_block_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_do_block do_block, struct dy_core_expr *expr, dy_array_t *sub_maps)
{
    switch (do_block.tag) {
    case DY_AST_DO_BLOCK_END_EXPR: {
        dy_array_t *sub_maps1 = dy_array_create(dy_allocator_stdlib(), sizeof(struct dy_text_range_core_map), 2);

        if (dy_ast_expr_to_core(ctx, *do_block.end_expr, expr, sub_maps1)) {
            struct dy_text_range_core_map map1 = {
                .text_range = do_block.end_expr->text_range,
                .expr = *expr,
                .sub_maps = sub_maps1
            };

            dy_array_add(sub_maps, &map1);

            return true;
        } else {
            return false;
        }
    }
    case DY_AST_DO_BLOCK_LET:
        return do_block_let_to_core(ctx, do_block.let, expr, sub_maps);
    case DY_AST_DO_BLOCK_IGNORED_EXPR:
        return do_block_ignored_expr_to_core(ctx, do_block.ignored_expr, expr, sub_maps);
    case DY_AST_DO_BLOCK_EQUALITY:
        return do_block_equality_to_core(ctx, do_block.equality, expr, sub_maps);
    }

    DY_IMPOSSIBLE_ENUM();
}

bool do_block_equality_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_do_block_equality equality, struct dy_core_expr *expr, dy_array_t *sub_maps)
{
    struct dy_core_expr e1;
    dy_array_t *sub_maps1 = dy_array_create(dy_allocator_stdlib(), sizeof(struct dy_text_range_core_map), 2);
    bool e1_conversion_succeeded = dy_ast_expr_to_core(ctx, *equality.e1, &e1, sub_maps1);

    struct dy_text_range_core_map map1 = {
        .text_range = equality.e1->text_range,
        .expr = e1,
        .sub_maps = sub_maps1
    };

    struct dy_core_expr e2;
    dy_array_t *sub_maps2 = dy_array_create(dy_allocator_stdlib(), sizeof(struct dy_text_range_core_map), 2);
    bool e2_conversion_succeeded = dy_ast_expr_to_core(ctx, *equality.e2, &e2, sub_maps2);

    struct dy_text_range_core_map map2 = {
        .text_range = equality.e2->text_range,
        .expr = e2,
        .sub_maps = sub_maps2
    };

    struct dy_core_expr rest;
    bool rest_conversion_succeeded = dy_ast_do_block_to_core(ctx, *equality.rest, &rest, sub_maps);

    if (!e1_conversion_succeeded || !e2_conversion_succeeded || !rest_conversion_succeeded) {
        return false;
    }

    dy_array_add(sub_maps, &map1);
    dy_array_add(sub_maps, &map2);

    struct dy_core_expr positive_type_map = {
        .tag = DY_CORE_EXPR_VALUE_MAP,
        .value_map = {
            .e1 = alloc_expr(ctx, e1),
            .e2 = alloc_expr(ctx, rest),
            .polarity = DY_CORE_POLARITY_POSITIVE,
            .is_implicit = false,
        }
    };

    struct dy_core_expr result_type = {
        .tag = DY_CORE_EXPR_UNKNOWN,
        .unknown = {
            .id = (*ctx->running_id)++,
            .type = alloc_expr(ctx, (struct dy_core_expr){ .tag = DY_CORE_EXPR_END, .end_polarity = DY_CORE_POLARITY_POSITIVE }),
            .is_inference_var = false,
        }
    };

    struct dy_core_expr elim = {
        .tag = DY_CORE_EXPR_VALUE_MAP_ELIM,
        .value_map_elim = {
            .id = (*ctx->running_id)++,
            .expr = alloc_expr(ctx, positive_type_map),
            .value_map = {
                .e1 = alloc_expr(ctx, e2),
                .e2 = alloc_expr(ctx, result_type),
                .polarity = DY_CORE_POLARITY_NEGATIVE,
                .is_implicit = false,
            },
        }
    };

    *expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_TYPE_MAP,
        .type_map = {
            .arg_id = result_type.unknown.id,
            .arg_type = result_type.unknown.type,
            .expr = alloc_expr(ctx, elim),
            .polarity = DY_CORE_POLARITY_NEGATIVE,
            .is_implicit = true,
        }
    };

    return true;
}

bool do_block_let_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_do_block_let let, struct dy_core_expr *expr, dy_array_t *sub_maps)
{
    struct dy_core_expr e;
    dy_array_t *sub_maps1 = dy_array_create(dy_allocator_stdlib(), sizeof(struct dy_text_range_core_map), 2);
    bool expr_conversion_succeeded = dy_ast_expr_to_core(ctx, *let.expr, &e, sub_maps1);

    struct dy_text_range_core_map map1 = {
        .text_range = let.expr->text_range,
        .expr = e,
        .sub_maps = sub_maps1
    };

    size_t id = (*ctx->running_id)++;

    struct dy_core_unknown unknown = create_inference_var(ctx);

    struct dy_core_expr *arg_type = ctx->allocator.alloc(sizeof *arg_type, ctx->allocator.env);
    *arg_type = (struct dy_core_expr){
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
    bool rest_conversion_succeeded = dy_ast_do_block_to_core(ctx, *let.rest, &rest, sub_maps);

    dy_array_set_size(ctx->bound_vars, old_size);

    if (!expr_conversion_succeeded || !rest_conversion_succeeded) {
        return false;
    }

    dy_array_add(sub_maps, &map1);

    struct dy_core_expr positive_type_map = {
        .tag = DY_CORE_EXPR_TYPE_MAP,
        .type_map = {
            .arg_id = id,
            .arg_type = arg_type,
            .expr = alloc_expr(ctx, rest),
            .polarity = DY_CORE_POLARITY_POSITIVE,
            .is_implicit = false,
        }
    };

    struct dy_core_expr result_type_map = {
        .tag = DY_CORE_EXPR_TYPE_MAP,
        .type_map = {
            .arg_id = unknown.id,
            .arg_type = unknown.type,
            .expr = alloc_expr(ctx, positive_type_map),
            .polarity = DY_CORE_POLARITY_POSITIVE,
            .is_implicit = true,
        }
    };

    struct dy_core_expr result_type = {
        .tag = DY_CORE_EXPR_UNKNOWN,
        .unknown = {
            .id = (*ctx->running_id)++,
            .type = alloc_expr(ctx, (struct dy_core_expr){ .tag = DY_CORE_EXPR_END, .end_polarity = DY_CORE_POLARITY_POSITIVE }),
            .is_inference_var = false,
        }
    };

    struct dy_core_expr elim = {
        .tag = DY_CORE_EXPR_VALUE_MAP_ELIM,
        .value_map_elim = {
            .id = (*ctx->running_id)++,
            .expr = alloc_expr(ctx, result_type_map),
            .value_map = {
                .e1 = alloc_expr(ctx, e),
                .e2 = alloc_expr(ctx, result_type),
                .polarity = DY_CORE_POLARITY_NEGATIVE,
                .is_implicit = false,
            },
        }
    };

    *expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_TYPE_MAP,
        .type_map = {
            .arg_id = result_type.unknown.id,
            .arg_type = result_type.unknown.type,
            .expr = alloc_expr(ctx, elim),
            .polarity = DY_CORE_POLARITY_NEGATIVE,
            .is_implicit = true,
        }
    };

    return true;
}

bool do_block_ignored_expr_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_do_block_ignored_expr ignored_expr, struct dy_core_expr *expr, dy_array_t *sub_maps)
{
    struct dy_core_expr e;
    dy_array_t *sub_maps1 = dy_array_create(dy_allocator_stdlib(), sizeof(struct dy_text_range_core_map), 2);
    bool expr_conversion_succeeded = dy_ast_expr_to_core(ctx, *ignored_expr.expr, &e, sub_maps1);

    struct dy_text_range_core_map map1 = {
        .text_range = ignored_expr.expr->text_range,
        .expr = e,
        .sub_maps = sub_maps1
    };

    size_t id = (*ctx->running_id)++;

    struct dy_core_expr rest;
    bool rest_conversion_succeeded = dy_ast_do_block_to_core(ctx, *ignored_expr.rest, &rest, sub_maps);

    if (!expr_conversion_succeeded || !rest_conversion_succeeded) {
        return false;
    }

    dy_array_add(sub_maps, &map1);

    struct dy_core_unknown unknown = create_inference_var(ctx);

    struct dy_core_expr unknown_expr = {
        .tag = DY_CORE_EXPR_UNKNOWN,
        .unknown = unknown
    };

    struct dy_core_expr positive_type_map = {
        .tag = DY_CORE_EXPR_TYPE_MAP,
        .type_map = {
            .arg_id = id,
            .arg_type = alloc_expr(ctx, unknown_expr),
            .expr = alloc_expr(ctx, rest),
            .polarity = DY_CORE_POLARITY_POSITIVE,
            .is_implicit = false,
        }
    };

    struct dy_core_expr result_type_map = {
        .tag = DY_CORE_EXPR_TYPE_MAP,
        .type_map = {
            .arg_id = unknown.id,
            .arg_type = unknown.type,
            .expr = alloc_expr(ctx, positive_type_map),
            .polarity = DY_CORE_POLARITY_POSITIVE,
            .is_implicit = true,
        }
    };

    struct dy_core_expr result_type = {
        .tag = DY_CORE_EXPR_UNKNOWN,
        .unknown = {
            .id = (*ctx->running_id)++,
            .type = alloc_expr(ctx, (struct dy_core_expr){ .tag = DY_CORE_EXPR_END, .end_polarity = DY_CORE_POLARITY_POSITIVE }),
            .is_inference_var = false,
        }
    };

    struct dy_core_expr elim = {
        .tag = DY_CORE_EXPR_VALUE_MAP_ELIM,
        .value_map_elim = {
            .id = (*ctx->running_id)++,
            .expr = alloc_expr(ctx, result_type_map),
            .value_map = {
                .e1 = alloc_expr(ctx, e),
                .e2 = alloc_expr(ctx, result_type),
                .polarity = DY_CORE_POLARITY_NEGATIVE,
                .is_implicit = false,
            },
        }
    };

    *expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_TYPE_MAP,
        .type_map = {
            .arg_id = result_type.unknown.id,
            .arg_type = result_type.unknown.type,
            .expr = alloc_expr(ctx, elim),
            .polarity = DY_CORE_POLARITY_NEGATIVE,
            .is_implicit = true,
        }
    };

    return true;
}

bool dy_ast_value_map_elim_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_value_map_elim elim, struct dy_core_expr *expr, dy_array_t *sub_maps)
{
    struct dy_core_expr e1;
    dy_array_t *sub_maps1 = dy_array_create(dy_allocator_stdlib(), sizeof(struct dy_text_range_core_map), 2);
    bool b1 = dy_ast_expr_to_core(ctx, *elim.expr, &e1, sub_maps1);

    struct dy_text_range_core_map map1 = {
        .text_range = elim.expr->text_range,
        .expr = e1,
        .sub_maps = sub_maps1
    };

    struct dy_core_expr e2;
    bool b2 = dy_ast_negative_value_map_to_core(ctx, elim.value_map, &e2, sub_maps);

    if (!b1 || !b2) {
        return false;
    }

    dy_array_add(sub_maps, &map1);

    dy_assert(e2.tag == DY_CORE_EXPR_VALUE_MAP);

    *expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_VALUE_MAP_ELIM,
        .value_map_elim = {
            .id = (*ctx->running_id)++,
            .expr = alloc_expr(ctx, e1),
            .value_map = e2.value_map,
        }
    };

    return true;
}

bool dy_ast_type_map_elim_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_type_map_elim elim, struct dy_core_expr *expr, dy_array_t *sub_maps)
{
    struct dy_core_expr e1;
    dy_array_t *sub_maps1 = dy_array_create(dy_allocator_stdlib(), sizeof(struct dy_text_range_core_map), 2);
    bool b1 = dy_ast_expr_to_core(ctx, *elim.expr, &e1, sub_maps1);

    struct dy_text_range_core_map map1 = {
        .text_range = elim.expr->text_range,
        .expr = e1,
        .sub_maps = sub_maps1
    };

    struct dy_core_expr e2;
    bool b2 = dy_ast_negative_type_map_to_core(ctx, elim.type_map, &e2, sub_maps);

    if (!b1 || !b2) {
        return false;
    }

    dy_array_add(sub_maps, &map1);

    dy_assert(e2.tag == DY_CORE_EXPR_TYPE_MAP);

    *expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_TYPE_MAP_ELIM,
        .type_map_elim = {
            .id = (*ctx->running_id)++,
            .expr = alloc_expr(ctx, e1),
            .type_map = e2.type_map,
        }
    };

    return true;
}

bool dy_ast_juxtaposition_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_juxtaposition juxtaposition, struct dy_core_expr *expr, dy_array_t *sub_maps)
{
    struct dy_core_expr left;
    dy_array_t *sub_maps1 = dy_array_create(dy_allocator_stdlib(), sizeof(struct dy_text_range_core_map), 2);
    bool b1 = dy_ast_expr_to_core(ctx, *juxtaposition.left, &left, sub_maps1);

    struct dy_text_range_core_map map1 = {
        .text_range = juxtaposition.left->text_range,
        .expr = left,
        .sub_maps = sub_maps1
    };

    struct dy_core_expr right;
    dy_array_t *sub_maps2 = dy_array_create(dy_allocator_stdlib(), sizeof(struct dy_text_range_core_map), 2);
    bool b2 = dy_ast_expr_to_core(ctx, *juxtaposition.right, &right, sub_maps2);

    struct dy_text_range_core_map map2 = {
        .text_range = juxtaposition.right->text_range,
        .expr = right,
        .sub_maps = sub_maps2
    };

    if (!b1 || !b2) {
        return false;
    }

    dy_array_add(sub_maps, &map1);
    dy_array_add(sub_maps, &map2);

    struct dy_core_unknown unknown = create_inference_var(ctx);

    struct dy_core_expr unknown_expr = {
        .tag = DY_CORE_EXPR_UNKNOWN,
        .unknown = unknown
    };

    struct dy_core_expr elim = {
        .tag = DY_CORE_EXPR_VALUE_MAP_ELIM,
        .value_map_elim = {
            .id = (*ctx->running_id)++,
            .expr = alloc_expr(ctx, left),
            .value_map = {
                .e1 = alloc_expr(ctx, right),
                .e2 = alloc_expr(ctx, unknown_expr),
                .polarity = DY_CORE_POLARITY_NEGATIVE,
                .is_implicit = false,
            },
        }
    };

    *expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_TYPE_MAP,
        .type_map = {
            .arg_id = unknown.id,
            .arg_type = unknown.type,
            .expr = alloc_expr(ctx, elim),
            .polarity = DY_CORE_POLARITY_NEGATIVE,
            .is_implicit = true,
        }
    };

    return true;
}

struct dy_core_expr *alloc_expr(struct dy_ast_to_core_ctx *ctx, struct dy_core_expr expr)
{
    return dy_alloc(&expr, sizeof expr, ctx->allocator);
}

struct dy_core_unknown create_inference_var(struct dy_ast_to_core_ctx *ctx)
{
    struct dy_core_expr type_all = {
        .tag = DY_CORE_EXPR_END,
        .end_polarity = DY_CORE_POLARITY_POSITIVE
    };

    return (struct dy_core_unknown){
        .id = (*ctx->running_id)++,
        .type = alloc_expr(ctx, type_all),
        .is_inference_var = false
    };
}
