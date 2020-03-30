/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_AST_TO_CORE_H
#define DY_AST_TO_CORE_H

#include <duality/syntax/ast.h>
#include <duality/syntax/parser.h>

#include <duality/core/core.h>

#include <duality/support/array.h>
#include <duality/support/obj_pool.h>

struct dy_ast_to_core_bound_var {
    dy_string_t name;
    size_t replacement_id;
    struct dy_core_expr type;
};

struct dy_ast_to_core_ctx {
    size_t *running_id;
    dy_array_t *bound_vars;
    dy_array_t *unbound_vars;
    dy_obj_pool_t *core_expr_pool;
};

DY_SYNTAX_API bool dy_ast_expr_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_expr expr, struct dy_core_expr *core_expr);

DY_SYNTAX_API bool dy_ast_positive_type_map_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_type_map type_map, struct dy_core_expr *expr);

DY_SYNTAX_API bool dy_ast_negative_type_map_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_type_map type_map, struct dy_core_expr *expr);

DY_SYNTAX_API bool dy_ast_list_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_list list, struct dy_core_expr *expr);

DY_SYNTAX_API bool dy_ast_choice_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_list choice, struct dy_core_expr *expr);

DY_SYNTAX_API bool dy_ast_try_block_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_list try_block, struct dy_core_expr *expr);

DY_SYNTAX_API bool dy_ast_positive_expr_map_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_expr_map expr_map, struct dy_core_expr *expr);

DY_SYNTAX_API bool dy_ast_negative_expr_map_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_expr_map expr_map, struct dy_core_expr *expr);

DY_SYNTAX_API bool dy_ast_do_block_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_do_block do_block, struct dy_core_expr *expr);

DY_SYNTAX_API bool dy_ast_expr_map_elim_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_expr_map_elim elim, struct dy_core_expr *expr);

DY_SYNTAX_API bool dy_ast_type_map_elim_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_type_map_elim elim, struct dy_core_expr *expr);

DY_SYNTAX_API bool dy_ast_juxtaposition_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_juxtaposition juxtaposition, struct dy_core_expr *expr);

DY_SYNTAX_API bool dy_ast_variable_to_core(struct dy_ast_to_core_ctx *ctx, dy_string_t variable, struct dy_core_expr *expr);

#endif // DY_AST_TO_CORE_H
