/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "ast.h"

#include "../support/array.h"
#include "../support/stream.h"
#include "../support/bail.h"

/**
 * The parser, transforming a text stream into an AST.
 */

struct dy_parser_ctx {
    struct dy_stream stream;
    dy_array_t string_arrays; /** String literals are copied into this array. */
};

enum infix_op {
    INFIX_OP_STRAIGHT_ARROW,
    INFIX_OP_SQUIGGLY_ARROW,
    INFIX_OP_JUXTAPOSITION,
    INFIX_OP_AT_STRAIGHT_ARROW,
    INFIX_OP_AT_SQUIGGLY_ARROW,
    INFIX_OP_BANG
};

static inline bool dy_parse_literal(struct dy_parser_ctx *ctx, dy_string_t s);

static inline bool dy_parse_expr(struct dy_parser_ctx *ctx, struct dy_ast_expr *expr);

static inline bool dy_parse_variable(struct dy_parser_ctx *ctx, struct dy_ast_literal *var);

static inline bool dy_parse_positive_type_map(struct dy_parser_ctx *ctx, struct dy_ast_type_map *type_map);

static inline bool dy_parse_negative_type_map(struct dy_parser_ctx *ctx, struct dy_ast_type_map *type_map);

static inline bool dy_parse_implicit_positive_type_map(struct dy_parser_ctx *ctx, struct dy_ast_type_map *type_map);

static inline bool dy_parse_implicit_negative_type_map(struct dy_parser_ctx *ctx, struct dy_ast_type_map *type_map);

static inline bool dy_parse_positive_recursion(struct dy_parser_ctx *ctx, struct dy_ast_recursion *recursion);

static inline bool dy_parse_negative_recursion(struct dy_parser_ctx *ctx, struct dy_ast_recursion *recursion);

static inline bool dy_parse_do_block(struct dy_parser_ctx *ctx, struct dy_ast_do_block *do_block);

static inline bool dy_parse_file(struct dy_parser_ctx *ctx, struct dy_ast_do_block *do_block);

static inline bool dy_parse_list(struct dy_parser_ctx *ctx, struct dy_ast_list *list);

static inline bool dy_parse_try_block(struct dy_parser_ctx *ctx, struct dy_ast_list *try_block);

static inline bool dy_parse_choice(struct dy_parser_ctx *ctx, struct dy_ast_list *choice);

static inline bool left_op_is_first(enum infix_op left, enum infix_op right);

static inline bool parse_expr_non_left_recursive(struct dy_parser_ctx *ctx, struct dy_ast_expr *expr);

static inline enum infix_op parse_infix_op(struct dy_parser_ctx *ctx);

static inline bool parse_expr_further(struct dy_parser_ctx *ctx, struct dy_ast_expr left, enum infix_op left_op, struct dy_ast_expr *expr);

static inline bool combine_infix(struct dy_ast_expr left, enum infix_op op, struct dy_ast_expr right, struct dy_ast_expr *expr);

static inline bool parse_expr_parenthesized(struct dy_parser_ctx *ctx, struct dy_ast_expr *expr);

static inline bool parse_arg(struct dy_parser_ctx *ctx, struct dy_ast_arg *arg);

static inline void skip_whitespace_except_newline(struct dy_parser_ctx *ctx);

static inline void skip_whitespace(struct dy_parser_ctx *ctx);

static inline bool get_char(struct dy_parser_ctx *ctx, char *c);

static inline bool parse_one_of(struct dy_parser_ctx *ctx, dy_string_t chars, char *c);

static inline bool parse_exactly_one(struct dy_parser_ctx *ctx, char c);

static inline bool parse_bare_list(struct dy_parser_ctx *ctx, size_t start_index, struct dy_ast_list *list);

static inline bool parse_bare_list_inner(struct dy_parser_ctx *ctx, struct dy_ast_list_inner *list);

static inline bool skip_line_comment(struct dy_parser_ctx *ctx);

static inline bool skip_block_comment(struct dy_parser_ctx *ctx);

static inline bool parse_type_map(struct dy_parser_ctx *ctx, dy_string_t arrow_literal, bool is_implicit, struct dy_ast_type_map *type_map);

