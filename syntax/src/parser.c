/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <duality/syntax/parser.h>

#include <duality/support/assert.h>

enum infix_op {
    INFIX_OP_STRAIGHT_ARROW,
    INFIX_OP_SQUIGGLY_ARROW,
    INFIX_OP_JUXTAPOSITION,
    INFIX_OP_AT_STRAIGHT_ARROW,
    INFIX_OP_AT_SQUIGGLY_ARROW,
    INFIX_OP_BANG
};

enum assoc {
    ASSOC_LEFT,
    ASSOC_RIGHT
};

static bool left_op_is_first(enum infix_op left, enum infix_op right);

static bool parse_expr_non_left_recursive(struct dy_parser_ctx *ctx, struct dy_ast_expr *expr);

static enum infix_op parse_infix_op(struct dy_parser_ctx *ctx);

static bool parse_expr_further(struct dy_parser_ctx *ctx, struct dy_ast_expr left, enum infix_op left_op, struct dy_ast_expr *expr);

static bool combine_infix(struct dy_parser_ctx *ctx, struct dy_ast_expr left, enum infix_op op, struct dy_ast_expr right, struct dy_ast_expr *expr);

static bool parse_expr_parenthesized(struct dy_parser_ctx *ctx, struct dy_ast_expr *expr);

static bool parse_arg(struct dy_parser_ctx *ctx, struct dy_ast_arg *arg);

static void skip_whitespace_except_newline(struct dy_parser_ctx *ctx);

static void skip_whitespace(struct dy_parser_ctx *ctx);

static bool get_char(struct dy_parser_ctx *ctx, char *c);

static bool parse_one_of(struct dy_parser_ctx *ctx, dy_string_t chars, char *c);

static bool parse_exactly_one(struct dy_parser_ctx *ctx, char c);

static bool parse_bare_list(struct dy_parser_ctx *ctx, struct dy_ast_list *list);

static bool parse_bare_list_inner(struct dy_parser_ctx *ctx, struct dy_ast_list *list);

static bool skip_line_comment(struct dy_parser_ctx *ctx);

static bool skip_block_comment(struct dy_parser_ctx *ctx);

static bool parse_type_map(struct dy_parser_ctx *ctx, dy_string_t arrow_literal, struct dy_ast_type_map *type_map);

static bool parse_do_block_body(struct dy_parser_ctx *ctx, struct dy_ast_do_block *do_block);

static bool parse_do_block_body_equality(struct dy_parser_ctx *ctx, struct dy_ast_do_block_equality *equality);

static bool parse_do_block_body_let(struct dy_parser_ctx *ctx, struct dy_ast_do_block_let *let);

static bool parse_do_block_body_ignored_expr(struct dy_parser_ctx *ctx, struct dy_ast_do_block_ignored_expr *ignored_expr);

static bool skip_semicolon_or_newline(struct dy_parser_ctx *ctx);

bool dy_parse_literal(struct dy_parser_ctx *ctx, dy_string_t s)
{
    size_t start_index = ctx->stream.current_index;

    for (size_t i = 0; i < s.size; ++i) {
        if (!parse_exactly_one(ctx, s.ptr[i])) {
            ctx->stream.current_index = start_index;
            return false;
        }
    }

    return true;
}

