/*
 * Copyright 2017-2019 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_PARSER_H
#define DY_PARSER_H

#include <duality/syntax/api.h>
#include <duality/syntax/ast.h>

#include <duality/support/allocator.h>
#include <duality/support/string.h>
#include <duality/support/array.h>

enum dy_parser_error_choice_tag {
    DY_PARSER_ERROR_CHOICE_NO_SUMMARY,
    DY_PARSER_ERROR_CHOICE_EXPRESSION,
    DY_PARSER_ERROR_CHOICE_LOWERCASE_LETTERS,
    DY_PARSER_ERROR_CHOICE_LOWERCASE_LETTERS_DIGITS_UNDERSCORE_AND_QUESTION_MARK
};

struct dy_parser_error_choice {
    dy_array_t *errors_without_index;
    enum dy_parser_error_choice_tag tag;
};

enum dy_parser_error_tag {
    DY_PARSER_ERROR_EXPECTED_ANY_CHAR,
    DY_PARSER_ERROR_EXPECTED_CHAR,
    DY_PARSER_ERROR_CHOICE
};

struct dy_parser_error_without_index {
    union {
        char expected_char;
        struct dy_parser_error_choice choice;
    };

    enum dy_parser_error_tag tag;
};

struct dy_parser_error {
    size_t text_index;
    struct dy_parser_error_without_index error_without_index;
};

struct dy_parser_ctx {
    dy_string_t text;

    size_t index_in;

    struct dy_allocator allocator;

    size_t *index_out;
    struct dy_parser_error *error;
};

struct dy_parser_transaction_multiple {
    size_t *index_out;
};

struct dy_parser_transaction_choice {
    struct dy_parser_error *actual_error_ptr;
    size_t largest_text_index;
    struct dy_parser_error *last_error;
    dy_array_t *accum_errors;
};

DY_SYNTAX_API struct dy_parser_transaction_multiple dy_parse_multiple_start(struct dy_parser_ctx *ctx);

DY_SYNTAX_API void dy_parse_multiple_succeeded(struct dy_parser_ctx ctx, struct dy_parser_transaction_multiple transaction);


DY_SYNTAX_API struct dy_parser_transaction_choice dy_parse_choice_start(struct dy_parser_ctx *ctx);

DY_SYNTAX_API void dy_parse_choice_failed(struct dy_parser_transaction_choice *transaction);

DY_SYNTAX_API void dy_parse_all_choices_failed(struct dy_parser_ctx *ctx, struct dy_parser_transaction_choice transaction, enum dy_parser_error_choice_tag tag);


DY_SYNTAX_API bool dy_parse_literal(struct dy_parser_ctx ctx, dy_string_t s);

DY_SYNTAX_API bool dy_parse_expr(struct dy_parser_ctx ctx, struct dy_ast_expr *expr);

DY_SYNTAX_API bool dy_parse_variable(struct dy_parser_ctx ctx, dy_string_t *var);

DY_SYNTAX_API bool dy_parse_positive_type_map(struct dy_parser_ctx ctx, struct dy_ast_type_map *type_map);

DY_SYNTAX_API bool dy_parse_negative_type_map(struct dy_parser_ctx ctx, struct dy_ast_type_map *type_map);

DY_SYNTAX_API bool dy_parse_value_map_elim(struct dy_parser_ctx ctx, struct dy_ast_value_map_elim *value_map_elim);

DY_SYNTAX_API bool dy_parse_type_map_elim(struct dy_parser_ctx ctx, struct dy_ast_type_map_elim *type_map_elim);

DY_SYNTAX_API bool dy_parse_do_block(struct dy_parser_ctx ctx, struct dy_ast_do_block *do_block);

DY_SYNTAX_API bool dy_parse_file(struct dy_parser_ctx ctx, struct dy_ast_do_block *do_block);

DY_SYNTAX_API bool dy_parse_list(struct dy_parser_ctx ctx, struct dy_ast_list *list);

DY_SYNTAX_API bool dy_parse_try_block(struct dy_parser_ctx ctx, struct dy_ast_list *try_block);

DY_SYNTAX_API bool dy_parse_choice(struct dy_parser_ctx ctx, struct dy_ast_list *choice);

DY_SYNTAX_API bool dy_parse_string(struct dy_parser_ctx ctx, dy_string_t *string);

#endif // DY_PARSER_H