static inline bool parse_recursion(struct dy_parser_ctx *ctx, dy_string_t arrow_literal, struct dy_ast_recursion *recursion);

static inline bool parse_do_block_body(struct dy_parser_ctx *ctx, struct dy_ast_do_block_body *do_block);

static inline bool parse_do_block_body_equality(struct dy_parser_ctx *ctx, struct dy_ast_do_block_equality *equality);

static inline bool parse_do_block_body_let(struct dy_parser_ctx *ctx, struct dy_ast_do_block_let *let);

static inline bool parse_do_block_body_inverted_let(struct dy_parser_ctx *ctx, struct dy_ast_do_block_let *let);

static inline bool parse_do_block_body_ignored_expr(struct dy_parser_ctx *ctx, struct dy_ast_do_block_ignored_expr *ignored_expr);

static inline bool parse_do_block_body_inverted_ignored_expr(struct dy_parser_ctx *ctx, struct dy_ast_do_block_ignored_expr *ignored_expr);

static inline bool parse_do_block_def(struct dy_parser_ctx *ctx, struct dy_ast_do_block_def *def);

static inline bool skip_semicolon_or_newline(struct dy_parser_ctx *ctx);

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

bool dy_parse_variable(struct dy_parser_ctx *ctx, struct dy_ast_literal *var)
{
    size_t start_index = ctx->stream.current_index;

    dy_array_t var_name = dy_array_create(sizeof(char), DY_ALIGNOF(char), 8);
    dy_array_add(&ctx->string_arrays, &var_name);

    char c;
    if (!parse_one_of(ctx, DY_STR_LIT("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"), &c)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_array_add(&var_name, &c);

    for (;;) {
        if (!parse_one_of(ctx, DY_STR_LIT("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-?"), &c)) {
            dy_string_t final_var = {
                .ptr = var_name.buffer,
                .size = var_name.num_elems
            };

            if (dy_string_are_equal(final_var, DY_STR_LIT("list"))
                || dy_string_are_equal(final_var, DY_STR_LIT("try"))
                || dy_string_are_equal(final_var, DY_STR_LIT("let"))
                || dy_string_are_equal(final_var, DY_STR_LIT("choice"))
                || dy_string_are_equal(final_var, DY_STR_LIT("All"))
                || dy_string_are_equal(final_var, DY_STR_LIT("Any"))
                || dy_string_are_equal(final_var, DY_STR_LIT("prec"))
                || dy_string_are_equal(final_var, DY_STR_LIT("nrec"))
                || dy_string_are_equal(final_var, DY_STR_LIT("Symbol"))
                || dy_string_are_equal(final_var, DY_STR_LIT("def"))
                || dy_string_are_equal(final_var, DY_STR_LIT("invert"))
                || dy_string_are_equal(final_var, DY_STR_LIT("do"))) {
                ctx->stream.current_index = start_index;
                return false;
            }

            *var = (struct dy_ast_literal){
                .text_range = {
                    .start = start_index,
                    .end = ctx->stream.current_index,
                },
                .value = final_var
            };

            return true;
        }

        dy_array_add(&var_name, &c);
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

    dy_ast_expr_release(left);

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
            *expr = dy_ast_expr_retain(left);
            return true;
        }

        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    enum infix_op right_op = parse_infix_op(ctx);

    if (left_op_is_first(left_op, right_op)) {
        struct dy_ast_expr new_left;
        bool result = combine_infix(left, left_op, right, &new_left);

        dy_ast_expr_release(right);

        if (!result) {
            ctx->stream.current_index = start_index;
            return false;
        }

        skip_whitespace_except_newline(ctx);

        result = parse_expr_further(ctx, new_left, right_op, expr);

        dy_ast_expr_release(new_left);

        if (!result) {
            ctx->stream.current_index = start_index;
            return false;
        }

        return true;
    } else {
        skip_whitespace_except_newline(ctx);

        struct dy_ast_expr new_right;
        bool result = parse_expr_further(ctx, right, right_op, &new_right);

        dy_ast_expr_release(right);

        if (!result) {
            ctx->stream.current_index = start_index;
            return false;
        }

        result = combine_infix(left, left_op, new_right, expr);

        dy_ast_expr_release(new_right);

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
            .tag = DY_AST_EXPR_POSITIVE_TYPE_MAP,
            .positive_type_map = positive_type_map
        };

        return true;
    }

    struct dy_ast_type_map negative_type_map;
    if (dy_parse_negative_type_map(ctx, &negative_type_map)) {
        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_NEGATIVE_TYPE_MAP,
            .negative_type_map = negative_type_map
        };

        return true;
    }

    struct dy_ast_type_map implicit_positive_type_map;
    if (dy_parse_implicit_positive_type_map(ctx, &implicit_positive_type_map)) {
        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_POSITIVE_TYPE_MAP,
            .positive_type_map = implicit_positive_type_map
        };

        return true;
    }

    struct dy_ast_type_map implicit_negative_type_map;
    if (dy_parse_implicit_negative_type_map(ctx, &implicit_negative_type_map)) {
        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_NEGATIVE_TYPE_MAP,
            .negative_type_map = implicit_negative_type_map
        };

        return true;
    }

    struct dy_ast_recursion positive_recursion;
    if (dy_parse_positive_recursion(ctx, &positive_recursion)) {
        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_POSITIVE_RECURSION,
            .positive_recursion = positive_recursion
        };

        return true;
    }

    struct dy_ast_recursion negative_recursion;
    if (dy_parse_negative_recursion(ctx, &negative_recursion)) {
        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_NEGATIVE_RECURSION,
            .negative_recursion = negative_recursion
        };

        return true;
    }

    struct dy_ast_list list;
    if (dy_parse_list(ctx, &list)) {
        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_LIST,
            .list = list
        };

        return true;
    }

    struct dy_ast_list choice;
    if (dy_parse_choice(ctx, &choice)) {
        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_CHOICE,
            .choice = choice
        };

        return true;
    }

    struct dy_ast_list try_block;
    if (dy_parse_try_block(ctx, &try_block)) {
        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_TRY_BLOCK,
            .try_block = try_block
        };

        return true;
    }

    struct dy_ast_do_block do_block;
    if (dy_parse_do_block(ctx, &do_block)) {
        *expr = (struct dy_ast_expr){
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
            .static_literal_text_range = {
                .start = start_index,
                .end = ctx->stream.current_index,
            },
            .tag = DY_AST_EXPR_ALL
        };

        return true;
    }

    if (dy_parse_literal(ctx, DY_STR_LIT("Any"))) {
        *expr = (struct dy_ast_expr){
            .static_literal_text_range = {
                .start = start_index,
                .end = ctx->stream.current_index,
            },
            .tag = DY_AST_EXPR_ANY
        };

        return true;
    }

    if (dy_parse_literal(ctx, DY_STR_LIT("Symbol"))) {
        *expr = (struct dy_ast_expr){
            .static_literal_text_range = {
                .start = start_index,
                .end = ctx->stream.current_index,
            },
            .tag = DY_AST_EXPR_SYMBOL
        };

        return true;
    }

    struct dy_ast_literal var;
    if (dy_parse_variable(ctx, &var)) {
        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_VARIABLE,
            .variable = var
        };

        return true;
    }

    return false;
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