bool dy_parse_variable(struct dy_parser_ctx *ctx, dy_string_t *var)
{
    size_t start_index = ctx->stream.current_index;

    dy_array_t *var_name = dy_array_create(sizeof(char), 8);
    dy_array_add(ctx->string_arrays, &var_name);

    char c;
    if (!parse_one_of(ctx, DY_STR_LIT("abcdefghijklmnopqrstuvwxyz"), &c)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_array_add(var_name, &c);

    for (;;) {
        if (!parse_one_of(ctx, DY_STR_LIT("abcdefghijklmnopqrstuvwxyz0123456789-?"), &c)) {
            dy_string_t final_var = {
                .ptr = dy_array_buffer(var_name),
                .size = dy_array_size(var_name)
            };

            if (dy_string_are_equal(final_var, DY_STR_LIT("list"))
                || dy_string_are_equal(final_var, DY_STR_LIT("try"))
                || dy_string_are_equal(final_var, DY_STR_LIT("let"))
                || dy_string_are_equal(final_var, DY_STR_LIT("choice"))
                || dy_string_are_equal(final_var, DY_STR_LIT("String"))
                || dy_string_are_equal(final_var, DY_STR_LIT("All"))
                || dy_string_are_equal(final_var, DY_STR_LIT("Nothing"))
                || dy_string_are_equal(final_var, DY_STR_LIT("rec"))) {
                ctx->stream.current_index = start_index;
                return false;
            }

            *var = final_var;

            return true;
        }

        dy_array_add(var_name, &c);
    }
}

bool skip_line_comment(struct dy_parser_ctx *ctx)
{
    size_t start_index = ctx->stream.current_index;

    if (!dy_parse_literal(ctx, DY_STR_LIT("#"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    for (;;) {
        char c;
        if (!get_char(ctx, &c)) {
            return true;
        }

        if (c == '\n') {
            return true;
        }
    }
}

bool skip_block_comment(struct dy_parser_ctx *ctx)
{
    size_t start_index = ctx->stream.current_index;

    if (!dy_parse_literal(ctx, DY_STR_LIT("/#"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    for (;;) {
        if (skip_block_comment(ctx)) {
            continue;
        }

        if (dy_parse_literal(ctx, DY_STR_LIT("#/"))) {
            return true;
        }

        char c;
        if (get_char(ctx, &c)) {
            continue;
        }

        ctx->stream.current_index = start_index;

        return false;
    }
}

bool dy_parse_expr(struct dy_parser_ctx *ctx, struct dy_ast_expr *expr)
{
    size_t start_index = ctx->stream.current_index;

    struct dy_ast_expr left;
    if (!parse_expr_non_left_recursive(ctx, &left)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    enum infix_op op = parse_infix_op(ctx);

    skip_whitespace_except_newline(ctx);

    bool result = parse_expr_further(ctx, left, op, expr);

    dy_ast_expr_release(ctx->pool, left);

    if (!result) {
        ctx->stream.current_index = start_index;
        return false;
    }

    return result;
}

bool parse_expr_further(struct dy_parser_ctx *ctx, struct dy_ast_expr left, enum infix_op left_op, struct dy_ast_expr *expr)
{
    size_t start_index = ctx->stream.current_index;

    struct dy_ast_expr right;
    if (!parse_expr_non_left_recursive(ctx, &right)) {
        if (left_op == INFIX_OP_JUXTAPOSITION) {
            *expr = dy_ast_expr_retain(ctx->pool, left);
            return true;
        }

        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    enum infix_op right_op = parse_infix_op(ctx);

    if (left_op_is_first(left_op, right_op)) {
        struct dy_ast_expr new_left;
        bool result = combine_infix(ctx, left, left_op, right, &new_left);

        dy_ast_expr_release(ctx->pool, right);

        if (!result) {
            ctx->stream.current_index = start_index;
            return false;
        }

        skip_whitespace_except_newline(ctx);

        result = parse_expr_further(ctx, new_left, right_op, expr);

        dy_ast_expr_release(ctx->pool, new_left);

        if (!result) {
            ctx->stream.current_index = start_index;
            return false;
        }

        return true;
    } else {
        skip_whitespace_except_newline(ctx);

        struct dy_ast_expr new_right;
        bool result = parse_expr_further(ctx, right, right_op, &new_right);

        dy_ast_expr_release(ctx->pool, right);

        if (!result) {
            ctx->stream.current_index = start_index;
            return false;
        }

        result = combine_infix(ctx, left, left_op, new_right, expr);

        dy_ast_expr_release(ctx->pool, new_right);

        if (!result) {
            ctx->stream.current_index = start_index;
            return false;
        }

        return true;
    }
}

bool parse_expr_non_left_recursive(struct dy_parser_ctx *ctx, struct dy_ast_expr *expr)
{
    size_t start_index = ctx->stream.current_index;

    struct dy_ast_type_map positive_type_map;
    if (dy_parse_positive_type_map(ctx, &positive_type_map)) {
        *expr = (struct dy_ast_expr){
            .text_range = {
                .start = start_index,
                .end = ctx->stream.current_index,
            },
            .tag = DY_AST_EXPR_POSITIVE_TYPE_MAP,
            .positive_type_map = positive_type_map
        };

        return true;
    }

    struct dy_ast_type_map negative_type_map;
    if (dy_parse_negative_type_map(ctx, &negative_type_map)) {
        *expr = (struct dy_ast_expr){
            .text_range = {
                .start = start_index,
                .end = ctx->stream.current_index,
            },
            .tag = DY_AST_EXPR_NEGATIVE_TYPE_MAP,
            .negative_type_map = negative_type_map
        };

        return true;
    }

    struct dy_ast_list list;
    if (dy_parse_list(ctx, &list)) {
        *expr = (struct dy_ast_expr){
            .text_range = {
                .start = start_index,
                .end = ctx->stream.current_index,
            },
            .tag = DY_AST_EXPR_LIST,
            .list = list
        };

        return true;
    }

    struct dy_ast_list choice;
    if (dy_parse_choice(ctx, &choice)) {
        *expr = (struct dy_ast_expr){
            .text_range = {
                .start = start_index,
                .end = ctx->stream.current_index,
            },
            .tag = DY_AST_EXPR_CHOICE,
            .choice = choice
        };

        return true;
    }

    struct dy_ast_list try_block;
    if (dy_parse_try_block(ctx, &try_block)) {
        *expr = (struct dy_ast_expr){
            .text_range = {
                .start = start_index,
                .end = ctx->stream.current_index,
            },
            .tag = DY_AST_EXPR_TRY_BLOCK,
            .try_block = try_block
        };

        return true;
    }

    struct dy_ast_do_block do_block;
    if (dy_parse_do_block(ctx, &do_block)) {
        *expr = (struct dy_ast_expr){
            .text_range = {
                .start = start_index,
                .end = ctx->stream.current_index,
            },
            .tag = DY_AST_EXPR_DO_BLOCK,
            .do_block = do_block
        };

        return true;
    }

    if (parse_expr_parenthesized(ctx, expr)) {
        return true;
    }

    if (dy_parse_literal(ctx, DY_STR_LIT("All"))) {
        *expr = (struct dy_ast_expr){
            .text_range = {
                .start = start_index,
                .end = ctx->stream.current_index,
            },
            .tag = DY_AST_EXPR_ALL
        };

        return true;
    }

    if (dy_parse_literal(ctx, DY_STR_LIT("Nothing"))) {
        *expr = (struct dy_ast_expr){
            .text_range = {
                .start = start_index,
                .end = ctx->stream.current_index,
            },
            .tag = DY_AST_EXPR_NOTHING
        };

        return true;
    }

    if (dy_parse_literal(ctx, DY_STR_LIT("String"))) {
        *expr = (struct dy_ast_expr){
            .text_range = {
                .start = start_index,
                .end = ctx->stream.current_index,
            },
            .tag = DY_AST_EXPR_TYPE_STRING
        };

        return true;
    }

    dy_string_t var;
    if (dy_parse_variable(ctx, &var)) {
        *expr = (struct dy_ast_expr){
            .text_range = {
                .start = start_index,
                .end = ctx->stream.current_index,
            },
            .tag = DY_AST_EXPR_VARIABLE,
            .variable = var
        };

        return true;
    }

    dy_string_t string;
    if (dy_parse_string(ctx, &string)) {
        *expr = (struct dy_ast_expr){
            .text_range = {
                .start = start_index,
                .end = ctx->stream.current_index,
            },
            .tag = DY_AST_EXPR_STRING,
            .string = string
        };

        return true;
    }

    return false;
}

bool dy_parse_string(struct dy_parser_ctx *ctx, dy_string_t *string)
{
    size_t start_index = ctx->stream.current_index;

    if (!parse_exactly_one(ctx, '\"')) {
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_array_t *string_storage = dy_array_create(sizeof(char), 8);
    dy_array_add(ctx->string_arrays, &string_storage);

    for (;;) {
        char c;
        if (!get_char(ctx, &c)) {
            ctx->stream.current_index = start_index;
            return false;
        }

        if (c == '\"') {
            *string = (dy_string_t){
                .ptr = dy_array_buffer(string_storage),
                .size = dy_array_size(string_storage)
            };

            return true;
        }

        dy_array_add(string_storage, &c);
    }
}

enum infix_op parse_infix_op(struct dy_parser_ctx *ctx)
{
    if (dy_parse_literal(ctx, DY_STR_LIT("!"))) {
        return INFIX_OP_BANG;
    }

    if (dy_parse_literal(ctx, DY_STR_LIT("->"))) {
        return INFIX_OP_STRAIGHT_ARROW;
    }

    if (dy_parse_literal(ctx, DY_STR_LIT("~>"))) {
        return INFIX_OP_SQUIGGLY_ARROW;
    }

    if (dy_parse_literal(ctx, DY_STR_LIT("@->"))) {
        return INFIX_OP_AT_STRAIGHT_ARROW;
    }

    if (dy_parse_literal(ctx, DY_STR_LIT("@~>"))) {
        return INFIX_OP_AT_SQUIGGLY_ARROW;
    }

    return INFIX_OP_JUXTAPOSITION;
}

bool parse_bare_list(struct dy_parser_ctx *ctx, struct dy_ast_list *list)
{
    size_t start_index = ctx->stream.current_index;

    if (!dy_parse_literal(ctx, DY_STR_LIT("{"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    return parse_bare_list_inner(ctx, list);
}

bool parse_bare_list_inner(struct dy_parser_ctx *ctx, struct dy_ast_list *list)
{
    size_t start_index = ctx->stream.current_index;

    struct dy_ast_expr expr;
    if (!dy_parse_expr(ctx, &expr)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    if (dy_parse_literal(ctx, DY_STR_LIT("}"))) {
        *list = (struct dy_ast_list){
            .expr = dy_ast_expr_new(ctx->pool, expr),
            .next_or_null = NULL
        };

        return true;
    }

    if (dy_parse_literal(ctx, DY_STR_LIT(","))) {
        skip_whitespace(ctx);

        struct dy_ast_list next;
        if (!parse_bare_list_inner(ctx, &next)) {
            dy_ast_expr_release(ctx->pool, expr);
            ctx->stream.current_index = start_index;
            return false;
        }

        *list = (struct dy_ast_list){
            .expr = dy_ast_expr_new(ctx->pool, expr),
            .next_or_null = dy_ast_list_new(ctx->pool, next)
        };

        return true;
    }

    if (dy_parse_literal(ctx, DY_STR_LIT("\n")) || dy_parse_literal(ctx, DY_STR_LIT("\r\n"))) {
        skip_whitespace(ctx);

        if (dy_parse_literal(ctx, DY_STR_LIT("}"))) {
            *list = (struct dy_ast_list){
                .expr = dy_ast_expr_new(ctx->pool, expr),
                .next_or_null = NULL
            };

            return true;
        } else {
            struct dy_ast_list next;
            if (!parse_bare_list_inner(ctx, &next)) {
                dy_ast_expr_release(ctx->pool, expr);
                ctx->stream.current_index = start_index;
                return false;
            }

            *list = (struct dy_ast_list){
                .expr = dy_ast_expr_new(ctx->pool, expr),
                .next_or_null = dy_ast_list_new(ctx->pool, next)
            };

            return true;
        }
    }

    ctx->stream.current_index = start_index;

    return false;
}

bool dy_parse_list(struct dy_parser_ctx *ctx, struct dy_ast_list *list)
{
    size_t start_index = ctx->stream.current_index;

    if (!dy_parse_literal(ctx, DY_STR_LIT("list"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    if (!parse_bare_list(ctx, list)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    return true;
}

bool dy_parse_choice(struct dy_parser_ctx *ctx, struct dy_ast_list *choice)
{
    size_t start_index = ctx->stream.current_index;

    if (!dy_parse_literal(ctx, DY_STR_LIT("choice"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    if (!parse_bare_list(ctx, choice)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    return true;
}

bool dy_parse_try_block(struct dy_parser_ctx *ctx, struct dy_ast_list *try_block)
{
    size_t start_index = ctx->stream.current_index;

    if (!dy_parse_literal(ctx, DY_STR_LIT("try"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    if (!parse_bare_list(ctx, try_block)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    return true;
}

bool dy_parse_do_block(struct dy_parser_ctx *ctx, struct dy_ast_do_block *do_block)
{
    size_t start_index = ctx->stream.current_index;

    if (!dy_parse_literal(ctx, DY_STR_LIT("do"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT("{"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    struct dy_ast_do_block body;
    if (!parse_do_block_body(ctx, &body)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT("}"))) {
        dy_ast_do_block_release(ctx->pool, body);
        ctx->stream.current_index = start_index;
        return false;
    }

    *do_block = body;

    return true;
}

bool dy_parse_file(struct dy_parser_ctx *ctx, struct dy_ast_do_block *do_block)
{
    size_t start_index = ctx->stream.current_index;

    skip_whitespace(ctx);

    struct dy_ast_do_block body;
    if (!parse_do_block_body(ctx, &body)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    *do_block = body;

    return true;
}

bool parse_do_block_body(struct dy_parser_ctx *ctx, struct dy_ast_do_block *do_block)
{
    struct dy_ast_do_block_equality equality;
    if (parse_do_block_body_equality(ctx, &equality)) {
        *do_block = (struct dy_ast_do_block){
            .tag = DY_AST_DO_BLOCK_EQUALITY,
            .equality = equality
        };

        return true;
    }

    struct dy_ast_do_block_let let;
    if (parse_do_block_body_let(ctx, &let)) {
        *do_block = (struct dy_ast_do_block){
            .tag = DY_AST_DO_BLOCK_LET,
            .let = let
        };

        return true;
    }

    struct dy_ast_do_block_ignored_expr ignored_expr;
    if (parse_do_block_body_ignored_expr(ctx, &ignored_expr)) {
        *do_block = (struct dy_ast_do_block){
            .tag = DY_AST_DO_BLOCK_IGNORED_EXPR,
            .ignored_expr = ignored_expr
        };

        return true;
    }

    struct dy_ast_expr end_expr;
    if (dy_parse_expr(ctx, &end_expr)) {
        *do_block = (struct dy_ast_do_block){
            .tag = DY_AST_DO_BLOCK_END_EXPR,
            .end_expr = dy_ast_expr_new(ctx->pool, end_expr)
        };

        return true;
    }

    return false;
}

bool parse_do_block_body_equality(struct dy_parser_ctx *ctx, struct dy_ast_do_block_equality *equality)
{
    size_t start_index = ctx->stream.current_index;

    struct dy_ast_expr e1;
    if (!dy_parse_expr(ctx, &e1)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT("="))) {
        dy_ast_expr_release(ctx->pool, e1);
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    struct dy_ast_expr e2;
    if (!dy_parse_expr(ctx, &e2)) {
        dy_ast_expr_release(ctx->pool, e1);
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    if (!skip_semicolon_or_newline(ctx)) {
        dy_ast_expr_release(ctx->pool, e1);
        dy_ast_expr_release(ctx->pool, e2);
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    struct dy_ast_do_block rest;
    if (!parse_do_block_body(ctx, &rest)) {
        dy_ast_expr_release(ctx->pool, e1);
        dy_ast_expr_release(ctx->pool, e2);
        ctx->stream.current_index = start_index;
        return false;
    }

    *equality = (struct dy_ast_do_block_equality){
        .e1 = dy_ast_expr_new(ctx->pool, e1),
        .e2 = dy_ast_expr_new(ctx->pool, e2),
        .rest = dy_ast_do_block_new(ctx->pool, rest)
    };

    return true;
}

bool parse_do_block_body_let(struct dy_parser_ctx *ctx, struct dy_ast_do_block_let *let)
{
    size_t start_index = ctx->stream.current_index;

    if (!dy_parse_literal(ctx, DY_STR_LIT("let"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    dy_string_t name;
    if (!dy_parse_variable(ctx, &name)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT("="))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    struct dy_ast_expr expr;
    if (!dy_parse_expr(ctx, &expr)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    if (!skip_semicolon_or_newline(ctx)) {
        dy_ast_expr_release(ctx->pool, expr);
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    struct dy_ast_do_block rest;
    if (!parse_do_block_body(ctx, &rest)) {
        dy_ast_expr_release(ctx->pool, expr);
        ctx->stream.current_index = start_index;
        return false;
    }

    *let = (struct dy_ast_do_block_let){
        .arg_name = name,
        .expr = dy_ast_expr_new(ctx->pool, expr),
        .rest = dy_ast_do_block_new(ctx->pool, rest)
    };

    return true;
}

bool parse_do_block_body_ignored_expr(struct dy_parser_ctx *ctx, struct dy_ast_do_block_ignored_expr *ignored_expr)
{
    size_t start_index = ctx->stream.current_index;

    struct dy_ast_expr expr;
    if (!dy_parse_expr(ctx, &expr)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    if (!skip_semicolon_or_newline(ctx)) {
        dy_ast_expr_release(ctx->pool, expr);
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    struct dy_ast_do_block rest;
    if (!parse_do_block_body(ctx, &rest)) {
        dy_ast_expr_release(ctx->pool, expr);
        ctx->stream.current_index = start_index;
        return false;
    }

    *ignored_expr = (struct dy_ast_do_block_ignored_expr){
        .expr = dy_ast_expr_new(ctx->pool, expr),
        .rest = dy_ast_do_block_new(ctx->pool, rest)
    };

    return true;
}

bool skip_semicolon_or_newline(struct dy_parser_ctx *ctx)
{
    if (dy_parse_literal(ctx, DY_STR_LIT(";"))) {
        return true;
    }

    if (dy_parse_literal(ctx, DY_STR_LIT("\n")) || dy_parse_literal(ctx, DY_STR_LIT("\r\n"))) {
        return true;
    }

    return false;
}

bool parse_expr_parenthesized(struct dy_parser_ctx *ctx, struct dy_ast_expr *expr)
{
    size_t start_index = ctx->stream.current_index;

    if (!dy_parse_literal(ctx, DY_STR_LIT("("))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    struct dy_ast_expr e;
    if (!dy_parse_expr(ctx, &e)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT(")"))) {
        dy_ast_expr_release(ctx->pool, e);
        ctx->stream.current_index = start_index;
        return false;
    }

    e.text_range = (struct dy_range){
        .start = start_index,
        .end = ctx->stream.current_index
    };

    *expr = e;

    return true;
}

bool parse_type_map(struct dy_parser_ctx *ctx, dy_string_t arrow_literal, struct dy_ast_type_map *type_map)
{
    size_t start_index = ctx->stream.current_index;

    struct dy_ast_arg arg;
    if (!parse_arg(ctx, &arg)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    if (!dy_parse_literal(ctx, arrow_literal)) {
        if (arg.has_type) {
            dy_ast_expr_release_ptr(ctx->pool, arg.type);
        }
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    struct dy_ast_expr expr;
    if (!dy_parse_expr(ctx, &expr)) {
        if (arg.has_type) {
            dy_ast_expr_release_ptr(ctx->pool, arg.type);
        }
        ctx->stream.current_index = start_index;
        return false;
    }

    *type_map = (struct dy_ast_type_map){
        .arg = arg,
        .expr = dy_ast_expr_new(ctx->pool, expr)
    };

    return true;
}

bool dy_parse_positive_type_map(struct dy_parser_ctx *ctx, struct dy_ast_type_map *type_map)
{
    return parse_type_map(ctx, DY_STR_LIT("->"), type_map);
}

bool dy_parse_negative_type_map(struct dy_parser_ctx *ctx, struct dy_ast_type_map *type_map)
{
    return parse_type_map(ctx, DY_STR_LIT("~>"), type_map);
}

bool parse_arg(struct dy_parser_ctx *ctx, struct dy_ast_arg *arg)
{
    size_t start_index = ctx->stream.current_index;

    if (!dy_parse_literal(ctx, DY_STR_LIT("["))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    if (dy_parse_literal(ctx, DY_STR_LIT("]"))) {
        *arg = (struct dy_ast_arg){
            .has_name = false,
            .has_type = false
        };
        return true;
    }

    bool has_name;
    dy_string_t name;
    if (dy_parse_literal(ctx, DY_STR_LIT("_"))) {
        has_name = false;
    } else {
        has_name = true;

        if (!dy_parse_variable(ctx, &name)) {
            ctx->stream.current_index = start_index;
            return false;
        }
    }

    skip_whitespace(ctx);

    struct dy_ast_expr type;
    bool has_type = dy_parse_expr(ctx, &type);

    skip_whitespace(ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT("]"))) {
        dy_ast_expr_release(ctx->pool, type);
        ctx->stream.current_index = start_index;
        return false;
    }

    *arg = (struct dy_ast_arg){
        .name = name,
        .has_name = has_name,
        .type = dy_ast_expr_new(ctx->pool, type),
        .has_type = has_type
    };

    return true;
}

bool combine_infix(struct dy_parser_ctx *ctx, struct dy_ast_expr left, enum infix_op op, struct dy_ast_expr right, struct dy_ast_expr *expr)
{
    switch (op) {
    case INFIX_OP_BANG:
        if (right.tag == DY_AST_EXPR_NEGATIVE_EXPR_MAP) {
            *expr = (struct dy_ast_expr){
                .text_range = {
                    .start = left.text_range.start,
                    .end = right.text_range.end,
                },
                .tag = DY_AST_EXPR_EXPR_MAP_ELIM,
                .expr_map_elim = {
                    .expr = dy_ast_expr_new(ctx->pool, dy_ast_expr_retain(ctx->pool, left)),
                    .expr_map = {
                        .e1 = dy_ast_expr_retain_ptr(ctx->pool, right.negative_expr_map.e1),
                        .e2 = dy_ast_expr_retain_ptr(ctx->pool, right.negative_expr_map.e2),
                        .is_implicit = right.negative_expr_map.is_implicit,
                    },
                }
            };

            return true;
        }

        if (right.tag == DY_AST_EXPR_NEGATIVE_TYPE_MAP) {
            if (right.negative_type_map.arg.has_type) {
                dy_ast_expr_retain_ptr(ctx->pool, right.negative_type_map.arg.type);
            }

            *expr = (struct dy_ast_expr){
                .text_range = {
                    .start = left.text_range.start,
                    .end = right.text_range.end,
                },
                .tag = DY_AST_EXPR_TYPE_MAP_ELIM,
                .type_map_elim = {
                    .expr = dy_ast_expr_new(ctx->pool, dy_ast_expr_retain(ctx->pool, left)),
                    .type_map = {
                        .arg = right.negative_type_map.arg,
                        .expr = dy_ast_expr_retain_ptr(ctx->pool, right.negative_type_map.expr),
                        .is_implicit = right.negative_type_map.is_implicit,
                    },
                }
            };

            return true;
        }

        return false;
    case INFIX_OP_STRAIGHT_ARROW:
        *expr = (struct dy_ast_expr){
            .text_range = {
                .start = left.text_range.start,
                .end = right.text_range.end,
            },
            .tag = DY_AST_EXPR_POSITIVE_EXPR_MAP,
            .positive_expr_map = {
                .e1 = dy_ast_expr_new(ctx->pool, dy_ast_expr_retain(ctx->pool, left)),
                .e2 = dy_ast_expr_new(ctx->pool, dy_ast_expr_retain(ctx->pool, right)),
                .is_implicit = false,
            }
        };

        return true;
    case INFIX_OP_SQUIGGLY_ARROW:
        *expr = (struct dy_ast_expr){
            .text_range = {
                .start = left.text_range.start,
                .end = right.text_range.end,
            },
            .tag = DY_AST_EXPR_NEGATIVE_EXPR_MAP,
            .negative_expr_map = {
                .e1 = dy_ast_expr_new(ctx->pool, dy_ast_expr_retain(ctx->pool, left)),
                .e2 = dy_ast_expr_new(ctx->pool, dy_ast_expr_retain(ctx->pool, right)),
                .is_implicit = false,
            }
        };

        return true;
    case INFIX_OP_AT_STRAIGHT_ARROW:
        *expr = (struct dy_ast_expr){
            .text_range = {
                .start = left.text_range.start,
                .end = right.text_range.end,
            },
            .tag = DY_AST_EXPR_POSITIVE_EXPR_MAP,
            .positive_expr_map = {
                .e1 = dy_ast_expr_new(ctx->pool, dy_ast_expr_retain(ctx->pool, left)),
                .e2 = dy_ast_expr_new(ctx->pool, dy_ast_expr_retain(ctx->pool, right)),
                .is_implicit = true,
            }
        };

        return true;
    case INFIX_OP_AT_SQUIGGLY_ARROW:
        *expr = (struct dy_ast_expr){
            .text_range = {
                .start = left.text_range.start,
                .end = right.text_range.end,
            },
            .tag = DY_AST_EXPR_NEGATIVE_EXPR_MAP,
            .negative_expr_map = {
                .e1 = dy_ast_expr_new(ctx->pool, dy_ast_expr_retain(ctx->pool, left)),
                .e2 = dy_ast_expr_new(ctx->pool, dy_ast_expr_retain(ctx->pool, right)),
                .is_implicit = true,
            }
        };

        return true;
    case INFIX_OP_JUXTAPOSITION:
        *expr = (struct dy_ast_expr){
            .text_range = {
                .start = left.text_range.start,
                .end = right.text_range.end,
            },
            .tag = DY_AST_EXPR_JUXTAPOSITION,
            .juxtaposition = {
                .left = dy_ast_expr_new(ctx->pool, dy_ast_expr_retain(ctx->pool, left)),
                .right = dy_ast_expr_new(ctx->pool, dy_ast_expr_retain(ctx->pool, right)),
            }
        };

        return true;
    }

    DY_IMPOSSIBLE_ENUM();
}

bool left_op_is_first(enum infix_op left, enum infix_op right)
{
    switch (left) {
    case INFIX_OP_BANG:
        return false;
    case INFIX_OP_JUXTAPOSITION:
        return true;
    case INFIX_OP_STRAIGHT_ARROW:
        // fallthrough
    case INFIX_OP_SQUIGGLY_ARROW:
        // fallthrough
    case INFIX_OP_AT_STRAIGHT_ARROW:
        // fallthrough
    case INFIX_OP_AT_SQUIGGLY_ARROW:
        switch (right) {
        case INFIX_OP_BANG:
            return true;
        case INFIX_OP_JUXTAPOSITION:
            // fallthrough
        case INFIX_OP_STRAIGHT_ARROW:
            // fallthrough
        case INFIX_OP_SQUIGGLY_ARROW:
            // fallthrough
        case INFIX_OP_AT_STRAIGHT_ARROW:
            // fallthrough
        case INFIX_OP_AT_SQUIGGLY_ARROW:
            return false;
        }

        DY_IMPOSSIBLE_ENUM();
    }

    DY_IMPOSSIBLE_ENUM();
}

void skip_whitespace(struct dy_parser_ctx *ctx)
{
    for (;;) {
        char c;
        if (parse_one_of(ctx, DY_STR_LIT(" \r\n\t"), &c)) {
            continue;
        }

        if (skip_block_comment(ctx)) {
            continue;
        }

        if (skip_line_comment(ctx)) {
            continue;
        }

        return;
    }
}

void skip_whitespace_except_newline(struct dy_parser_ctx *ctx)
{
    for (;;) {
        char c;
        if (parse_one_of(ctx, DY_STR_LIT(" \t"), &c)) {
            continue;
        }

        if (skip_block_comment(ctx)) {
            continue;
        }

        if (skip_line_comment(ctx)) {
            continue;
        }

        return;
    }
}

bool parse_one_of(struct dy_parser_ctx *ctx, dy_string_t chars, char *c)
{
    char ch;
    if (!get_char(ctx, &ch)) {
        return false;
    }

    if (!dy_string_matches_one_of(ch, chars)) {
        --ctx->stream.current_index;
        return false;
    }

    *c = ch;

    return true;
}

bool parse_exactly_one(struct dy_parser_ctx *ctx, char c)
{
    char ch;
    if (!get_char(ctx, &ch)) {
        return false;
    }

    if (ch != c) {
        --ctx->stream.current_index;
        return false;
    }

    return true;
}

bool get_char(struct dy_parser_ctx *ctx, char *c)
{
    return dy_stream_get_char(&ctx->stream, c);
}
