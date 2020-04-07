/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_PARSER_H
#define DY_PARSER_H

#include <duality/syntax/api.h>
#include <duality/syntax/ast.h>

#include <duality/support/string.h>
#include <duality/support/array.h>
#include <duality/support/range.h>
#include <duality/support/stream.h>

struct dy_parser_ctx {
    struct dy_stream stream;
    dy_array_t *string_arrays;
    dy_obj_pool_t *pool;
};

DY_SYNTAX_API bool dy_parse_literal(struct dy_parser_ctx *ctx, dy_string_t s);

DY_SYNTAX_API bool dy_parse_expr(struct dy_parser_ctx *ctx, struct dy_ast_expr *expr);

DY_SYNTAX_API bool dy_parse_variable(struct dy_parser_ctx *ctx, dy_string_t *var);

DY_SYNTAX_API bool dy_parse_positive_type_map(struct dy_parser_ctx *ctx, struct dy_ast_type_map *type_map);

DY_SYNTAX_API bool dy_parse_negative_type_map(struct dy_parser_ctx *ctx, struct dy_ast_type_map *type_map);

DY_SYNTAX_API bool dy_parse_implicit_positive_type_map(struct dy_parser_ctx *ctx, struct dy_ast_type_map *type_map);

DY_SYNTAX_API bool dy_parse_implicit_negative_type_map(struct dy_parser_ctx *ctx, struct dy_ast_type_map *type_map);

DY_SYNTAX_API bool dy_parse_positive_recursion(struct dy_parser_ctx *ctx, struct dy_ast_recursion *recursion);

DY_SYNTAX_API bool dy_parse_negative_recursion(struct dy_parser_ctx *ctx, struct dy_ast_recursion *recursion);

DY_SYNTAX_API bool dy_parse_expr_map_elim(struct dy_parser_ctx *ctx, struct dy_ast_expr_map_elim *expr_map_elim);

DY_SYNTAX_API bool dy_parse_type_map_elim(struct dy_parser_ctx *ctx, struct dy_ast_type_map_elim *type_map_elim);

DY_SYNTAX_API bool dy_parse_do_block(struct dy_parser_ctx *ctx, struct dy_ast_do_block *do_block);

DY_SYNTAX_API bool dy_parse_file(struct dy_parser_ctx *ctx, struct dy_ast_do_block *do_block);

DY_SYNTAX_API bool dy_parse_list(struct dy_parser_ctx *ctx, struct dy_ast_list *list);

DY_SYNTAX_API bool dy_parse_try_block(struct dy_parser_ctx *ctx, struct dy_ast_list *try_block);

DY_SYNTAX_API bool dy_parse_choice(struct dy_parser_ctx *ctx, struct dy_ast_list *choice);

DY_SYNTAX_API bool dy_parse_string(struct dy_parser_ctx *ctx, dy_string_t *string);

#endif // DY_PARSER_H