bool parse_bare_list(struct dy_parser_ctx *ctx, size_t start, struct dy_ast_list *list)
{
    size_t start_index = ctx->stream.current_index;

    if (!dy_parse_literal(ctx, DY_STR_LIT("{"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    struct dy_ast_list_inner inner;
    if (!parse_bare_list_inner(ctx, &inner)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    *list = (struct dy_ast_list){
        .text_range = {
            .start = start,
            .end = ctx->stream.current_index,
        },
        .inner = inner
    };

    return true;
}

bool parse_bare_list_inner(struct dy_parser_ctx *ctx, struct dy_ast_list_inner *list)
{
    size_t start_index = ctx->stream.current_index;

    struct dy_ast_expr expr;
    if (!dy_parse_expr(ctx, &expr)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    if (dy_parse_literal(ctx, DY_STR_LIT("}"))) {
        *list = (struct dy_ast_list_inner){
            .expr = dy_ast_expr_new(expr),
            .next_or_null = NULL
        };

        return true;
    }

    if (dy_parse_literal(ctx, DY_STR_LIT(","))) {
        skip_whitespace(ctx);

        struct dy_ast_list_inner next;
        if (!parse_bare_list_inner(ctx, &next)) {
            dy_ast_expr_release(expr);
            ctx->stream.current_index = start_index;
            return false;
        }

        *list = (struct dy_ast_list_inner){
            .expr = dy_ast_expr_new(expr),
            .next_or_null = dy_ast_list_new(next)
        };

        return true;
    }

    if (dy_parse_literal(ctx, DY_STR_LIT("\n")) || dy_parse_literal(ctx, DY_STR_LIT("\r\n"))) {
        skip_whitespace(ctx);

        if (dy_parse_literal(ctx, DY_STR_LIT("}"))) {
            *list = (struct dy_ast_list_inner){
                .expr = dy_ast_expr_new(expr),
                .next_or_null = NULL
            };

            return true;
        } else {
            struct dy_ast_list_inner next;
            if (!parse_bare_list_inner(ctx, &next)) {
                dy_ast_expr_release(expr);
                ctx->stream.current_index = start_index;
                return false;
            }

            *list = (struct dy_ast_list_inner){
                .expr = dy_ast_expr_new(expr),
                .next_or_null = dy_ast_list_new(next)
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

    if (!parse_bare_list(ctx, start_index, list)) {
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

    if (!parse_bare_list(ctx, start_index, choice)) {
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

    if (!parse_bare_list(ctx, start_index, try_block)) {
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

    struct dy_ast_do_block_body body;
    if (!parse_do_block_body(ctx, &body)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT("}"))) {
        dy_ast_do_block_release(body);
        ctx->stream.current_index = start_index;
        return false;
    }

    *do_block = (struct dy_ast_do_block){
        .text_range = {
            .start = start_index,
            .end = ctx->stream.current_index,
        },
        .body = body
    };

    return true;
}

bool dy_parse_file(struct dy_parser_ctx *ctx, struct dy_ast_do_block *do_block)
{
    size_t start_index = ctx->stream.current_index;

    skip_whitespace(ctx);

    struct dy_ast_do_block_body body;
    if (!parse_do_block_body(ctx, &body)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    *do_block = (struct dy_ast_do_block){
        .text_range = *(struct dy_range *)&body,
        .body = body
    };

    return true;
}

bool parse_do_block_body(struct dy_parser_ctx *ctx, struct dy_ast_do_block_body *do_block)
{
    struct dy_ast_do_block_equality equality;
    if (parse_do_block_body_equality(ctx, &equality)) {
        *do_block = (struct dy_ast_do_block_body){
            .tag = DY_AST_DO_BLOCK_EQUALITY,
            .equality = equality
        };

        return true;
    }

    struct dy_ast_do_block_let let;
    if (parse_do_block_body_let(ctx, &let)) {
        *do_block = (struct dy_ast_do_block_body){
            .tag = DY_AST_DO_BLOCK_LET,
            .let = let
        };

        return true;
    }

    struct dy_ast_do_block_let inverted_let;
    if (parse_do_block_body_inverted_let(ctx, &inverted_let)) {
        *do_block = (struct dy_ast_do_block_body){
            .tag = DY_AST_DO_BLOCK_INVERTED_LET,
            .inverted_let = inverted_let
        };

        return true;
    }

    struct dy_ast_do_block_ignored_expr inverted_ignored_expr;
    if (parse_do_block_body_inverted_ignored_expr(ctx, &inverted_ignored_expr)) {
        *do_block = (struct dy_ast_do_block_body){
            .tag = DY_AST_DO_BLOCK_INVERTED_IGNORED_EXPR,
            .ignored_expr = inverted_ignored_expr
        };

        return true;
    }

    struct dy_ast_do_block_def def;
    if (parse_do_block_def(ctx, &def)) {
        *do_block = (struct dy_ast_do_block_body){
            .tag = DY_AST_DO_BLOCK_DEF,
            .def = def
        };

        return true;
    }

    struct dy_ast_do_block_ignored_expr ignored_expr;
    if (parse_do_block_body_ignored_expr(ctx, &ignored_expr)) {
        *do_block = (struct dy_ast_do_block_body){
            .tag = DY_AST_DO_BLOCK_IGNORED_EXPR,
            .ignored_expr = ignored_expr
        };

        return true;
    }

    struct dy_ast_expr end_expr;
    if (dy_parse_expr(ctx, &end_expr)) {
        *do_block = (struct dy_ast_do_block_body){
            .tag = DY_AST_DO_BLOCK_END_EXPR,
            .end_expr = dy_ast_expr_new(end_expr)
        };

        return true;
    }

    return false;
}

bool parse_do_block_def(struct dy_parser_ctx *ctx, struct dy_ast_do_block_def *def)
{
    size_t start_index = ctx->stream.current_index;

    if (!dy_parse_literal(ctx, DY_STR_LIT("def"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    struct dy_ast_literal name;
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
        dy_ast_expr_release(expr);
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    struct dy_ast_do_block_body rest;
    if (!parse_do_block_body(ctx, &rest)) {
        dy_ast_expr_release(expr);
        ctx->stream.current_index = start_index;
        return false;
    }

    *def = (struct dy_ast_do_block_def){
        .name = name,
        .expr = dy_ast_expr_new(expr),
        .rest = dy_ast_do_block_new(rest)
    };

    return true;
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
        dy_ast_expr_release(e1);
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    struct dy_ast_expr e2;
    if (!dy_parse_expr(ctx, &e2)) {
        dy_ast_expr_release(e1);
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    if (!skip_semicolon_or_newline(ctx)) {
        dy_ast_expr_release(e1);
        dy_ast_expr_release(e2);
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    struct dy_ast_do_block_body rest;
    if (!parse_do_block_body(ctx, &rest)) {
        dy_ast_expr_release(e1);
        dy_ast_expr_release(e2);
        ctx->stream.current_index = start_index;
        return false;
    }

    *equality = (struct dy_ast_do_block_equality){
        .text_range = {
            .start = start_index,
            .end = ctx->stream.current_index,
        },
        .e1 = dy_ast_expr_new(e1),
        .e2 = dy_ast_expr_new(e2),
        .rest = dy_ast_do_block_new(rest)
    };

    return true;
}

bool parse_do_block_body_inverted_let(struct dy_parser_ctx *ctx, struct dy_ast_do_block_let *let)
{
    size_t start_index = ctx->stream.current_index;

    if (!dy_parse_literal(ctx, DY_STR_LIT("invert"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    if (!parse_do_block_body_let(ctx, let)) {
        ctx->stream.current_index = start_index;
        return false;
    }

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

    struct dy_ast_literal name;
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
        dy_ast_expr_release(expr);
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    struct dy_ast_do_block_body rest;
    if (!parse_do_block_body(ctx, &rest)) {
        dy_ast_expr_release(expr);
        ctx->stream.current_index = start_index;
        return false;
    }

    *let = (struct dy_ast_do_block_let){
        .text_range = {
            .start = start_index,
            .end = ctx->stream.current_index,
        },
        .arg_name = name,
        .expr = dy_ast_expr_new(expr),
        .rest = dy_ast_do_block_new(rest)
    };

    return true;
}

bool parse_do_block_body_inverted_ignored_expr(struct dy_parser_ctx *ctx, struct dy_ast_do_block_ignored_expr *ignored_expr)
{
    size_t start_index = ctx->stream.current_index;

    if (!dy_parse_literal(ctx, DY_STR_LIT("invert"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    if (!parse_do_block_body_ignored_expr(ctx, ignored_expr)) {
        ctx->stream.current_index = start_index;
        return false;
    }

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
        dy_ast_expr_release(expr);
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    struct dy_ast_do_block_body rest;
    if (!parse_do_block_body(ctx, &rest)) {
        dy_ast_expr_release(expr);
        ctx->stream.current_index = start_index;
        return false;
    }

    *ignored_expr = (struct dy_ast_do_block_ignored_expr){
        .text_range = {
            .start = start_index,
            .end = ctx->stream.current_index,
        },
        .expr = dy_ast_expr_new(expr),
        .rest = dy_ast_do_block_new(rest)
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
        dy_ast_expr_release(e);
        ctx->stream.current_index = start_index;
        return false;
    }

    struct dy_range *text_range = (struct dy_range *)&e;
    *text_range = (struct dy_range){
        .start = start_index,
        .end = ctx->stream.current_index
    };

    *expr = e;

    return true;
}

bool parse_recursion(struct dy_parser_ctx *ctx, dy_string_t rec_lit, struct dy_ast_recursion *recursion)
{
    size_t start_index = ctx->stream.current_index;

    if (!dy_parse_literal(ctx, rec_lit)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    struct dy_ast_literal self_name;
    if (!dy_parse_variable(ctx, &self_name)) {
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

    *recursion = (struct dy_ast_recursion){
        .text_range = {
            .start = start_index,
            .end = ctx->stream.current_index,
        },
        .name = self_name,
        .expr = dy_ast_expr_new(expr)
    };

    return true;
}

bool dy_parse_positive_recursion(struct dy_parser_ctx *ctx, struct dy_ast_recursion *recursion)
{
    return parse_recursion(ctx, DY_STR_LIT("prec"), recursion);
}

bool dy_parse_negative_recursion(struct dy_parser_ctx *ctx, struct dy_ast_recursion *recursion)
{
    return parse_recursion(ctx, DY_STR_LIT("nrec"), recursion);
}

bool parse_type_map(struct dy_parser_ctx *ctx, dy_string_t arrow_literal, bool is_implicit, struct dy_ast_type_map *type_map)
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
            dy_ast_expr_release_ptr(arg.type);
        }
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    struct dy_ast_expr expr;
    if (!dy_parse_expr(ctx, &expr)) {
        if (arg.has_type) {
            dy_ast_expr_release_ptr(arg.type);
        }
        ctx->stream.current_index = start_index;
        return false;
    }

    *type_map = (struct dy_ast_type_map){
        .text_range = {
            .start = start_index,
            .end = ctx->stream.current_index,
        },
        .arg = arg,
        .expr = dy_ast_expr_new(expr),
        .is_implicit = is_implicit
    };

    return true;
}

bool dy_parse_positive_type_map(struct dy_parser_ctx *ctx, struct dy_ast_type_map *type_map)
{
    return parse_type_map(ctx, DY_STR_LIT("->"), false, type_map);
}

bool dy_parse_negative_type_map(struct dy_parser_ctx *ctx, struct dy_ast_type_map *type_map)
{
    return parse_type_map(ctx, DY_STR_LIT("~>"), false, type_map);
}

bool dy_parse_implicit_positive_type_map(struct dy_parser_ctx *ctx, struct dy_ast_type_map *type_map)
{
    return parse_type_map(ctx, DY_STR_LIT("@->"), true, type_map);
}

bool dy_parse_implicit_negative_type_map(struct dy_parser_ctx *ctx, struct dy_ast_type_map *type_map)
{
    return parse_type_map(ctx, DY_STR_LIT("@~>"), true, type_map);
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
    struct dy_ast_literal name;
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
        if (has_type) {
            dy_ast_expr_release(type);
        }
        ctx->stream.current_index = start_index;
        return false;
    }

    *arg = (struct dy_ast_arg){
        .name = name,
        .has_name = has_name,
        .type = has_type ? dy_ast_expr_new(type) : NULL,
        .has_type = has_type
    };

    return true;
}

bool combine_infix(struct dy_ast_expr left, enum infix_op op, struct dy_ast_expr right, struct dy_ast_expr *expr)
{
    switch (op) {
    case INFIX_OP_BANG:
        if (right.tag == DY_AST_EXPR_NEGATIVE_EQUALITY_MAP) {
            *expr = (struct dy_ast_expr){
                .tag = DY_AST_EXPR_EQUALITY_MAP_ELIM,
                .equality_map_elim = {
                    .text_range = {
                        .start = ((struct dy_range *)&left)->start,
                        .end = ((struct dy_range *)&right)->end,
                    },
                    .expr = dy_ast_expr_new(dy_ast_expr_retain(left)),
                    .equality_map = {
                        .e1 = dy_ast_expr_retain_ptr(right.negative_equality_map.e1),
                        .e2 = dy_ast_expr_retain_ptr(right.negative_equality_map.e2),
                        .is_implicit = right.negative_equality_map.is_implicit,
                    },
                }
            };

            return true;
        }

        if (right.tag == DY_AST_EXPR_NEGATIVE_TYPE_MAP) {
            if (right.negative_type_map.arg.has_type) {
                dy_ast_expr_retain_ptr(right.negative_type_map.arg.type);
            }

            *expr = (struct dy_ast_expr){
                .tag = DY_AST_EXPR_TYPE_MAP_ELIM,
                .type_map_elim = {
                    .text_range = {
                        .start = ((struct dy_range *)&left)->start,
                        .end = ((struct dy_range *)&right)->end,
                    },
                    .expr = dy_ast_expr_new(dy_ast_expr_retain(left)),
                    .type_map = {
                        .arg = right.negative_type_map.arg,
                        .expr = dy_ast_expr_retain_ptr(right.negative_type_map.expr),
                        .is_implicit = right.negative_type_map.is_implicit,
                    },
                }
            };

            return true;
        }

        return false;
    case INFIX_OP_STRAIGHT_ARROW:
        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_POSITIVE_EQUALITY_MAP,
            .positive_equality_map = {
                .text_range = {
                    .start = ((struct dy_range *)&left)->start,
                    .end = ((struct dy_range *)&right)->end,
                },
                .e1 = dy_ast_expr_new(dy_ast_expr_retain(left)),
                .e2 = dy_ast_expr_new(dy_ast_expr_retain(right)),
                .is_implicit = false,
            }
        };

        return true;
    case INFIX_OP_SQUIGGLY_ARROW:
        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_NEGATIVE_EQUALITY_MAP,
            .negative_equality_map = {
                .text_range = {
                    .start = ((struct dy_range *)&left)->start,
                    .end = ((struct dy_range *)&right)->end,
                },
                .e1 = dy_ast_expr_new(dy_ast_expr_retain(left)),
                .e2 = dy_ast_expr_new(dy_ast_expr_retain(right)),
                .is_implicit = false,
            }
        };

        return true;
    case INFIX_OP_AT_STRAIGHT_ARROW:
        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_POSITIVE_EQUALITY_MAP,
            .positive_equality_map = {
                .text_range = {
                    .start = ((struct dy_range *)&left)->start,
                    .end = ((struct dy_range *)&right)->end,
                },
                .e1 = dy_ast_expr_new(dy_ast_expr_retain(left)),
                .e2 = dy_ast_expr_new(dy_ast_expr_retain(right)),
                .is_implicit = true,
            }
        };

        return true;
    case INFIX_OP_AT_SQUIGGLY_ARROW:
        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_NEGATIVE_EQUALITY_MAP,
            .negative_equality_map = {
                .text_range = {
                    .start = ((struct dy_range *)&left)->start,
                    .end = ((struct dy_range *)&right)->end,
                },
                .e1 = dy_ast_expr_new(dy_ast_expr_retain(left)),
                .e2 = dy_ast_expr_new(dy_ast_expr_retain(right)),
                .is_implicit = true,
            }
        };

        return true;
    case INFIX_OP_JUXTAPOSITION:
        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_JUXTAPOSITION,
            .juxtaposition = {
                .text_range = {
                    .start = ((struct dy_range *)&left)->start,
                    .end = ((struct dy_range *)&right)->end,
                },
                .left = dy_ast_expr_new(dy_ast_expr_retain(left)),
                .right = dy_ast_expr_new(dy_ast_expr_retain(right)),
            }
        };

        return true;
    }

    dy_bail("Impossible infix op.");
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

        dy_bail("Impossible infix op.");
    }

    dy_bail("Impossible infix op.");
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
