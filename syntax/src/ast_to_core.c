/*
 * Copyright 2017-2019 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <duality/syntax/ast_to_core.h>

#include <duality/support/assert.h>
#include <duality/core/type_of.h>

static struct dy_core_expr *alloc_expr(struct dy_ast_to_core_ctx ctx, struct dy_core_expr expr);

static bool ast_type_map_to_core(struct dy_ast_to_core_ctx ctx, struct dy_ast_type_map type_map, enum dy_core_polarity polarity, struct dy_core_expr *expr);

static bool ast_value_map_to_core(struct dy_ast_to_core_ctx ctx, struct dy_ast_value_map value_map, enum dy_core_polarity polarity, struct dy_core_expr *expr);

static bool do_block_equality_to_core(struct dy_ast_to_core_ctx ctx, struct dy_ast_do_block_equality equality, struct dy_core_expr *expr);

static bool do_block_let_to_core(struct dy_ast_to_core_ctx ctx, struct dy_ast_do_block_let let, struct dy_core_expr *expr);

static bool do_block_ignored_expr_to_core(struct dy_ast_to_core_ctx ctx, struct dy_ast_do_block_ignored_expr ignored_expr, struct dy_core_expr *expr);

bool dy_ast_expr_to_core(struct dy_ast_to_core_ctx ctx, struct dy_ast_expr expr, struct dy_core_expr *core_expr)
{
    switch (expr.tag) {
    case DY_AST_EXPR_POSITIVE_TYPE_MAP:
        return dy_ast_positive_type_map_to_core(ctx, expr.positive_type_map, core_expr);
    case DY_AST_EXPR_NEGATIVE_TYPE_MAP:
        return dy_ast_negative_type_map_to_core(ctx, expr.negative_type_map, core_expr);
    case DY_AST_EXPR_LIST:
        return dy_ast_list_to_core(ctx, expr.list, core_expr);
    case DY_AST_EXPR_POSITIVE_VALUE_MAP:
        return dy_ast_positive_value_map_to_core(ctx, expr.positive_value_map, core_expr);
    case DY_AST_EXPR_NEGATIVE_VALUE_MAP:
        return dy_ast_negative_value_map_to_core(ctx, expr.negative_value_map, core_expr);
    case DY_AST_EXPR_DO_BLOCK:
        return dy_ast_do_block_to_core(ctx, expr.do_block, core_expr);
    case DY_AST_EXPR_CHOICE:
        return dy_ast_choice_to_core(ctx, expr.choice, core_expr);
    case DY_AST_EXPR_TRY_BLOCK:
        return dy_ast_try_block_to_core(ctx, expr.try_block, core_expr);
    case DY_AST_EXPR_VALUE_MAP_ELIM:
        return dy_ast_value_map_elim_to_core(ctx, expr.value_map_elim, core_expr);
    case DY_AST_EXPR_TYPE_MAP_ELIM:
        return dy_ast_type_map_elim_to_core(ctx, expr.type_map_elim, core_expr);
    case DY_AST_EXPR_VARIABLE:
        return dy_ast_variable_to_core(ctx, expr.variable, core_expr);
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
    case DY_AST_EXPR_TYPE_TYPE:
        *core_expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_TYPE_OF_TYPES
        };

        return true;
    }

    DY_IMPOSSIBLE_ENUM();
}

bool dy_ast_variable_to_core(struct dy_ast_to_core_ctx ctx, dy_string_t variable, struct dy_core_expr *expr)
{
    for (size_t i = dy_array_size(ctx.bound_vars); i-- > 0;) {
        struct dy_ast_to_core_bound_var bound_var;
        dy_array_get(ctx.bound_vars, i, &bound_var);

        if (dy_string_are_equal(variable, bound_var.name)) {
            *expr = (struct dy_core_expr){
                .tag = DY_CORE_EXPR_UNKNOWN,
                .unknown = {
                    .id = bound_var.replacement_id,
                    .type = bound_var.type,
                }
            };

            return true;
        }
    }

    dy_array_add(ctx.unbound_vars, &variable);

    return false;
}

bool ast_type_map_to_core(struct dy_ast_to_core_ctx ctx, struct dy_ast_type_map type_map, enum dy_core_polarity polarity, struct dy_core_expr *expr)
{
    struct dy_core_expr *type = ctx.allocator.alloc(sizeof *type, ctx.allocator.env);
    if (!dy_ast_expr_to_core(ctx, *type_map.arg.type, type)) {
        return false;
    }

    size_t id = (*ctx.running_id)++;

    size_t old_size;
    if (type_map.arg.has_name) {
        struct dy_ast_to_core_bound_var bound_var = {
            .name = type_map.arg.name,
            .replacement_id = id,
            .type = type
        };

        old_size = dy_array_add(ctx.bound_vars, &bound_var);
    } else {
        old_size = dy_array_size(ctx.bound_vars);
    }

    struct dy_core_expr e;
    bool succeeded = dy_ast_expr_to_core(ctx, *type_map.expr, &e);

    dy_array_set_size(ctx.bound_vars, old_size);

    if (!succeeded) {
        return false;
    }

    *expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_TYPE_MAP,
        .type_map = {
            .arg = {
                .id = id,
                .type = type },
            .expr = alloc_expr(ctx, e),
            .polarity = polarity }
    };

    return true;
}

bool dy_ast_positive_type_map_to_core(struct dy_ast_to_core_ctx ctx, struct dy_ast_type_map type_map, struct dy_core_expr *expr)
{
    return ast_type_map_to_core(ctx, type_map, DY_CORE_POLARITY_POSITIVE, expr);
}

bool dy_ast_negative_type_map_to_core(struct dy_ast_to_core_ctx ctx, struct dy_ast_type_map type_map, struct dy_core_expr *expr)
{
    return ast_type_map_to_core(ctx, type_map, DY_CORE_POLARITY_NEGATIVE, expr);
}

bool dy_ast_list_to_core(struct dy_ast_to_core_ctx ctx, struct dy_ast_list list, struct dy_core_expr *expr)
{
    dy_assert(list.num_exprs != 0);

    if (list.num_exprs == 1) {
        return dy_ast_expr_to_core(ctx, list.exprs[0], expr);
    }

    struct dy_ast_list new_list = {
        .exprs = list.exprs + 1,
        .num_exprs = list.num_exprs - 1
    };

    struct dy_core_expr e1;
    bool b1 = dy_ast_expr_to_core(ctx, list.exprs[0], &e1);

    struct dy_core_expr e2;
    bool b2 = dy_ast_list_to_core(ctx, new_list, &e2);

    if (!b1 || !b2) {
        return false;
    }

    *expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_BOTH,
        .both = {
            .first = alloc_expr(ctx, e1),
            .second = alloc_expr(ctx, e2),
        }
    };

    return true;
}

bool dy_ast_choice_to_core(struct dy_ast_to_core_ctx ctx, struct dy_ast_list choice, struct dy_core_expr *expr)
{
    dy_assert(choice.num_exprs != 0);

    if (choice.num_exprs == 1) {
        return dy_ast_expr_to_core(ctx, choice.exprs[0], expr);
    }

    struct dy_ast_list new_choice = {
        .exprs = choice.exprs + 1,
        .num_exprs = choice.num_exprs - 1
    };

    struct dy_core_expr e1;
    bool b1 = dy_ast_expr_to_core(ctx, choice.exprs[0], &e1);

    struct dy_core_expr e2;
    bool b2 = dy_ast_choice_to_core(ctx, new_choice, &e2);

    if (!b1 || !b2) {
        return false;
    }

    *expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_ANY_OF,
        .any_of = {
            .first = alloc_expr(ctx, e1),
            .second = alloc_expr(ctx, e2),
        }
    };

    return true;
}

bool dy_ast_try_block_to_core(struct dy_ast_to_core_ctx ctx, struct dy_ast_list try_block, struct dy_core_expr *expr)
{
    dy_assert(try_block.num_exprs != 0);

    if (try_block.num_exprs == 1) {
        return dy_ast_expr_to_core(ctx, try_block.exprs[0], expr);
    }

    struct dy_ast_list new_try_block = {
        .exprs = try_block.exprs + 1,
        .num_exprs = try_block.num_exprs - 1
    };

    struct dy_core_expr e1;
    bool b1 = dy_ast_expr_to_core(ctx, try_block.exprs[0], &e1);

    struct dy_core_expr e2;
    bool b2 = dy_ast_try_block_to_core(ctx, new_try_block, &e2);

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

bool ast_value_map_to_core(struct dy_ast_to_core_ctx ctx, struct dy_ast_value_map value_map, enum dy_core_polarity polarity, struct dy_core_expr *expr)
{
    struct dy_core_expr e1;
    bool b1 = dy_ast_expr_to_core(ctx, *value_map.e1, &e1);

    struct dy_core_expr e2;
    bool b2 = dy_ast_expr_to_core(ctx, *value_map.e2, &e2);

    if (!b1 || !b2) {
        return false;
    }

    *expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_VALUE_MAP,
        .value_map = {
            .e1 = alloc_expr(ctx, e1),
            .e2 = alloc_expr(ctx, e2),
            .polarity = polarity,
        }
    };

    return true;
}

bool dy_ast_positive_value_map_to_core(struct dy_ast_to_core_ctx ctx, struct dy_ast_value_map positive_value_map, struct dy_core_expr *expr)
{
    return ast_value_map_to_core(ctx, positive_value_map, DY_CORE_POLARITY_POSITIVE, expr);
}

bool dy_ast_negative_value_map_to_core(struct dy_ast_to_core_ctx ctx, struct dy_ast_value_map negative_value_map, struct dy_core_expr *expr)
{
    return ast_value_map_to_core(ctx, negative_value_map, DY_CORE_POLARITY_NEGATIVE, expr);
}

bool dy_ast_do_block_to_core(struct dy_ast_to_core_ctx ctx, struct dy_ast_do_block do_block, struct dy_core_expr *expr)
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

bool do_block_equality_to_core(struct dy_ast_to_core_ctx ctx, struct dy_ast_do_block_equality equality, struct dy_core_expr *expr)
{
    struct dy_core_expr e1;
    if (!dy_ast_expr_to_core(ctx, *equality.e1, &e1)) {
        return false;
    }

    struct dy_core_expr e2;
    if (!dy_ast_expr_to_core(ctx, *equality.e2, &e2)) {
        return false;
    }

    struct dy_core_expr rest;
    if (!dy_ast_do_block_to_core(ctx, *equality.rest, &rest)) {
        return false;
    }

    struct dy_check_ctx check_ctx = {
        .running_id = ctx.running_id,
        .allocator = ctx.allocator
    };

    struct dy_core_expr type_of_rest = dy_type_of(check_ctx, rest);

    struct dy_core_expr positive_type_map = {
        .tag = DY_CORE_EXPR_VALUE_MAP,
        .value_map = {
            .e1 = alloc_expr(ctx, e1),
            .e2 = alloc_expr(ctx, rest),
            .polarity = DY_CORE_POLARITY_POSITIVE,
        }
    };

    *expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_VALUE_MAP_ELIM,
        .value_map_elim = {
            .expr = alloc_expr(ctx, positive_type_map),
            .value_map = {
                .e1 = alloc_expr(ctx, e2),
                .e2 = alloc_expr(ctx, type_of_rest),
                .polarity = DY_CORE_POLARITY_NEGATIVE,
            },
        }
    };

    return true;
}

bool do_block_let_to_core(struct dy_ast_to_core_ctx ctx, struct dy_ast_do_block_let let, struct dy_core_expr *expr)
{
    struct dy_core_expr e;
    if (!dy_ast_expr_to_core(ctx, *let.expr, &e)) {
        return false;
    }

    size_t id = (*ctx.running_id)++;

    struct dy_check_ctx check_ctx = {
        .running_id = ctx.running_id,
        .allocator = ctx.allocator
    };

    struct dy_core_expr *type_of_e = alloc_expr(ctx, dy_type_of(check_ctx, e));

    struct dy_ast_to_core_bound_var bound_var = {
        .name = let.arg_name,
        .replacement_id = id,
        .type = type_of_e
    };

    size_t old_size = dy_array_add(ctx.bound_vars, &bound_var);

    struct dy_core_expr rest;
    bool succeeded = dy_ast_do_block_to_core(ctx, *let.rest, &rest);

    dy_array_set_size(ctx.bound_vars, old_size);

    if (!succeeded) {
        return false;
    }

    struct dy_core_expr type_of_rest = dy_type_of(check_ctx, rest);

    struct dy_core_expr positive_type_map = {
        .tag = DY_CORE_EXPR_TYPE_MAP,
        .type_map = {
            .arg = {
                .id = id,
                .type = type_of_e,
            },
            .expr = alloc_expr(ctx, rest),
            .polarity = DY_CORE_POLARITY_POSITIVE,
        }
    };

    *expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_VALUE_MAP_ELIM,
        .value_map_elim = {
            .expr = alloc_expr(ctx, positive_type_map),
            .value_map = {
                .e1 = alloc_expr(ctx, e),
                .e2 = alloc_expr(ctx, type_of_rest),
                .polarity = DY_CORE_POLARITY_NEGATIVE,
            },
        }
    };

    return true;
}

bool do_block_ignored_expr_to_core(struct dy_ast_to_core_ctx ctx, struct dy_ast_do_block_ignored_expr ignored_expr, struct dy_core_expr *expr)
{
    struct dy_core_expr e;
    if (!dy_ast_expr_to_core(ctx, *ignored_expr.expr, &e)) {
        return false;
    }

    size_t id = (*ctx.running_id)++;

    struct dy_check_ctx check_ctx = {
        .running_id = ctx.running_id,
        .allocator = ctx.allocator
    };

    struct dy_core_expr rest;
    if (!dy_ast_do_block_to_core(ctx, *ignored_expr.rest, &rest)) {
        return false;
    }

    struct dy_core_expr type_of_rest = dy_type_of(check_ctx, rest);

    struct dy_core_expr positive_type_map = {
        .tag = DY_CORE_EXPR_TYPE_MAP,
        .type_map = {
            .arg = {
                .id = id,
                .type = alloc_expr(ctx, dy_type_of(check_ctx, e)),
            },
            .expr = alloc_expr(ctx, rest),
            .polarity = DY_CORE_POLARITY_POSITIVE,
        }
    };

    *expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_VALUE_MAP_ELIM,
        .value_map_elim = {
            .expr = alloc_expr(ctx, positive_type_map),
            .value_map = {
                .e1 = alloc_expr(ctx, e),
                .e2 = alloc_expr(ctx, type_of_rest),
                .polarity = DY_CORE_POLARITY_NEGATIVE,
            },
        }
    };

    return true;
}

bool dy_ast_value_map_elim_to_core(struct dy_ast_to_core_ctx ctx, struct dy_ast_value_map_elim elim, struct dy_core_expr *expr)
{
    struct dy_core_expr e1;
    bool b1 = dy_ast_expr_to_core(ctx, *elim.expr, &e1);

    struct dy_core_expr e2;
    bool b2 = dy_ast_negative_value_map_to_core(ctx, elim.value_map, &e2);

    if (!b1 || !b2) {
        return false;
    }

    dy_assert(e2.tag == DY_CORE_EXPR_VALUE_MAP);

    *expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_VALUE_MAP_ELIM,
        .value_map_elim = {
            .expr = alloc_expr(ctx, e1),
            .value_map = e2.value_map }
    };

    return true;
}

bool dy_ast_type_map_elim_to_core(struct dy_ast_to_core_ctx ctx, struct dy_ast_type_map_elim elim, struct dy_core_expr *expr)
{
    struct dy_core_expr e1;
    bool b1 = dy_ast_expr_to_core(ctx, *elim.expr, &e1);

    struct dy_core_expr e2;
    bool b2 = dy_ast_negative_type_map_to_core(ctx, elim.type_map, &e2);

    if (!b1 || !b2) {
        return false;
    }

    dy_assert(e2.tag == DY_CORE_EXPR_TYPE_MAP);

    *expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_TYPE_MAP_ELIM,
        .type_map_elim = {
            .expr = alloc_expr(ctx, e1),
            .type_map = e2.type_map }
    };

    return true;
}

struct dy_core_expr *alloc_expr(struct dy_ast_to_core_ctx ctx, struct dy_core_expr expr)
{
    return dy_alloc(&expr, sizeof expr, ctx.allocator);
}
