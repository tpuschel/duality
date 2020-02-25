/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_AST_TO_CORE_H
#define DY_AST_TO_CORE_H

#include <duality/syntax/ast.h>
#include <duality/core/core.h>
#include <duality/support/array.h>
#include <duality/syntax/parser.h>

struct dy_ast_to_core_bound_var {
    dy_string_t name;
    size_t replacement_id;
    const struct dy_core_expr *type;
};

struct dy_ast_to_core_ctx {
    size_t *running_id;
    struct dy_allocator allocator;
    dy_array_t *bound_vars;
    dy_array_t *unbound_vars;
};

struct dy_text_range_core_map {
    struct dy_range text_range;
    struct dy_core_expr expr;
    dy_array_t *sub_maps;
};

DY_SYNTAX_API bool dy_ast_expr_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_expr expr, struct dy_core_expr *core_expr, dy_array_t *sub_maps);

DY_SYNTAX_API bool dy_ast_positive_type_map_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_type_map type_map, struct dy_core_expr *expr, dy_array_t *sub_maps);

DY_SYNTAX_API bool dy_ast_negative_type_map_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_type_map type_map, struct dy_core_expr *expr, dy_array_t *sub_maps);

DY_SYNTAX_API bool dy_ast_list_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_list list, struct dy_core_expr *expr, dy_array_t *sub_maps);

DY_SYNTAX_API bool dy_ast_choice_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_list choice, struct dy_core_expr *expr, dy_array_t *sub_maps);

DY_SYNTAX_API bool dy_ast_try_block_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_list try_block, struct dy_core_expr *expr, dy_array_t *sub_maps);

DY_SYNTAX_API bool dy_ast_positive_value_map_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_value_map value_map, struct dy_core_expr *expr, dy_array_t *sub_maps);

DY_SYNTAX_API bool dy_ast_negative_value_map_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_value_map value_map, struct dy_core_expr *expr, dy_array_t *sub_maps);

DY_SYNTAX_API bool dy_ast_do_block_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_do_block do_block, struct dy_core_expr *expr, dy_array_t *sub_maps);

DY_SYNTAX_API bool dy_ast_value_map_elim_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_value_map_elim elim, struct dy_core_expr *expr, dy_array_t *sub_maps);

DY_SYNTAX_API bool dy_ast_type_map_elim_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_type_map_elim elim, struct dy_core_expr *expr, dy_array_t *sub_maps);

DY_SYNTAX_API bool dy_ast_juxtaposition_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_juxtaposition juxtaposition, struct dy_core_expr *expr, dy_array_t *sub_maps);

DY_SYNTAX_API bool dy_ast_variable_to_core(struct dy_ast_to_core_ctx *ctx, dy_string_t variable, struct dy_core_expr *expr);

#endif // DY_AST_TO_CORE_H
