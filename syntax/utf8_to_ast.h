/*
 * Copyright 2021 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "ast.h"

#include "../support/stream.h"
#include "../support/bail.h"

struct dy_utf8_to_ast_ctx {
    struct dy_stream stream;
};

enum dy_infix_op {
    DY_INFIX_OP_STRAIGHT_ARROW,
    DY_INFIX_OP_SQUIGGLY_ARROW,
    DY_INFIX_OP_AT,
    DY_INFIX_OP_AT_STRAIGHT_ARROW,
    DY_INFIX_OP_AT_SQUIGGLY_ARROW,
    DY_INFIX_OP_NOTHING,
};

static inline bool dy_utf8_literal(struct dy_utf8_to_ast_ctx *ctx, dy_string_t s);

static inline bool dy_utf8_to_size_t(struct dy_utf8_to_ast_ctx *ctx, size_t *nat);

static inline bool dy_utf8_to_ast_expr(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_expr *expr);

static inline bool dy_utf8_to_ast_variable(struct dy_utf8_to_ast_ctx *ctx, dy_array_t *var);

static inline bool dy_utf8_to_ast_do_block(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_do_block *do_block);

static inline bool dy_utf8_to_ast_file(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_do_block *file);

static inline bool dy_utf8_to_ast_list(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_list *list);

static inline bool dy_left_op_is_first(enum dy_infix_op left, enum dy_infix_op right);

static inline bool dy_utf8_to_non_left_recursive_ast_expr(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_expr *expr);

static inline bool dy_utf8_to_argument(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_argument *arg);

static inline enum dy_infix_op dy_utf8_to_ast_infix_op(struct dy_utf8_to_ast_ctx *ctx);

static inline bool dy_utf8_to_ast_expr_further(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_argument left, enum dy_infix_op left_op, struct dy_ast_expr *expr);

static inline bool dy_combine_infix(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_argument left, enum dy_infix_op op, struct dy_ast_argument right, struct dy_ast_expr *expr);

static inline bool dy_utf8_to_parenthesized_ast_expr(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_expr *expr);

static inline bool dy_utf8_to_ast_pattern(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_pattern *pattern);

static inline bool dy_utf8_to_ast_binding(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_binding *binding);

static inline bool dy_utf8_to_ast_pattern_simple(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_pattern_simple *solution);

static inline bool dy_utf8_to_ast_pattern_list(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_pattern_list *list);

static inline bool dy_utf8_to_ast_pattern_list_body(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_pattern_list_body *list_body);

static inline void dy_skip_whitespace_except_newline(struct dy_utf8_to_ast_ctx *ctx);

static inline void dy_skip_whitespace(struct dy_utf8_to_ast_ctx *ctx);

static inline bool dy_get_char(struct dy_utf8_to_ast_ctx *ctx, char *c);

static inline bool dy_utf8_one_of(struct dy_utf8_to_ast_ctx *ctx, dy_string_t chars, char *c);

static inline bool dy_utf8_exactly_one(struct dy_utf8_to_ast_ctx *ctx, char c);

static inline bool dy_utf8_to_ast_list_body(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_list_body *list_body);

static inline bool dy_skip_line_comment(struct dy_utf8_to_ast_ctx *ctx);

static inline bool dy_skip_block_comment(struct dy_utf8_to_ast_ctx *ctx);

static inline bool dy_utf8_to_ast_function(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_function *function);

static inline bool dy_utf8_to_ast_recursion(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_recursion *recursion);

static inline bool dy_utf8_to_ast_do_block_body(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_do_block *do_block);

static inline bool dy_utf8_to_ast_do_block_body_rest(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_do_block *do_block);

static inline bool dy_utf8_to_ast_do_block_stmnt(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_do_block_stmnt *stmnt);

static inline bool dy_utf8_to_ast_do_block_stmnt_let(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_do_block_stmnt_let *let);

static inline bool dy_utf8_to_ast_do_block_stmnt_expr(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_do_block_stmnt_expr *expr);

static inline bool dy_utf8_to_ast_do_block_stmnt_def(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_do_block_stmnt_def *def);

static inline bool dy_skip_semicolon_or_newline(struct dy_utf8_to_ast_ctx *ctx);

static inline bool dy_utf8_to_ast_string(struct dy_utf8_to_ast_ctx *ctx, dy_array_t *string);

static inline bool dy_utf8_to_ast_binding_with_type(struct dy_utf8_to_ast_ctx *ctx, dy_array_t name, bool have_name, struct dy_ast_binding *binding);

static inline bool dy_utf8_to_ast_binding_with_pattern(struct dy_utf8_to_ast_ctx *ctx, dy_array_t name, bool have_name, struct dy_ast_binding *binding);

static inline bool dy_utf8_to_ast_index(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_argument_index *index);

static inline bool dy_utf8_to_map_some(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_map_some *map_some);

static inline bool dy_utf8_to_map_either(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_map_either *map_either);

static inline bool dy_utf8_to_map_either_body(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_map_either_body *body);

static inline bool dy_utf8_to_map_fin(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_map_fin *map_fin);

static inline bool dy_utf8_to_binding_and_expr(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_binding *binding, struct dy_ast_expr *expr);

bool dy_utf8_literal(struct dy_utf8_to_ast_ctx *ctx, dy_string_t s)
{
    size_t start_index = ctx->stream.current_index;

    for (size_t i = 0; i < s.size; ++i) {
        if (!dy_utf8_exactly_one(ctx, s.ptr[i])) {
            ctx->stream.current_index = start_index;
            return false;
        }
    }

    return true;
}

bool dy_utf8_to_ast_variable(struct dy_utf8_to_ast_ctx *ctx, dy_array_t *var)
{
    size_t start_index = ctx->stream.current_index;

    char c;
    if (!dy_utf8_one_of(ctx, DY_STR_LIT("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"), &c)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_array_add(var, &c);

    for (;;) {
        if (!dy_utf8_one_of(ctx, DY_STR_LIT("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-?"), &c)) {
            dy_string_t final_var = {
                .ptr = var->buffer,
                .size = var->num_elems
            };

            if (dy_string_are_equal(final_var, DY_STR_LIT("list"))
                || dy_string_are_equal(final_var, DY_STR_LIT("let"))
                || dy_string_are_equal(final_var, DY_STR_LIT("either"))
                || dy_string_are_equal(final_var, DY_STR_LIT("Void"))
                || dy_string_are_equal(final_var, DY_STR_LIT("Any"))
                || dy_string_are_equal(final_var, DY_STR_LIT("String"))
                || dy_string_are_equal(final_var, DY_STR_LIT("Unwrap"))
                || dy_string_are_equal(final_var, DY_STR_LIT("Unfold"))
                || dy_string_are_equal(final_var, DY_STR_LIT("max"))
                || dy_string_are_equal(final_var, DY_STR_LIT("inf"))
                || dy_string_are_equal(final_var, DY_STR_LIT("fin"))
                || dy_string_are_equal(final_var, DY_STR_LIT("fun"))
                || dy_string_are_equal(final_var, DY_STR_LIT("def"))
                || dy_string_are_equal(final_var, DY_STR_LIT("inv"))
                || dy_string_are_equal(final_var, DY_STR_LIT("some"))
                || dy_string_are_equal(final_var, DY_STR_LIT("map"))
                || dy_string_are_equal(final_var, DY_STR_LIT("do"))) {
                ctx->stream.current_index = start_index;
                return false;
            }

            return true;
        }

        dy_array_add(var, &c);
    }
}

bool dy_utf8_to_size_t(struct dy_utf8_to_ast_ctx *ctx, size_t *nat)
{
    size_t start_index = ctx->stream.current_index;

    size_t ret;
    if (!dy_stream_parse_size_t_decimal(&ctx->stream, &ret)) {
        return false;
    }

    if (ret == 0) {
        ctx->stream.current_index = start_index;
        return false;
    } else {
        *nat = ret;
        return true;
    }
}

bool dy_utf8_to_ast_index(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_argument_index *index)
{
    size_t start_index = ctx->stream.current_index;

    size_t value;
    if (!dy_utf8_to_size_t(ctx, &value)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    start_index = ctx->stream.current_index;

    dy_skip_whitespace(ctx);

    if (dy_utf8_literal(ctx, DY_STR_LIT("max"))) {
        *index = (struct dy_ast_argument_index){
            .value = value,
            .is_maximum = true
        };

        return true;
    } else {
        *index = (struct dy_ast_argument_index){
            .value = value,
            .is_maximum = false
        };

        ctx->stream.current_index = start_index;
        return true;
    }
}

bool dy_skip_line_comment(struct dy_utf8_to_ast_ctx *ctx)
{
    size_t start_index = ctx->stream.current_index;

    if (!dy_utf8_literal(ctx, DY_STR_LIT("#"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    for (;;) {
        char c;
        if (!dy_get_char(ctx, &c)) {
            return true;
        }

        if (c == '\n') {
            return true;
        }
    }
}

bool dy_skip_block_comment(struct dy_utf8_to_ast_ctx *ctx)
{
    size_t start_index = ctx->stream.current_index;

    if (!dy_utf8_literal(ctx, DY_STR_LIT("/#"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    for (;;) {
        if (dy_skip_block_comment(ctx)) {
            continue;
        }

        if (dy_utf8_literal(ctx, DY_STR_LIT("#/"))) {
            return true;
        }

        char c;
        if (dy_get_char(ctx, &c)) {
            continue;
        }

        ctx->stream.current_index = start_index;

        return false;
    }
}

bool dy_utf8_to_ast_expr(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_expr *expr)
{
    size_t start_index = ctx->stream.current_index;

    struct dy_ast_argument left;
    if (!dy_utf8_to_argument(ctx, &left)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace_except_newline(ctx);

    enum dy_infix_op op = dy_utf8_to_ast_infix_op(ctx);

    dy_skip_whitespace_except_newline(ctx);

    if (!dy_utf8_to_ast_expr_further(ctx, left, op, expr)) {
        dy_ast_argument_release(left);
        ctx->stream.current_index = start_index;
        return false;
    }

    return true;
}

bool dy_utf8_to_ast_expr_further(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_argument left, enum dy_infix_op left_op, struct dy_ast_expr *expr)
{
    size_t start_index = ctx->stream.current_index;

    struct dy_ast_argument right;
    if (!dy_utf8_to_argument(ctx, &right)) {
        if (left.tag == DY_AST_ARGUMENT_EXPR && left_op == DY_INFIX_OP_NOTHING) {
            *expr = dy_ast_expr_retain(*left.expr);
            return true;
        }

        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace_except_newline(ctx);

    if (left.tag == DY_AST_ARGUMENT_EXPR && (left_op == DY_INFIX_OP_NOTHING || left_op == DY_INFIX_OP_AT) && dy_utf8_literal(ctx, DY_STR_LIT(":"))) {
        dy_skip_whitespace_except_newline(ctx);

        struct dy_ast_expr type;
        if (!dy_utf8_to_ast_expr(ctx, &type)) {
            dy_ast_argument_release(right);
            ctx->stream.current_index = start_index;
            return false;
        }

        struct dy_ast_expr e = {
            .tag = DY_AST_EXPR_JUXTAPOSITION,
            .juxtaposition = {
                .left = dy_ast_expr_retain_ptr(left.expr),
                .right = right,
                .type = dy_ast_expr_new(type),
                .is_implicit = left_op == DY_INFIX_OP_AT
            }
        };

        struct dy_ast_argument arg_wrap = {
            .tag = DY_AST_ARGUMENT_EXPR,
            .expr = dy_ast_expr_new(e)
        };

        enum dy_infix_op next_op = dy_utf8_to_ast_infix_op(ctx);

        bool ret = dy_utf8_to_ast_expr_further(ctx, arg_wrap, next_op, expr);

        dy_ast_argument_release(arg_wrap);

        if (!ret) {
            ctx->stream.current_index = start_index;
            return false;
        }

        return true;
    }

    enum dy_infix_op right_op = dy_utf8_to_ast_infix_op(ctx);

    if (dy_left_op_is_first(left_op, right_op)) {
        struct dy_ast_expr new_left;
        bool ret = dy_combine_infix(ctx, left, left_op, right, &new_left);

        dy_ast_argument_release(right);

        if (!ret) {
            ctx->stream.current_index = start_index;
            return false;
        }

        dy_skip_whitespace_except_newline(ctx);

        struct dy_ast_argument arg_wrap = {
            .tag = DY_AST_ARGUMENT_EXPR,
            .expr = dy_ast_expr_new(new_left)
        };

        ret = dy_utf8_to_ast_expr_further(ctx, arg_wrap, right_op, expr);

        dy_ast_argument_release(arg_wrap);

        if (!ret) {
            ctx->stream.current_index = start_index;
            return false;
        }

        return true;
    } else {
        dy_skip_whitespace_except_newline(ctx);

        struct dy_ast_expr new_right;
        bool ret = dy_utf8_to_ast_expr_further(ctx, right, right_op, &new_right);

        if (!ret && right_op == DY_INFIX_OP_NOTHING) {
            ret = dy_combine_infix(ctx, left, left_op, right, expr);
            dy_ast_argument_release(right);
            return ret;
        }

        dy_ast_argument_release(right);

        if (!ret) {
            ctx->stream.current_index = start_index;
            return false;
        }

        struct dy_ast_argument arg_wrap = {
            .tag = DY_AST_ARGUMENT_EXPR,
            .expr = dy_ast_expr_new(new_right)
        };

        ret = dy_combine_infix(ctx, left, left_op, arg_wrap, expr);

        dy_ast_argument_release(arg_wrap);

        if (!ret) {
            ctx->stream.current_index = start_index;
            return false;
        }

        return true;
    }
}

bool dy_utf8_to_argument(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_argument *arg)
{
    struct dy_ast_expr expr;
    if (dy_utf8_to_non_left_recursive_ast_expr(ctx, &expr)) {
        *arg = (struct dy_ast_argument){
            .tag = DY_AST_ARGUMENT_EXPR,
            .expr = dy_ast_expr_new(expr)
        };

        return true;
    }

    struct dy_ast_argument_index index;
    if (dy_utf8_to_ast_index(ctx, &index)) {
        *arg = (struct dy_ast_argument){
            .tag = DY_AST_ARGUMENT_INDEX,
            .index = index
        };

        return true;
    }

    if (dy_utf8_literal(ctx, DY_STR_LIT("Unfold"))) {
        *arg = (struct dy_ast_argument){
            .tag = DY_AST_ARGUMENT_UNFOLD
        };

        return true;
    }

    if (dy_utf8_literal(ctx, DY_STR_LIT("Unwrap"))) {
        *arg = (struct dy_ast_argument){
            .tag = DY_AST_ARGUMENT_UNWRAP
        };

        return true;
    }

    return false;
}

bool dy_utf8_to_non_left_recursive_ast_expr(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_expr *expr)
{
    struct dy_ast_function fun;
    if (dy_utf8_to_ast_function(ctx, &fun)) {
        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_FUNCTION,
            .function = fun
        };

        return true;
    }

    struct dy_ast_recursion rec;
    if (dy_utf8_to_ast_recursion(ctx, &rec)) {
        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_RECURSION,
            .recursion = rec
        };

        return true;
    }

    struct dy_ast_list list;
    if (dy_utf8_to_ast_list(ctx, &list)) {
        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_LIST,
            .list = list
        };

        return true;
    }

    struct dy_ast_do_block do_block;
    if (dy_utf8_to_ast_do_block(ctx, &do_block)) {
        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_DO_BLOCK,
            .do_block = do_block
        };

        return true;
    }

    dy_array_t string = dy_array_create(sizeof(char), DY_ALIGNOF(char), 8);
    if (dy_utf8_to_ast_string(ctx, &string)) {
        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_STRING,
            .string = string
        };

        return true;
    }

    if (dy_utf8_to_parenthesized_ast_expr(ctx, expr)) {
        return true;
    }

    if (dy_utf8_literal(ctx, DY_STR_LIT("Any"))) {
        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_ANY
        };

        return true;
    }

    if (dy_utf8_literal(ctx, DY_STR_LIT("Void"))) {
        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_VOID
        };

        return true;
    }

    if (dy_utf8_literal(ctx, DY_STR_LIT("String"))) {
        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_STRING_TYPE
        };

        return true;
    }

    dy_array_t var = dy_array_create(sizeof(char), DY_ALIGNOF(char), 8);
    if (dy_utf8_to_ast_variable(ctx, &var)) {
        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_VARIABLE,
            .variable = var
        };

        return true;
    }

    struct dy_ast_map_some map_some;
    if (dy_utf8_to_map_some(ctx, &map_some)) {
        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_MAP_SOME,
            .map_some = map_some
        };

        return true;
    }

    struct dy_ast_map_either map_either;
    if (dy_utf8_to_map_either(ctx, &map_either)) {
        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_MAP_EITHER,
            .map_either = map_either
        };

        return true;
    }

    struct dy_ast_map_fin map_fin;
    if (dy_utf8_to_map_fin(ctx, &map_fin)) {
        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_MAP_FIN,
            .map_fin = map_fin
        };

        return true;
    }

    return false;
}

bool dy_utf8_to_map_some(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_map_some *map_some)
{
    size_t start_index = ctx->stream.current_index;

    if (!dy_utf8_literal(ctx, DY_STR_LIT("map"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace(ctx);

    if (!dy_utf8_literal(ctx, DY_STR_LIT("some"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace(ctx);

    bool is_implicit = dy_utf8_literal(ctx, DY_STR_LIT("@"));

    dy_skip_whitespace(ctx);

    dy_array_t name = dy_array_create(sizeof(char), DY_ALIGNOF(char), 8);
    if (!dy_utf8_to_ast_variable(ctx, &name)) {
        dy_array_release(&name);
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace(ctx);

    struct dy_ast_expr type;
    bool have_type;
    if (dy_utf8_literal(ctx, DY_STR_LIT(":"))) {
        have_type = true;

        dy_skip_whitespace(ctx);

        if (!dy_utf8_to_ast_expr(ctx, &type)) {
            dy_array_release(&name);
            ctx->stream.current_index = start_index;
            return false;
        }
    } else {
        have_type = false;
    }

    dy_skip_whitespace(ctx);

    if (!dy_utf8_literal(ctx, DY_STR_LIT("=>"))) {
        dy_array_release(&name);

        if (have_type) {
            dy_ast_expr_release(type);
        }

        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace(ctx);

    struct dy_ast_binding binding;
    struct dy_ast_expr expr;
    if (!dy_utf8_to_binding_and_expr(ctx, &binding, &expr)) {
        dy_array_release(&name);
        if (have_type) {
            dy_ast_expr_release(type);
        }
        ctx->stream.current_index = start_index;
        return false;
    }

    *map_some = (struct dy_ast_map_some){
        .name = name,
        .is_implicit = is_implicit,
        .type = have_type ? dy_ast_expr_new(type) : NULL,
        .binding = binding,
        .expr = dy_ast_expr_new(expr)
    };

    return true;
}

bool dy_utf8_to_map_either(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_map_either *map_either)
{
    size_t start_index = ctx->stream.current_index;

    if (!dy_utf8_literal(ctx, DY_STR_LIT("map"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace(ctx);

    if (!dy_utf8_literal(ctx, DY_STR_LIT("either"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace(ctx);

    bool is_implicit = dy_utf8_literal(ctx, DY_STR_LIT("@"));

    dy_skip_whitespace(ctx);

    if (!dy_utf8_literal(ctx, DY_STR_LIT("{"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace(ctx);

    struct dy_ast_map_either_body body;
    if (!dy_utf8_to_map_either_body(ctx, &body)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    *map_either = (struct dy_ast_map_either){
        .body = body,
        .is_implicit = is_implicit
    };

    return true;
}

bool dy_utf8_to_map_either_body(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_map_either_body *body)
{
    size_t start_index = ctx->stream.current_index;

    struct dy_ast_binding binding;
    struct dy_ast_expr expr;
    if (!dy_utf8_to_binding_and_expr(ctx, &binding, &expr)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace_except_newline(ctx);

    if (dy_utf8_literal(ctx, DY_STR_LIT("}"))) {
        *body = (struct dy_ast_map_either_body){
            .binding = binding,
            .expr = dy_ast_expr_new(expr),
            .next = NULL
        };
        return true;
    }

    if (dy_utf8_literal(ctx, DY_STR_LIT(","))) {
        dy_skip_whitespace(ctx);

        struct dy_ast_map_either_body next;
        if (!dy_utf8_to_map_either_body(ctx, &next)) {
            dy_ast_binding_release(binding);
            dy_ast_expr_release(expr);
            ctx->stream.current_index = start_index;
            return false;
        }

        *body = (struct dy_ast_map_either_body){
            .binding = binding,
            .expr = dy_ast_expr_new(expr),
            .next = dy_ast_map_either_body_new(next)
        };

        return true;
    }

    if (dy_utf8_literal(ctx, DY_STR_LIT("\n")) || dy_utf8_literal(ctx, DY_STR_LIT("\r\n"))) {
        dy_skip_whitespace(ctx);

        if (dy_utf8_literal(ctx, DY_STR_LIT("}"))) {
            *body = (struct dy_ast_map_either_body){
                .binding = binding,
                .expr = dy_ast_expr_new(expr),
                .next = NULL
            };
            return true;
        } else {
            struct dy_ast_map_either_body next;
            if (!dy_utf8_to_map_either_body(ctx, &next)) {
                dy_ast_binding_release(binding);
                dy_ast_expr_release(expr);
                ctx->stream.current_index = start_index;
                return false;
            }

            *body = (struct dy_ast_map_either_body){
                .binding = binding,
                .expr = dy_ast_expr_new(expr),
                .next = dy_ast_map_either_body_new(next)
            };

            return true;
        }
    }

    dy_ast_binding_release(binding);
    dy_ast_expr_release(expr);

    ctx->stream.current_index = start_index;

    return false;
}

bool dy_utf8_to_map_fin(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_map_fin *map_fin)
{
    size_t start_index = ctx->stream.current_index;

    if (!dy_utf8_literal(ctx, DY_STR_LIT("map"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace(ctx);

    if (!dy_utf8_literal(ctx, DY_STR_LIT("fin"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace(ctx);

    bool is_implicit = dy_utf8_literal(ctx, DY_STR_LIT("@"));

    dy_skip_whitespace(ctx);

    dy_array_t name = dy_array_create(sizeof(char), DY_ALIGNOF(char), 8);
    if (!dy_utf8_to_ast_variable(ctx, &name)) {
        dy_array_release(&name);
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace(ctx);

    struct dy_ast_binding binding;
    struct dy_ast_expr expr;
    if (!dy_utf8_to_binding_and_expr(ctx, &binding, &expr)) {
        dy_array_release(&name);
        ctx->stream.current_index = start_index;
        return false;
    }

    *map_fin = (struct dy_ast_map_fin){
        .name = name,
        .is_implicit = is_implicit,
        .binding = binding,
        .expr = dy_ast_expr_new(expr)
    };

    return true;
}

bool dy_utf8_to_binding_and_expr(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_binding *binding, struct dy_ast_expr *expr)
{
    size_t start_index = ctx->stream.current_index;

    struct dy_ast_binding b;
    if (!dy_utf8_to_ast_binding(ctx, &b)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace(ctx);

    if (!dy_utf8_literal(ctx, DY_STR_LIT("=>"))) {
        dy_ast_binding_release(b);
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace(ctx);

    struct dy_ast_expr e;
    if (!dy_utf8_to_ast_expr(ctx, &e)) {
        dy_ast_binding_release(b);
        ctx->stream.current_index = start_index;
        return false;
    }

    *binding = b;
    *expr = e;

    return true;
}

bool dy_utf8_to_ast_string(struct dy_utf8_to_ast_ctx *ctx, dy_array_t *string)
{
    size_t start_index = ctx->stream.current_index;

    if (!dy_utf8_literal(ctx, DY_STR_LIT("'"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    for (;;) {
        char c;
        if (!dy_get_char(ctx, &c)) {
            ctx->stream.current_index = start_index;
            return false;
        }

        if (c == '\'') {
            break;
        }

        dy_array_add(string, &c);
    }

    return true;
}

enum dy_infix_op dy_utf8_to_ast_infix_op(struct dy_utf8_to_ast_ctx *ctx)
{
    if (dy_utf8_literal(ctx, DY_STR_LIT("->"))) {
        return DY_INFIX_OP_STRAIGHT_ARROW;
    }

    if (dy_utf8_literal(ctx, DY_STR_LIT("~>"))) {
        return DY_INFIX_OP_SQUIGGLY_ARROW;
    }

    if (dy_utf8_literal(ctx, DY_STR_LIT("@"))) {
        dy_skip_whitespace_except_newline(ctx);

        if (dy_utf8_literal(ctx, DY_STR_LIT("->"))) {
            return DY_INFIX_OP_AT_STRAIGHT_ARROW;
        } else if (dy_utf8_literal(ctx, DY_STR_LIT("~>"))) {
            return DY_INFIX_OP_AT_SQUIGGLY_ARROW;
        } else {
            return DY_INFIX_OP_AT;
        }
    }

    return DY_INFIX_OP_NOTHING;
}

bool dy_utf8_to_ast_list_body(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_list_body *list_body)
{
    size_t start_index = ctx->stream.current_index;

    struct dy_ast_expr expr;
    if (!dy_utf8_to_ast_expr(ctx, &expr)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace_except_newline(ctx);

    if (dy_utf8_literal(ctx, DY_STR_LIT("}"))) {
        *list_body = (struct dy_ast_list_body){
            .expr = dy_ast_expr_new(expr),
            .next = NULL
        };
        return true;
    }

    if (dy_utf8_literal(ctx, DY_STR_LIT(","))) {
        dy_skip_whitespace(ctx);

        struct dy_ast_list_body next;
        if (!dy_utf8_to_ast_list_body(ctx, &next)) {
            dy_ast_expr_release(expr);
            ctx->stream.current_index = start_index;
            return false;
        }

        *list_body = (struct dy_ast_list_body){
            .expr = dy_ast_expr_new(expr),
            .next = dy_ast_list_body_new(next)
        };

        return true;
    }

    if (dy_utf8_literal(ctx, DY_STR_LIT("\n")) || dy_utf8_literal(ctx, DY_STR_LIT("\r\n"))) {
        dy_skip_whitespace(ctx);

        if (dy_utf8_literal(ctx, DY_STR_LIT("}"))) {
            *list_body = (struct dy_ast_list_body){
                .expr = dy_ast_expr_new(expr),
                .next = NULL
            };
            return true;
        } else {
            struct dy_ast_list_body next;
            if (!dy_utf8_to_ast_list_body(ctx, &next)) {
                dy_ast_expr_release(expr);
                ctx->stream.current_index = start_index;
                return false;
            }

            *list_body = (struct dy_ast_list_body){
                .expr = dy_ast_expr_new(expr),
                .next = dy_ast_list_body_new(next)
            };

            return true;
        }
    }

    dy_ast_expr_release(expr);

    ctx->stream.current_index = start_index;

    return false;
}

bool dy_utf8_to_ast_list(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_list *list)
{
    size_t start_index = ctx->stream.current_index;

    bool is_either;
    if (dy_utf8_literal(ctx, DY_STR_LIT("list"))) {
        is_either = false;
    } else if (dy_utf8_literal(ctx, DY_STR_LIT("either"))) {
        is_either = true;
    } else {
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace(ctx);

    bool is_implicit = dy_utf8_literal(ctx, DY_STR_LIT("@"));

    dy_skip_whitespace(ctx);

    if (!dy_utf8_literal(ctx, DY_STR_LIT("{"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace(ctx);

    struct dy_ast_list_body list_body;
    if (!dy_utf8_to_ast_list_body(ctx, &list_body)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    *list = (struct dy_ast_list){
        .body = list_body,
        .is_either = is_either,
        .is_implicit = is_implicit
    };

    return true;
}

bool dy_utf8_to_ast_do_block(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_do_block *do_block)
{
    size_t start_index = ctx->stream.current_index;

    if (!dy_utf8_literal(ctx, DY_STR_LIT("do"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace(ctx);

    if (!dy_utf8_literal(ctx, DY_STR_LIT("{"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace(ctx);

    struct dy_ast_do_block body;
    if (!dy_utf8_to_ast_do_block_body(ctx, &body)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace(ctx);

    if (!dy_utf8_literal(ctx, DY_STR_LIT("}"))) {
        dy_ast_do_block_release(body);
        ctx->stream.current_index = start_index;
        return false;
    }

    *do_block = body;

    return true;
}

bool dy_utf8_to_ast_file(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_do_block *do_block)
{
    size_t start_index = ctx->stream.current_index;

    dy_skip_whitespace(ctx);

    struct dy_ast_do_block body;
    if (!dy_utf8_to_ast_do_block_body(ctx, &body)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace(ctx);

    *do_block = body;

    return true;
}

bool dy_utf8_to_ast_do_block_stmnt(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_do_block_stmnt *stmnt)
{
    struct dy_ast_do_block_stmnt_let let;
    if (dy_utf8_to_ast_do_block_stmnt_let(ctx, &let)) {
        *stmnt = (struct dy_ast_do_block_stmnt){
            .tag = DY_AST_DO_BLOCK_STMNT_LET,
            .let = let
        };
        return true;
    }

    struct dy_ast_do_block_stmnt_def def;
    if (dy_utf8_to_ast_do_block_stmnt_def(ctx, &def)) {
        *stmnt = (struct dy_ast_do_block_stmnt){
            .tag = DY_AST_DO_BLOCK_STMNT_DEF,
            .def = def
        };
        return true;
    }

    struct dy_ast_do_block_stmnt_expr expr;
    if (dy_utf8_to_ast_do_block_stmnt_expr(ctx, &expr)) {
        *stmnt = (struct dy_ast_do_block_stmnt){
            .tag = DY_AST_DO_BLOCK_STMNT_EXPR,
            .expr = expr
        };
        return true;
    }

    return false;
}

bool dy_utf8_to_ast_do_block_body(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_do_block *do_block)
{
    size_t start_index = ctx->stream.current_index;

    struct dy_ast_do_block_stmnt stmnt;
    if (!dy_utf8_to_ast_do_block_stmnt(ctx, &stmnt)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace_except_newline(ctx);

    struct dy_ast_do_block rest;
    if (dy_utf8_to_ast_do_block_body_rest(ctx, &rest)) {
        *do_block = (struct dy_ast_do_block){
            .stmnt = stmnt,
            .rest = dy_ast_do_block_new(rest)
        };

        return true;
    } else {
        if (stmnt.tag != DY_AST_DO_BLOCK_STMNT_EXPR || stmnt.expr.is_inverted) {
            dy_ast_do_block_stmnt_release(stmnt);
            ctx->stream.current_index = start_index;
            return false;
        }

        *do_block = (struct dy_ast_do_block){
            .stmnt = stmnt,
            .rest = NULL
        };

        return true;
    }
}

bool dy_utf8_to_ast_do_block_body_rest(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_do_block *do_block)
{
    if (!dy_skip_semicolon_or_newline(ctx)) {
        return false;
    }

    dy_skip_whitespace(ctx);

    return dy_utf8_to_ast_do_block_body(ctx, do_block);
}

bool dy_utf8_to_ast_do_block_stmnt_def(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_do_block_stmnt_def *def)
{
    size_t start_index = ctx->stream.current_index;

    if (!dy_utf8_literal(ctx, DY_STR_LIT("def"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace(ctx);

    dy_array_t name = dy_array_create(sizeof(char), DY_ALIGNOF(char), 8);
    if (!dy_utf8_to_ast_variable(ctx, &name)) {
        dy_array_release(&name);
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace(ctx);

    if (!dy_utf8_literal(ctx, DY_STR_LIT("="))) {
        dy_array_release(&name);
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace(ctx);

    struct dy_ast_expr expr;
    if (!dy_utf8_to_ast_expr(ctx, &expr)) {
        dy_array_release(&name);
        ctx->stream.current_index = start_index;
        return false;
    }

    *def = (struct dy_ast_do_block_stmnt_def){
        .name = name,
        .expr = dy_ast_expr_new(expr)
    };

    return true;
}

bool dy_utf8_to_ast_do_block_stmnt_let(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_do_block_stmnt_let *let)
{
    size_t start_index = ctx->stream.current_index;

    if (!dy_utf8_literal(ctx, DY_STR_LIT("let"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace(ctx);

    bool is_inverted = dy_utf8_literal(ctx, DY_STR_LIT("inv"));

    dy_skip_whitespace(ctx);

    struct dy_ast_binding binding;
    if (!dy_utf8_to_ast_binding(ctx, &binding)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace(ctx);

    if (!dy_utf8_literal(ctx, DY_STR_LIT("="))) {
        dy_ast_binding_release(binding);
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace(ctx);

    struct dy_ast_expr expr;
    if (!dy_utf8_to_ast_expr(ctx, &expr)) {
        dy_ast_binding_release(binding);
        ctx->stream.current_index = start_index;
        return false;
    }

    *let = (struct dy_ast_do_block_stmnt_let){
        .binding = binding,
        .expr = dy_ast_expr_new(expr),
        .is_inverted = is_inverted
    };

    return true;
}

bool dy_utf8_to_ast_do_block_stmnt_expr(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_do_block_stmnt_expr *expr)
{
    size_t start_index = ctx->stream.current_index;

    bool is_inverted = dy_utf8_literal(ctx, DY_STR_LIT("inv"));

    dy_skip_whitespace(ctx);

    struct dy_ast_expr e;
    if (!dy_utf8_to_ast_expr(ctx, &e)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    *expr = (struct dy_ast_do_block_stmnt_expr){
        .expr = dy_ast_expr_new(e),
        .is_inverted = is_inverted
    };

    return true;
}

bool dy_skip_semicolon_or_newline(struct dy_utf8_to_ast_ctx *ctx)
{
    if (dy_utf8_literal(ctx, DY_STR_LIT(";"))) {
        return true;
    }

    if (dy_utf8_literal(ctx, DY_STR_LIT("\n")) || dy_utf8_literal(ctx, DY_STR_LIT("\r\n"))) {
        return true;
    }

    return false;
}

bool dy_utf8_to_parenthesized_ast_expr(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_expr *expr)
{
    size_t start_index = ctx->stream.current_index;

    if (!dy_utf8_literal(ctx, DY_STR_LIT("("))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace(ctx);

    struct dy_ast_expr e;
    if (!dy_utf8_to_ast_expr(ctx, &e)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace(ctx);

    if (!dy_utf8_literal(ctx, DY_STR_LIT(")"))) {
        dy_ast_expr_release(e);
        ctx->stream.current_index = start_index;
        return false;
    }

    *expr = e;

    return true;
}

bool dy_utf8_to_ast_recursion(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_recursion *recursion)
{
    size_t start_index = ctx->stream.current_index;

    bool is_fin;
    if (dy_utf8_literal(ctx, DY_STR_LIT("inf"))) {
        is_fin = false;
    } else if (dy_utf8_literal(ctx, DY_STR_LIT("fin"))) {
        is_fin = true;
    } else {
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace(ctx);

    bool is_implicit = dy_utf8_literal(ctx, DY_STR_LIT("@"));

    dy_skip_whitespace(ctx);

    dy_array_t name = dy_array_create(1, 1, 8);
    if (!dy_utf8_to_ast_variable(ctx, &name)) {
        dy_array_release(&name);
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace(ctx);

    if (!dy_utf8_literal(ctx, DY_STR_LIT("="))) {
        dy_array_release(&name);
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace(ctx);

    struct dy_ast_expr expr;
    if (!dy_utf8_to_ast_expr(ctx, &expr)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    *recursion = (struct dy_ast_recursion){
        .name = name,
        .expr = dy_ast_expr_new(expr),
        .is_fin = is_fin,
        .is_implicit = is_implicit
    };

    return true;
}

bool dy_utf8_to_ast_function(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_function *function)
{
    size_t start_index = ctx->stream.current_index;

    bool is_some = false;
    if (dy_utf8_literal(ctx, DY_STR_LIT("fun"))) {
        is_some = false;
    } else if (dy_utf8_literal(ctx, DY_STR_LIT("some"))) {
        is_some = true;
    } else {
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace(ctx);

    bool is_implicit = dy_utf8_literal(ctx, DY_STR_LIT("@"));

    dy_skip_whitespace(ctx);

    struct dy_ast_binding binding;
    if (!dy_utf8_to_ast_binding(ctx, &binding)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace(ctx);

    if (!dy_utf8_literal(ctx, DY_STR_LIT("=>"))) {
        dy_ast_binding_release(binding);
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace(ctx);

    struct dy_ast_expr expr;
    if (!dy_utf8_to_ast_expr(ctx, &expr)) {
        dy_ast_binding_release(binding);
        ctx->stream.current_index = start_index;
        return false;
    }

    *function = (struct dy_ast_function){
        .binding = binding,
        .expr = dy_ast_expr_new(expr),
        .is_implicit = is_implicit,
        .is_some = is_some
    };

    return true;
}

bool dy_utf8_to_ast_binding(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_binding *binding)
{
    struct dy_ast_pattern pattern;
    if (dy_utf8_to_ast_pattern(ctx, &pattern)) {
        *binding = (struct dy_ast_binding){
            .have_name = false,
            .tag = DY_AST_BINDING_PATTERN,
            .pattern = pattern
        };

        return true;
    }

    dy_array_t name = dy_array_create(sizeof(char), DY_ALIGNOF(char), 4);
    bool have_name;

    if (dy_utf8_to_ast_variable(ctx, &name)) {
        have_name = true;
    } else if (dy_utf8_literal(ctx, DY_STR_LIT("_"))) {
        have_name = false;
    } else {
        return false;
    }

    dy_skip_whitespace(ctx);

    if (dy_utf8_to_ast_binding_with_type(ctx, name, have_name, binding)) {
        return true;
    } else if (dy_utf8_to_ast_binding_with_pattern(ctx, name, have_name, binding)) {
        return true;
    } else {
        *binding = (struct dy_ast_binding){
            .name = name,
            .have_name = have_name,
            .tag = DY_AST_BINDING_NOTHING
        };

        return true;
    }
}

bool dy_utf8_to_ast_binding_with_type(struct dy_utf8_to_ast_ctx *ctx, dy_array_t name, bool have_name, struct dy_ast_binding *binding)
{
    size_t start_index = ctx->stream.current_index;

    if (!dy_utf8_literal(ctx, DY_STR_LIT(":"))) {
        return false;
    }

    dy_skip_whitespace(ctx);

    struct dy_ast_expr type;
    if (!dy_utf8_to_ast_expr(ctx, &type)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    *binding = (struct dy_ast_binding){
        .name = name,
        .have_name = have_name,
        .tag = DY_AST_BINDING_TYPE,
        .type = dy_ast_expr_new(type)
    };

    return true;
}

bool dy_utf8_to_ast_binding_with_pattern(struct dy_utf8_to_ast_ctx *ctx, dy_array_t name, bool have_name, struct dy_ast_binding *binding)
{
    size_t start_index = ctx->stream.current_index;

    if (!dy_utf8_literal(ctx, DY_STR_LIT("="))) {
        return false;
    }

    dy_skip_whitespace(ctx);

    struct dy_ast_pattern pattern;
    if (!dy_utf8_to_ast_pattern(ctx, &pattern)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    *binding = (struct dy_ast_binding){
        .name = name,
        .have_name = have_name,
        .tag = DY_AST_BINDING_PATTERN,
        .pattern = pattern
    };

    return true;
}

bool dy_utf8_to_ast_pattern(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_pattern *pattern)
{
    struct dy_ast_pattern_simple simple;
    if (dy_utf8_to_ast_pattern_simple(ctx, &simple)) {
        *pattern = (struct dy_ast_pattern){
            .tag = DY_AST_PATTERN_SIMPLE,
            .simple = simple
        };

        return true;
    }

    struct dy_ast_pattern_list list;
    if (dy_utf8_to_ast_pattern_list(ctx, &list)) {
        *pattern = (struct dy_ast_pattern){
            .tag = DY_AST_PATTERN_LIST,
            .list = list
        };

        return true;
    }

    return false;
}

bool dy_utf8_to_ast_pattern_simple(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_pattern_simple *simple)
{
    size_t start_index = ctx->stream.current_index;

    struct dy_ast_argument arg;
    if (!dy_utf8_to_argument(ctx, &arg)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace(ctx);

    bool is_implicit = dy_utf8_literal(ctx, DY_STR_LIT("@"));

    dy_skip_whitespace(ctx);

    if (!dy_utf8_literal(ctx, DY_STR_LIT("~>"))) {
        dy_ast_argument_release(arg);
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace(ctx);

    struct dy_ast_binding binding;
    if (!dy_utf8_to_ast_binding(ctx, &binding)) {
        dy_ast_argument_release(arg);
        ctx->stream.current_index = start_index;
        return false;
    }

    *simple = (struct dy_ast_pattern_simple){
        .arg = arg,
        .binding = dy_ast_binding_new(binding),
        .is_implicit = is_implicit
    };

    return true;
}

bool dy_utf8_to_ast_pattern_list(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_pattern_list *list)
{
    size_t start_index = ctx->stream.current_index;

    bool is_implicit = dy_utf8_literal(ctx, DY_STR_LIT("@"));

    dy_skip_whitespace(ctx);

    if (!dy_utf8_literal(ctx, DY_STR_LIT("{"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    struct dy_ast_pattern_list_body body;
    if (!dy_utf8_to_ast_pattern_list_body(ctx, &body)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    *list = (struct dy_ast_pattern_list){
        .body = body,
        .is_implicit = is_implicit
    };

    return true;
}

bool dy_utf8_to_ast_pattern_list_body(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_pattern_list_body *list_body)
{
    size_t start_index = ctx->stream.current_index;

    struct dy_ast_binding binding;
    if (!dy_utf8_to_ast_binding(ctx, &binding)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_skip_whitespace_except_newline(ctx);

    if (dy_utf8_literal(ctx, DY_STR_LIT("}"))) {
        *list_body = (struct dy_ast_pattern_list_body){
            .binding = dy_ast_binding_new(binding),
            .next = NULL
        };
        return true;
    }

    if (dy_utf8_literal(ctx, DY_STR_LIT(","))) {
        dy_skip_whitespace(ctx);

        struct dy_ast_pattern_list_body next;
        if (!dy_utf8_to_ast_pattern_list_body(ctx, &next)) {
            dy_ast_binding_release(binding);
            ctx->stream.current_index = start_index;
            return false;
        }

        *list_body = (struct dy_ast_pattern_list_body){
            .binding = dy_ast_binding_new(binding),
            .next = dy_ast_pattern_list_body_new(next)
        };

        return true;
    }

    if (dy_utf8_literal(ctx, DY_STR_LIT("\n")) || dy_utf8_literal(ctx, DY_STR_LIT("\r\n"))) {
        dy_skip_whitespace(ctx);

        if (dy_utf8_literal(ctx, DY_STR_LIT("}"))) {
            *list_body = (struct dy_ast_pattern_list_body){
                .binding = dy_ast_binding_new(binding),
                .next = NULL
            };
            return true;
        } else {
            struct dy_ast_pattern_list_body next;
            if (!dy_utf8_to_ast_pattern_list_body(ctx, &next)) {
                dy_ast_binding_release(binding);
                ctx->stream.current_index = start_index;
                return false;
            }

            *list_body = (struct dy_ast_pattern_list_body){
                .binding = dy_ast_binding_new(binding),
                .next = dy_ast_pattern_list_body_new(next)
            };

            return true;
        }
    }

    ctx->stream.current_index = start_index;
    return false;
}

bool dy_combine_infix(struct dy_utf8_to_ast_ctx *ctx, struct dy_ast_argument left, enum dy_infix_op op, struct dy_ast_argument right, struct dy_ast_expr *expr)
{
    switch (op) {
    case DY_INFIX_OP_STRAIGHT_ARROW:
    case DY_INFIX_OP_SQUIGGLY_ARROW:
    case DY_INFIX_OP_AT_STRAIGHT_ARROW:
    case DY_INFIX_OP_AT_SQUIGGLY_ARROW:
        if (right.tag != DY_AST_ARGUMENT_EXPR) {
            return false;
        }

        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_SIMPLE,
            .simple = {
                .argument = dy_ast_argument_retain(left),
                .expr = dy_ast_expr_retain_ptr(right.expr),
                .is_implicit = op == DY_INFIX_OP_AT_STRAIGHT_ARROW || op == DY_INFIX_OP_AT_SQUIGGLY_ARROW,
                .is_negative = op == DY_INFIX_OP_SQUIGGLY_ARROW || op == DY_INFIX_OP_AT_SQUIGGLY_ARROW
            }
        };

        return true;
    case DY_INFIX_OP_NOTHING:
    case DY_INFIX_OP_AT:
        if (left.tag != DY_AST_ARGUMENT_EXPR) {
            return false;
        }

        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_JUXTAPOSITION,
            .juxtaposition = {
                .left = dy_ast_expr_retain_ptr(left.expr),
                .right = dy_ast_argument_retain(right),
                .type = NULL,
                .is_implicit = op == DY_INFIX_OP_AT
            }
        };

        return true;
    }

    dy_bail("Impossible infix op.");
}

bool dy_left_op_is_first(enum dy_infix_op left, enum dy_infix_op right)
{
    switch (left) {
    case DY_INFIX_OP_NOTHING:
    case DY_INFIX_OP_AT:
        return true;
    case DY_INFIX_OP_STRAIGHT_ARROW:
    case DY_INFIX_OP_SQUIGGLY_ARROW:
    case DY_INFIX_OP_AT_STRAIGHT_ARROW:
    case DY_INFIX_OP_AT_SQUIGGLY_ARROW:
        return false;
    }

    dy_bail("impossible");
}

void dy_skip_whitespace(struct dy_utf8_to_ast_ctx *ctx)
{
    for (;;) {
        char c;
        if (dy_utf8_one_of(ctx, DY_STR_LIT(" \r\n\t"), &c)) {
            continue;
        }

        if (dy_skip_block_comment(ctx)) {
            continue;
        }

        if (dy_skip_line_comment(ctx)) {
            continue;
        }

        return;
    }
}

void dy_skip_whitespace_except_newline(struct dy_utf8_to_ast_ctx *ctx)
{
    for (;;) {
        char c;
        if (dy_utf8_one_of(ctx, DY_STR_LIT(" \t"), &c)) {
            continue;
        }

        if (dy_skip_block_comment(ctx)) {
            continue;
        }

        if (dy_skip_line_comment(ctx)) {
            continue;
        }

        return;
    }
}

bool dy_utf8_one_of(struct dy_utf8_to_ast_ctx *ctx, dy_string_t chars, char *c)
{
    char ch;
    if (!dy_get_char(ctx, &ch)) {
        return false;
    }

    if (!dy_string_matches_one_of(ch, chars)) {
        --ctx->stream.current_index;
        return false;
    }

    *c = ch;

    return true;
}

bool dy_utf8_exactly_one(struct dy_utf8_to_ast_ctx *ctx, char c)
{
    char ch;
    if (!dy_get_char(ctx, &ch)) {
        return false;
    }

    if (ch != c) {
        --ctx->stream.current_index;
        return false;
    }

    return true;
}

bool dy_get_char(struct dy_utf8_to_ast_ctx *ctx, char *c)
{
    return dy_stream_get_char(&ctx->stream, c);
}
