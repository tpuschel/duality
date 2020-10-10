/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "../core/core.h"

#include "../support/array.h"
#include "../support/stream.h"
#include "../support/range.h"

#include "unbound_variable.h"
#include "def.h"
#include "string.h"

/**
 * The parser, transforming a text stream into Core.
 */

/** Represents a variable in the source text and its replacement id for Core. */
struct dy_bound_var {
    dy_string_t name;
    size_t replacement_id;
};

struct dy_text_source {
    size_t id;
    struct dy_range text_range;
};

struct dy_parser_ctx {
    struct dy_stream stream;
    struct dy_core_ctx core_ctx;
    dy_array_t bound_vars;
    dy_array_t text_sources;
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

static inline bool dy_parse_expr(struct dy_parser_ctx *ctx, struct dy_core_expr *expr);

static inline bool dy_parse_variable(struct dy_parser_ctx *ctx, dy_array_t *var);

static inline bool dy_parse_positive_type_map(struct dy_parser_ctx *ctx, struct dy_core_expr *type_map);

static inline bool dy_parse_negative_type_map(struct dy_parser_ctx *ctx, struct dy_core_expr *type_map);

static inline bool dy_parse_implicit_positive_type_map(struct dy_parser_ctx *ctx, struct dy_core_expr *type_map);

static inline bool dy_parse_implicit_negative_type_map(struct dy_parser_ctx *ctx, struct dy_core_expr *type_map);

static inline bool dy_parse_positive_recursion(struct dy_parser_ctx *ctx, struct dy_core_recursion *recursion);

static inline bool dy_parse_negative_recursion(struct dy_parser_ctx *ctx, struct dy_core_recursion *recursion);

static inline bool dy_parse_do_block(struct dy_parser_ctx *ctx, struct dy_core_expr *do_block);

static inline bool dy_parse_file(struct dy_parser_ctx *ctx, struct dy_core_expr *do_block);

static inline bool dy_parse_list(struct dy_parser_ctx *ctx, struct dy_core_expr *list);

static inline bool dy_parse_try_block(struct dy_parser_ctx *ctx, struct dy_core_expr *try_block);

static inline bool dy_parse_choice(struct dy_parser_ctx *ctx, struct dy_core_expr *choice);

static inline bool left_op_is_first(enum infix_op left, enum infix_op right);

static inline bool parse_expr_non_left_recursive(struct dy_parser_ctx *ctx, struct dy_core_expr *expr);

static inline enum infix_op parse_infix_op(struct dy_parser_ctx *ctx);

static inline bool parse_expr_further(struct dy_parser_ctx *ctx, struct dy_core_expr left, enum infix_op left_op, struct dy_core_expr *expr);

static inline bool combine_infix(struct dy_parser_ctx *ctx, struct dy_core_expr left, enum infix_op op, struct dy_core_expr right, struct dy_core_expr *expr);

static inline bool parse_expr_parenthesized(struct dy_parser_ctx *ctx, struct dy_core_expr *expr);

static inline bool parse_arg(struct dy_parser_ctx *ctx, dy_array_t *name, bool *have_name, struct dy_core_expr *expr, bool *have_expr);

static inline void skip_whitespace_except_newline(struct dy_parser_ctx *ctx);

static inline void skip_whitespace(struct dy_parser_ctx *ctx);

static inline bool get_char(struct dy_parser_ctx *ctx, char *c);

static inline bool parse_one_of(struct dy_parser_ctx *ctx, dy_string_t chars, char *c);

static inline bool parse_exactly_one(struct dy_parser_ctx *ctx, char c);

static inline bool parse_bare_list(struct dy_parser_ctx *ctx, bool is_junction, enum dy_core_polarity polarity, struct dy_core_expr *list);

static inline bool skip_line_comment(struct dy_parser_ctx *ctx);

static inline bool skip_block_comment(struct dy_parser_ctx *ctx);

static inline bool parse_type_map(struct dy_parser_ctx *ctx, dy_string_t arrow_literal, bool is_implicit, struct dy_core_expr *type_map);

static inline bool parse_recursion(struct dy_parser_ctx *ctx, dy_string_t arrow_literal, struct dy_core_recursion *recursion);

static inline bool parse_do_block_body(struct dy_parser_ctx *ctx, struct dy_core_expr *do_block);

static inline bool parse_do_block_body_equality(struct dy_parser_ctx *ctx, struct dy_core_expr *equality);

static inline bool parse_do_block_body_let(struct dy_parser_ctx *ctx, struct dy_core_expr *let);

static inline bool parse_do_block_body_inverted_let(struct dy_parser_ctx *ctx, struct dy_core_expr *let);

static inline bool parse_do_block_body_ignored_expr(struct dy_parser_ctx *ctx, struct dy_core_expr *ignored_expr);

static inline bool parse_do_block_body_inverted_ignored_expr(struct dy_parser_ctx *ctx, struct dy_core_expr *ignored_expr);

static inline bool parse_do_block_def(struct dy_parser_ctx *ctx, struct dy_core_expr *def);

static inline bool skip_semicolon_or_newline(struct dy_parser_ctx *ctx);

static inline bool dy_parse_string(struct dy_parser_ctx *ctx, dy_array_t *string);

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

bool dy_parse_variable(struct dy_parser_ctx *ctx, dy_array_t *var)
{
    size_t start_index = ctx->stream.current_index;

    char c;
    if (!parse_one_of(ctx, DY_STR_LIT("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"), &c)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    dy_array_add(var, &c);

    for (;;) {
        if (!parse_one_of(ctx, DY_STR_LIT("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-?"), &c)) {
            dy_string_t final_var = {
                .ptr = var->buffer,
                .size = var->num_elems
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

            return true;
        }

        dy_array_add(var, &c);
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

bool dy_parse_expr(struct dy_parser_ctx *ctx, struct dy_core_expr *expr)
{
    size_t start_index = ctx->stream.current_index;

    struct dy_core_expr left;
    if (!parse_expr_non_left_recursive(ctx, &left)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    enum infix_op op = parse_infix_op(ctx);

    skip_whitespace_except_newline(ctx);

    if (!parse_expr_further(ctx, left, op, expr)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    return true;
}

bool parse_expr_further(struct dy_parser_ctx *ctx, struct dy_core_expr left, enum infix_op left_op, struct dy_core_expr *expr)
{
    size_t start_index = ctx->stream.current_index;

    struct dy_core_expr right;
    if (!parse_expr_non_left_recursive(ctx, &right)) {
        if (left_op == INFIX_OP_JUXTAPOSITION) {
            *expr = left;
            return true;
        }

        dy_core_expr_release(&ctx->core_ctx, left);

        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    enum infix_op right_op = parse_infix_op(ctx);

    if (left_op_is_first(left_op, right_op)) {
        struct dy_core_expr new_left;
        if (!combine_infix(ctx, left, left_op, right, &new_left)) {
            ctx->stream.current_index = start_index;
            return false;
        }

        skip_whitespace_except_newline(ctx);

        if (!parse_expr_further(ctx, new_left, right_op, expr)) {
            ctx->stream.current_index = start_index;
            return false;
        }

        return true;
    } else {
        skip_whitespace_except_newline(ctx);

        struct dy_core_expr new_right;
        if (!parse_expr_further(ctx, right, right_op, &new_right)) {
            dy_core_expr_release(&ctx->core_ctx, left);
            ctx->stream.current_index = start_index;
            return false;
        }

        if (!combine_infix(ctx, left, left_op, new_right, expr)) {
            ctx->stream.current_index = start_index;
            return false;
        }

        return true;
    }
}

bool parse_expr_non_left_recursive(struct dy_parser_ctx *ctx, struct dy_core_expr *expr)
{
    if (dy_parse_positive_type_map(ctx, expr)) {
        return true;
    }

    if (dy_parse_negative_type_map(ctx, expr)) {
        return true;
    }

    if (dy_parse_implicit_positive_type_map(ctx, expr)) {
        return true;
    }

    if (dy_parse_implicit_negative_type_map(ctx, expr)) {
        return true;
    }

    struct dy_core_recursion positive_recursion;
    if (dy_parse_positive_recursion(ctx, &positive_recursion)) {
        *expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_RECURSION,
            .recursion = positive_recursion
        };

        return true;
    }

    struct dy_core_recursion negative_recursion;
    if (dy_parse_negative_recursion(ctx, &negative_recursion)) {
        *expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_RECURSION,
            .recursion = negative_recursion
        };

        return true;
    }

    if (dy_parse_list(ctx, expr)) {
        return true;
    }

    if (dy_parse_choice(ctx, expr)) {
        return true;
    }

    if (dy_parse_try_block(ctx, expr)) {
        return true;
    }

    if (dy_parse_do_block(ctx, expr)) {
        return true;
    }

    dy_array_t string = dy_array_create(1, 1, 8);
    if (dy_parse_string(ctx, &string)) {
        struct dy_string_data data = {
            .value = string
        };

        *expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_CUSTOM,
            .custom = dy_string_create(data)
        };

        return true;
    }

    if (parse_expr_parenthesized(ctx, expr)) {
        return true;
    }

    if (dy_parse_literal(ctx, DY_STR_LIT("All"))) {
        *expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_END,
            .end_polarity = DY_CORE_POLARITY_POSITIVE
        };

        return true;
    }

    if (dy_parse_literal(ctx, DY_STR_LIT("Any"))) {
        *expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_END,
            .end_polarity = DY_CORE_POLARITY_NEGATIVE
        };

        return true;
    }

    if (dy_parse_literal(ctx, DY_STR_LIT("Symbol"))) {
        *expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_SYMBOL
        };

        return true;
    }

    dy_array_t var = dy_array_create(1, 1, 8);
    if (dy_parse_variable(ctx, &var)) {
        for (size_t i = ctx->bound_vars.num_elems; i-- > 0;) {
            struct dy_bound_var bound_var;
            dy_array_get(ctx->bound_vars, i, &bound_var);

            if (dy_string_are_equal(dy_array_view(&var), bound_var.name)) {
                *expr = (struct dy_core_expr){
                    .tag = DY_CORE_EXPR_VARIABLE,
                    .variable_id = bound_var.replacement_id,
                };

                dy_array_destroy(var);

                return true;
            }
        }

        struct dy_uv_data data = {
            .var = var
        };

        *expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_CUSTOM,
            .custom = dy_uv_create(data)
        };

        return true;
    }

    return false;
}

bool dy_parse_string(struct dy_parser_ctx *ctx, dy_array_t *string)
{
    size_t start_index = ctx->stream.current_index;

    if (!dy_parse_literal(ctx, DY_STR_LIT("\""))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    for (;;) {
        char c;
        if (!get_char(ctx, &c)) {
            ctx->stream.current_index = start_index;
            return false;
        }

        if (c == '\"') {
            break;
        }

        dy_array_add(string, &c);
    }

    return true;
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

bool parse_bare_list(struct dy_parser_ctx *ctx, bool is_junction, enum dy_core_polarity polarity, struct dy_core_expr *list)
{
    size_t start_index = ctx->stream.current_index;

    struct dy_core_expr expr;
    if (!dy_parse_expr(ctx, &expr)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    if (dy_parse_literal(ctx, DY_STR_LIT("}"))) {
        *list = expr;
        return true;
    }

    if (dy_parse_literal(ctx, DY_STR_LIT(","))) {
        skip_whitespace(ctx);

        struct dy_core_expr expr2;
        if (!parse_bare_list(ctx, is_junction, polarity, &expr2)) {
            dy_core_expr_release(&ctx->core_ctx, expr);
            ctx->stream.current_index = start_index;
            return false;
        }

        if (is_junction) {
            *list = (struct dy_core_expr){
                .tag = DY_CORE_EXPR_JUNCTION,
                .junction = {
                    .e1 = dy_core_expr_new(expr),
                    .e2 = dy_core_expr_new(expr2),
                    .polarity = polarity
                }
            };
        } else {
            *list = (struct dy_core_expr){
                .tag = DY_CORE_EXPR_ALTERNATIVE,
                .alternative = {
                    .first = dy_core_expr_new(expr),
                    .second = dy_core_expr_new(expr2)
                }
            };
        }

        return true;
    }

    if (dy_parse_literal(ctx, DY_STR_LIT("\n")) || dy_parse_literal(ctx, DY_STR_LIT("\r\n"))) {
        skip_whitespace(ctx);

        if (dy_parse_literal(ctx, DY_STR_LIT("}"))) {
            *list = expr;
            return true;
        } else {
            struct dy_core_expr expr2;
            if (!parse_bare_list(ctx, is_junction, polarity, &expr2)) {
                dy_core_expr_release(&ctx->core_ctx, expr);
                ctx->stream.current_index = start_index;
                return false;
            }

            if (is_junction) {
                *list = (struct dy_core_expr){
                    .tag = DY_CORE_EXPR_JUNCTION,
                    .junction = {
                        .e1 = dy_core_expr_new(expr),
                        .e2 = dy_core_expr_new(expr2),
                        .polarity = polarity
                    }
                };
            } else {
                *list = (struct dy_core_expr){
                    .tag = DY_CORE_EXPR_ALTERNATIVE,
                    .alternative = {
                        .first = dy_core_expr_new(expr),
                        .second = dy_core_expr_new(expr2)
                    }
                };
            }

            return true;
        }
    }

    ctx->stream.current_index = start_index;

    return false;
}

bool dy_parse_list(struct dy_parser_ctx *ctx, struct dy_core_expr *list)
{
    size_t start_index = ctx->stream.current_index;

    if (!dy_parse_literal(ctx, DY_STR_LIT("list"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT("{"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    if (!parse_bare_list(ctx, true, DY_CORE_POLARITY_POSITIVE, list)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    return true;
}

bool dy_parse_choice(struct dy_parser_ctx *ctx, struct dy_core_expr *choice)
{
    size_t start_index = ctx->stream.current_index;

    if (!dy_parse_literal(ctx, DY_STR_LIT("choice"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT("{"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    if (!parse_bare_list(ctx, true, DY_CORE_POLARITY_NEGATIVE, choice)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    return true;
}

bool dy_parse_try_block(struct dy_parser_ctx *ctx, struct dy_core_expr *try_block)
{
    size_t start_index = ctx->stream.current_index;

    if (!dy_parse_literal(ctx, DY_STR_LIT("try"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT("{"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    if (!parse_bare_list(ctx, false, DY_CORE_POLARITY_POSITIVE, try_block)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    return true;
}

bool dy_parse_do_block(struct dy_parser_ctx *ctx, struct dy_core_expr *do_block)
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

    struct dy_core_expr expr;
    if (!parse_do_block_body(ctx, &expr)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT("}"))) {
        dy_core_expr_release(&ctx->core_ctx, expr);
        ctx->stream.current_index = start_index;
        return false;
    }

    *do_block = expr;

    return true;
}

bool dy_parse_file(struct dy_parser_ctx *ctx, struct dy_core_expr *do_block)
{
    size_t start_index = ctx->stream.current_index;

    skip_whitespace(ctx);

    if (!parse_do_block_body(ctx, do_block)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    return true;
}

bool parse_do_block_body(struct dy_parser_ctx *ctx, struct dy_core_expr *do_block)
{
    if (parse_do_block_body_equality(ctx, do_block)) {
        return true;
    }

    if (parse_do_block_body_let(ctx, do_block)) {
        return true;
    }

    if (parse_do_block_body_inverted_let(ctx, do_block)) {
        return true;
    }

    if (parse_do_block_body_inverted_ignored_expr(ctx, do_block)) {
        return true;
    }

    if (parse_do_block_def(ctx, do_block)) {
        return true;
    }

    if (parse_do_block_body_ignored_expr(ctx, do_block)) {
        return true;
    }

    if (dy_parse_expr(ctx, do_block)) {
        return true;
    }

    return false;
}

bool parse_do_block_def(struct dy_parser_ctx *ctx, struct dy_core_expr *def)
{
    size_t start_index = ctx->stream.current_index;

    if (!dy_parse_literal(ctx, DY_STR_LIT("def"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    dy_array_t name = dy_array_create(1, 1, 8);
    if (!dy_parse_variable(ctx, &name)) {
        dy_array_destroy(name);
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT("="))) {
        dy_array_destroy(name);
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    struct dy_core_expr expr;
    if (!dy_parse_expr(ctx, &expr)) {
        dy_array_destroy(name);
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    if (!skip_semicolon_or_newline(ctx)) {
        dy_array_destroy(name);
        dy_core_expr_release(&ctx->core_ctx, expr);
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    size_t id = ctx->core_ctx.running_id++;

    dy_array_add(&ctx->bound_vars, &(struct dy_bound_var){
        .name = dy_array_view(&name),
        .replacement_id = id
    });

    struct dy_core_expr rest;
    bool b = parse_do_block_body(ctx, &rest);

    ctx->bound_vars.num_elems--;

    dy_array_destroy(name);

    if (!b) {
        dy_core_expr_release(&ctx->core_ctx, expr);
        ctx->stream.current_index = start_index;
        return false;
    }

    struct dy_def_data data = {
        .id = id,
        .arg = expr,
        .body = rest
    };

    *def = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_CUSTOM,
        .custom = dy_def_create(data)
    };

    return true;
}

bool parse_do_block_body_equality(struct dy_parser_ctx *ctx, struct dy_core_expr *equality)
{
    size_t start_index = ctx->stream.current_index;

    struct dy_core_expr e1;
    if (!dy_parse_expr(ctx, &e1)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT("="))) {
        dy_core_expr_release(&ctx->core_ctx, e1);
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    struct dy_core_expr e2;
    if (!dy_parse_expr(ctx, &e2)) {
        dy_core_expr_release(&ctx->core_ctx, e1);
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    if (!skip_semicolon_or_newline(ctx)) {
        dy_core_expr_release(&ctx->core_ctx, e1);
        dy_core_expr_release(&ctx->core_ctx, e2);
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    struct dy_core_expr rest;
    if (!parse_do_block_body(ctx, &rest)) {
        dy_core_expr_release(&ctx->core_ctx, e1);
        dy_core_expr_release(&ctx->core_ctx, e2);
        ctx->stream.current_index = start_index;
        return false;
    }

    struct dy_core_expr positive_equality_map = {
        .tag = DY_CORE_EXPR_EQUALITY_MAP,
        .equality_map = {
            .e1 = dy_core_expr_new(e1),
            .e2 = dy_core_expr_new(rest),
            .polarity = DY_CORE_POLARITY_POSITIVE,
            .is_implicit = false,
        }
    };

    struct dy_core_expr result_type = {
        .tag = DY_CORE_EXPR_VARIABLE,
        .variable_id = ctx->core_ctx.running_id++
    };

    struct dy_core_expr elim = {
        .tag = DY_CORE_EXPR_EQUALITY_MAP_ELIM,
        .equality_map_elim = {
            .expr = dy_core_expr_new(positive_equality_map),
            .map = {
                .e1 = dy_core_expr_new(e2),
                .e2 = dy_core_expr_new(result_type),
                .polarity = DY_CORE_POLARITY_NEGATIVE,
                .is_implicit = false,
            },
            .check_result = DY_MAYBE,
            .id = ctx->core_ctx.running_id++
        }
    };

    struct dy_core_expr any = {
        .tag = DY_CORE_EXPR_END,
        .end_polarity = DY_CORE_POLARITY_NEGATIVE
    };

    *equality = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_INFERENCE_TYPE_MAP,
        .inference_type_map = {
            .id = result_type.variable_id,
            .type = dy_core_expr_new(any),
            .expr = dy_core_expr_new(elim),
            .polarity = DY_CORE_POLARITY_POSITIVE,
        }
    };

    return true;
}

bool parse_do_block_body_inverted_let(struct dy_parser_ctx *ctx, struct dy_core_expr *let)
{
    size_t start_index = ctx->stream.current_index;

    if (!dy_parse_literal(ctx, DY_STR_LIT("invert"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT("let"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    dy_array_t name = dy_array_create(1, 1, 8);
    if (!dy_parse_variable(ctx, &name)) {
        dy_array_destroy(name);
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT("="))) {
        dy_array_destroy(name);
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    struct dy_core_expr expr;
    if (!dy_parse_expr(ctx, &expr)) {
        dy_array_destroy(name);
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    if (!skip_semicolon_or_newline(ctx)) {
        dy_array_destroy(name);
        dy_core_expr_release(&ctx->core_ctx, expr);
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    size_t id = ctx->core_ctx.running_id++;

    dy_array_add(&ctx->bound_vars, &(struct dy_bound_var){
        .name = dy_array_view(&name),
        .replacement_id = id
    });

    struct dy_core_expr rest;
    bool b = parse_do_block_body(ctx, &rest);

    ctx->bound_vars.num_elems--;
    dy_array_destroy(name);

    if (!b) {
        dy_core_expr_release(&ctx->core_ctx, expr);
        ctx->stream.current_index = start_index;
        return false;
    }

    struct dy_core_expr arg_type = {
        .tag = DY_CORE_EXPR_VARIABLE,
        .variable_id = ctx->core_ctx.running_id++
    };

    struct dy_core_expr positive_type_map = {
        .tag = DY_CORE_EXPR_TYPE_MAP,
        .type_map = {
            .id = id,
            .type = dy_core_expr_new(arg_type),
            .expr = dy_core_expr_new(rest),
            .polarity = DY_CORE_POLARITY_POSITIVE,
            .is_implicit = false,
        }
    };

    struct dy_core_expr any = {
        .tag = DY_CORE_EXPR_END,
        .end_polarity = DY_CORE_POLARITY_NEGATIVE
    };

    struct dy_core_expr result_inference_type_map = {
        .tag = DY_CORE_EXPR_INFERENCE_TYPE_MAP,
        .inference_type_map = {
            .id = arg_type.variable_id,
            .type = dy_core_expr_new(any),
            .expr = dy_core_expr_new(positive_type_map),
            .polarity = DY_CORE_POLARITY_NEGATIVE,
        }
    };

    struct dy_core_expr result_type = {
        .tag = DY_CORE_EXPR_VARIABLE,
        .variable_id = ctx->core_ctx.running_id++
    };

    struct dy_core_expr elim = {
        .tag = DY_CORE_EXPR_EQUALITY_MAP_ELIM,
        .equality_map_elim = {
            .expr = dy_core_expr_new(expr),
            .map = {
                .e1 = dy_core_expr_new(result_inference_type_map),
                .e2 = dy_core_expr_new(dy_core_expr_retain(&ctx->core_ctx, result_type)),
                .polarity = DY_CORE_POLARITY_NEGATIVE,
                .is_implicit = false,
            },
            .check_result = DY_MAYBE,
            .id = ctx->core_ctx.running_id++
        }
    };

    *let = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_INFERENCE_TYPE_MAP,
        .inference_type_map = {
            .id = result_type.variable_id,
            .type = dy_core_expr_new(any),
            .expr = dy_core_expr_new(elim),
            .polarity = DY_CORE_POLARITY_POSITIVE,
        }
    };

    return true;
}

bool parse_do_block_body_let(struct dy_parser_ctx *ctx, struct dy_core_expr *let)
{
    size_t start_index = ctx->stream.current_index;

    if (!dy_parse_literal(ctx, DY_STR_LIT("let"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    dy_array_t name = dy_array_create(1, 1, 8);
    if (!dy_parse_variable(ctx, &name)) {
        dy_array_destroy(name);
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT("="))) {
        dy_array_destroy(name);
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    struct dy_core_expr expr;
    if (!dy_parse_expr(ctx, &expr)) {
        dy_array_destroy(name);
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    if (!skip_semicolon_or_newline(ctx)) {
        dy_array_destroy(name);
        dy_core_expr_release(&ctx->core_ctx, expr);
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    size_t id = ctx->core_ctx.running_id++;

    dy_array_add(&ctx->bound_vars, &(struct dy_bound_var){
        .name = dy_array_view(&name),
        .replacement_id = id
    });

    struct dy_core_expr rest;
    bool b = parse_do_block_body(ctx, &rest);

    ctx->bound_vars.num_elems--;
    dy_array_destroy(name);

    if (!b) {
        dy_core_expr_release(&ctx->core_ctx, expr);
        ctx->stream.current_index = start_index;
        return false;
    }

    struct dy_core_expr arg_type = {
        .tag = DY_CORE_EXPR_VARIABLE,
        .variable_id = ctx->core_ctx.running_id++
    };

    struct dy_core_expr positive_type_map = {
        .tag = DY_CORE_EXPR_TYPE_MAP,
        .type_map = {
            .id = id,
            .type = dy_core_expr_new(arg_type),
            .expr = dy_core_expr_new(rest),
            .polarity = DY_CORE_POLARITY_POSITIVE,
            .is_implicit = false,
        }
    };

    struct dy_core_expr any = {
        .tag = DY_CORE_EXPR_END,
        .end_polarity = DY_CORE_POLARITY_NEGATIVE
    };

    struct dy_core_expr result_inference_type_map = {
        .tag = DY_CORE_EXPR_INFERENCE_TYPE_MAP,
        .inference_type_map = {
            .id = arg_type.variable_id,
            .type = dy_core_expr_new(any),
            .expr = dy_core_expr_new(positive_type_map),
            .polarity = DY_CORE_POLARITY_NEGATIVE,
        }
    };

    struct dy_core_expr result_type = {
        .tag = DY_CORE_EXPR_VARIABLE,
        .variable_id = ctx->core_ctx.running_id++
    };

    struct dy_core_expr elim = {
        .tag = DY_CORE_EXPR_EQUALITY_MAP_ELIM,
        .equality_map_elim = {
            .expr = dy_core_expr_new(result_inference_type_map),
            .map = {
                .e1 = dy_core_expr_new(expr),
                .e2 = dy_core_expr_new(dy_core_expr_retain(&ctx->core_ctx, result_type)),
                .polarity = DY_CORE_POLARITY_NEGATIVE,
                .is_implicit = false,
            },
            .check_result = DY_MAYBE,
            .id = ctx->core_ctx.running_id++
        }
    };

    *let = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_INFERENCE_TYPE_MAP,
        .inference_type_map = {
            .id = result_type.variable_id,
            .type = dy_core_expr_new(dy_core_expr_retain(&ctx->core_ctx, any)),
            .expr = dy_core_expr_new(elim),
            .polarity = DY_CORE_POLARITY_POSITIVE,
        }
    };

    return true;
}

bool parse_do_block_body_inverted_ignored_expr(struct dy_parser_ctx *ctx, struct dy_core_expr *ignored_expr)
{
    size_t start_index = ctx->stream.current_index;

    if (!dy_parse_literal(ctx, DY_STR_LIT("invert"))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    struct dy_core_expr expr;
    if (!dy_parse_expr(ctx, &expr)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    if (!skip_semicolon_or_newline(ctx)) {
        dy_core_expr_release(&ctx->core_ctx, expr);
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    struct dy_core_expr rest;
    if (!parse_do_block_body(ctx, &rest)) {
        dy_core_expr_release(&ctx->core_ctx, expr);
        ctx->stream.current_index = start_index;
        return false;
    }

    struct dy_core_expr arg_type = {
        .tag = DY_CORE_EXPR_VARIABLE,
        .variable_id = ctx->core_ctx.running_id++
    };

    struct dy_core_expr positive_type_map = {
        .tag = DY_CORE_EXPR_TYPE_MAP,
        .type_map = {
            .id = ctx->core_ctx.running_id++,
            .type = dy_core_expr_new(arg_type),
            .expr = dy_core_expr_new(rest),
            .polarity = DY_CORE_POLARITY_POSITIVE,
            .is_implicit = false,
        }
    };

    struct dy_core_expr any = {
        .tag = DY_CORE_EXPR_END,
        .end_polarity = DY_CORE_POLARITY_NEGATIVE
    };

    struct dy_core_expr result_inference_type_map = {
        .tag = DY_CORE_EXPR_INFERENCE_TYPE_MAP,
        .inference_type_map = {
            .id = arg_type.variable_id,
            .type = dy_core_expr_new(any),
            .expr = dy_core_expr_new(positive_type_map),
            .polarity = DY_CORE_POLARITY_NEGATIVE,
        }
    };

    struct dy_core_expr result_type = {
        .tag = DY_CORE_EXPR_VARIABLE,
        .variable_id = ctx->core_ctx.running_id++
    };

    struct dy_core_expr elim = {
        .tag = DY_CORE_EXPR_EQUALITY_MAP_ELIM,
        .equality_map_elim = {
            .expr = dy_core_expr_new(expr),
            .map = {
                .e1 = dy_core_expr_new(result_inference_type_map),
                .e2 = dy_core_expr_new(dy_core_expr_retain(&ctx->core_ctx, result_type)),
                .polarity = DY_CORE_POLARITY_NEGATIVE,
                .is_implicit = false,
            },
            .check_result = DY_MAYBE,
            .id = ctx->core_ctx.running_id++
        }
    };

    *ignored_expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_INFERENCE_TYPE_MAP,
        .inference_type_map = {
            .id = result_type.variable_id,
            .type = dy_core_expr_new(dy_core_expr_retain(&ctx->core_ctx, any)),
            .expr = dy_core_expr_new(elim),
            .polarity = DY_CORE_POLARITY_POSITIVE,
        }
    };

    return true;
}

bool parse_do_block_body_ignored_expr(struct dy_parser_ctx *ctx, struct dy_core_expr *ignored_expr)
{
    size_t start_index = ctx->stream.current_index;

    struct dy_core_expr expr;
    if (!dy_parse_expr(ctx, &expr)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    if (!skip_semicolon_or_newline(ctx)) {
        dy_core_expr_release(&ctx->core_ctx, expr);
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    struct dy_core_expr rest;
    if (!parse_do_block_body(ctx, &rest)) {
        dy_core_expr_release(&ctx->core_ctx, expr);
        ctx->stream.current_index = start_index;
        return false;
    }

    struct dy_core_expr inference_var_expr = {
        .tag = DY_CORE_EXPR_VARIABLE,
        .variable_id = ctx->core_ctx.running_id++
    };

    struct dy_core_expr positive_type_map = {
        .tag = DY_CORE_EXPR_TYPE_MAP,
        .type_map = {
            .id = ctx->core_ctx.running_id++,
            .type = dy_core_expr_new(inference_var_expr),
            .expr = dy_core_expr_new(rest),
            .polarity = DY_CORE_POLARITY_POSITIVE,
            .is_implicit = false,
        }
    };

    struct dy_core_expr any = {
        .tag = DY_CORE_EXPR_END,
        .end_polarity = DY_CORE_POLARITY_NEGATIVE
    };

    struct dy_core_expr result_inference_type_map = {
        .tag = DY_CORE_EXPR_INFERENCE_TYPE_MAP,
        .inference_type_map = {
            .id = inference_var_expr.variable_id,
            .type = dy_core_expr_new(any),
            .expr = dy_core_expr_new(positive_type_map),
            .polarity = DY_CORE_POLARITY_NEGATIVE,
        }
    };

    struct dy_core_expr result_type = {
        .tag = DY_CORE_EXPR_VARIABLE,
        .variable_id = ctx->core_ctx.running_id++
    };

    struct dy_core_expr elim = {
        .tag = DY_CORE_EXPR_EQUALITY_MAP_ELIM,
        .equality_map_elim = {
            .expr = dy_core_expr_new(result_inference_type_map),
            .map = {
                .e1 = dy_core_expr_new(expr),
                .e2 = dy_core_expr_new(dy_core_expr_retain(&ctx->core_ctx, result_type)),
                .polarity = DY_CORE_POLARITY_NEGATIVE,
                .is_implicit = false,
            },
            .check_result = DY_MAYBE,
            .id = ctx->core_ctx.running_id++
        }
    };

    *ignored_expr = (struct dy_core_expr){
        .tag = DY_CORE_EXPR_INFERENCE_TYPE_MAP,
        .inference_type_map = {
            .id = result_type.variable_id,
            .type = dy_core_expr_new(dy_core_expr_retain(&ctx->core_ctx, any)),
            .expr = dy_core_expr_new(elim),
            .polarity = DY_CORE_POLARITY_POSITIVE,
        }
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

bool parse_expr_parenthesized(struct dy_parser_ctx *ctx, struct dy_core_expr *expr)
{
    size_t start_index = ctx->stream.current_index;

    if (!dy_parse_literal(ctx, DY_STR_LIT("("))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    struct dy_core_expr e;
    if (!dy_parse_expr(ctx, &e)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT(")"))) {
        dy_core_expr_release(&ctx->core_ctx, e);
        ctx->stream.current_index = start_index;
        return false;
    }

    *expr = e;

    return true;
}

bool parse_recursion(struct dy_parser_ctx *ctx, dy_string_t rec_lit, struct dy_core_recursion *recursion)
{
    size_t start_index = ctx->stream.current_index;

    if (!dy_parse_literal(ctx, rec_lit)) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    dy_array_t self_name = dy_array_create(1, 1, 8);
    if (!dy_parse_variable(ctx, &self_name)) {
        dy_array_destroy(self_name);
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    struct dy_core_expr type;
    if (!dy_parse_expr(ctx, &type)) {
        dy_array_destroy(self_name);
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT("="))) {
        dy_array_destroy(self_name);
        dy_core_expr_release(&ctx->core_ctx, type);
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace_except_newline(ctx);

    size_t id = ctx->core_ctx.running_id++;

    dy_array_add(&ctx->bound_vars, &(struct dy_bound_var){
        .name = dy_array_view(&self_name),
        .replacement_id = id
    });

    struct dy_core_expr expr;
    bool b = dy_parse_expr(ctx, &expr);

    ctx->bound_vars.num_elems--;
    dy_array_destroy(self_name);

    if (!b) {
        dy_core_expr_release(&ctx->core_ctx, type);
        ctx->stream.current_index = start_index;
        return false;
    }

    *recursion = (struct dy_core_recursion){
        .id = id,
        .type = dy_core_expr_new(type),
        .expr = dy_core_expr_new(expr),
        .polarity = dy_string_are_equal(rec_lit, DY_STR_LIT("prec")) ? DY_CORE_POLARITY_POSITIVE : DY_CORE_POLARITY_NEGATIVE,
        .check_result = DY_MAYBE
    };

    return true;
}

bool dy_parse_positive_recursion(struct dy_parser_ctx *ctx, struct dy_core_recursion *recursion)
{
    return parse_recursion(ctx, DY_STR_LIT("prec"), recursion);
}

bool dy_parse_negative_recursion(struct dy_parser_ctx *ctx, struct dy_core_recursion *recursion)
{
    return parse_recursion(ctx, DY_STR_LIT("nrec"), recursion);
}

bool parse_type_map(struct dy_parser_ctx *ctx, dy_string_t arrow_literal, bool is_implicit, struct dy_core_expr *type_map)
{
    size_t start_index = ctx->stream.current_index;

    dy_array_t name = dy_array_create(1, 1, 8);
    bool have_name = false;
    struct dy_core_expr type;
    bool have_type = false;
    if (!parse_arg(ctx, &name, &have_name, &type, &have_type)) {
        dy_array_destroy(name);
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    if (!dy_parse_literal(ctx, arrow_literal)) {
        dy_array_destroy(name);
        if (have_type) {
            dy_core_expr_release(&ctx->core_ctx, type);
        }
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    size_t id = ctx->core_ctx.running_id++;

    if (have_name) {
        dy_array_add(&ctx->bound_vars, &(struct dy_bound_var){
            .name = dy_array_view(&name),
            .replacement_id = id
        });
    }

    struct dy_core_expr expr;
    bool b = dy_parse_expr(ctx, &expr);

    dy_array_destroy(name);

    if (have_name) {
        ctx->bound_vars.num_elems--;
    }

    if (!b) {
        if (have_type) {
            dy_core_expr_release(&ctx->core_ctx, type);
        }
        ctx->stream.current_index = start_index;
        return false;
    }

    enum dy_core_polarity polarity;
    if (dy_string_are_equal(arrow_literal, DY_STR_LIT("->")) || dy_string_are_equal(arrow_literal, DY_STR_LIT("@->"))) {
        polarity = DY_CORE_POLARITY_POSITIVE;
    } else {
        polarity = DY_CORE_POLARITY_NEGATIVE;
    }

    if (have_type) {
        *type_map = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_TYPE_MAP,
            .type_map = {
                .id = id,
                .type = dy_core_expr_new(type),
                .expr = dy_core_expr_new(expr),
                .polarity = polarity,
                .is_implicit = is_implicit,
            }
        };
    } else {
        struct dy_core_expr inference_type = {
            .tag = DY_CORE_EXPR_VARIABLE,
            .variable_id = ctx->core_ctx.running_id++
        };

        struct dy_core_expr result_type_map = {
            .tag = DY_CORE_EXPR_TYPE_MAP,
            .type_map = {
                .id = id,
                .type = dy_core_expr_new(inference_type),
                .expr = dy_core_expr_new(expr),
                .polarity = polarity,
                .is_implicit = is_implicit,
            }
        };

        struct dy_core_expr any = {
            .tag = DY_CORE_EXPR_END,
            .end_polarity = DY_CORE_POLARITY_NEGATIVE
        };

        *type_map = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_INFERENCE_TYPE_MAP,
            .inference_type_map = {
                .id = inference_type.variable_id,
                .type = dy_core_expr_new(any),
                .expr = dy_core_expr_new(result_type_map),
                .polarity = DY_CORE_POLARITY_NEGATIVE,
            }
        };
    }

    return true;
}

bool dy_parse_positive_type_map(struct dy_parser_ctx *ctx, struct dy_core_expr *type_map)
{
    return parse_type_map(ctx, DY_STR_LIT("->"), false, type_map);
}

bool dy_parse_negative_type_map(struct dy_parser_ctx *ctx, struct dy_core_expr *type_map)
{
    return parse_type_map(ctx, DY_STR_LIT("~>"), false, type_map);
}

bool dy_parse_implicit_positive_type_map(struct dy_parser_ctx *ctx, struct dy_core_expr *type_map)
{
    return parse_type_map(ctx, DY_STR_LIT("@->"), true, type_map);
}

bool dy_parse_implicit_negative_type_map(struct dy_parser_ctx *ctx, struct dy_core_expr *type_map)
{
    return parse_type_map(ctx, DY_STR_LIT("@~>"), true, type_map);
}

bool parse_arg(struct dy_parser_ctx *ctx, dy_array_t *name, bool *have_name, struct dy_core_expr *expr, bool *have_expr)
{
    size_t start_index = ctx->stream.current_index;

    if (!dy_parse_literal(ctx, DY_STR_LIT("["))) {
        ctx->stream.current_index = start_index;
        return false;
    }

    skip_whitespace(ctx);

    if (dy_parse_literal(ctx, DY_STR_LIT("]"))) {
        return true;
    }

    if (!dy_parse_literal(ctx, DY_STR_LIT("_"))) {
        *have_name = true;

        if (!dy_parse_variable(ctx, name)) {
            ctx->stream.current_index = start_index;
            return false;
        }
    }

    skip_whitespace(ctx);

    *have_expr = dy_parse_expr(ctx, expr);

    skip_whitespace(ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT("]"))) {
        if (*have_expr) {
            dy_core_expr_release(&ctx->core_ctx, *expr);
        }
        ctx->stream.current_index = start_index;
        return false;
    }

    return true;
}

bool combine_infix(struct dy_parser_ctx *ctx, struct dy_core_expr left, enum infix_op op, struct dy_core_expr right, struct dy_core_expr *expr)
{
    switch (op) {
    case INFIX_OP_BANG:
        if (right.tag == DY_CORE_EXPR_EQUALITY_MAP && right.equality_map.polarity == DY_CORE_POLARITY_NEGATIVE) {
            *expr = (struct dy_core_expr){
                .tag = DY_CORE_EXPR_EQUALITY_MAP_ELIM,
                .equality_map_elim = {
                    .expr = dy_core_expr_new(left),
                    .map = right.equality_map,
                    .check_result = DY_MAYBE
                }
            };

            return true;
        }

        if (right.tag == DY_CORE_EXPR_TYPE_MAP && right.type_map.polarity == DY_CORE_POLARITY_NEGATIVE) {
            *expr = (struct dy_core_expr){
                .tag = DY_CORE_EXPR_TYPE_MAP_ELIM,
                .type_map_elim = {
                    .expr = dy_core_expr_new(left),
                    .map = right.type_map,
                    .check_result = DY_MAYBE
                }
            };

            return true;
        }

        dy_core_expr_release(&ctx->core_ctx, left);
        dy_core_expr_release(&ctx->core_ctx, right);

        return false;
    case INFIX_OP_STRAIGHT_ARROW:
        *expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_EQUALITY_MAP,
            .equality_map = {
                .e1 = dy_core_expr_new(left),
                .e2 = dy_core_expr_new(right),
                .is_implicit = false,
                .polarity = DY_CORE_POLARITY_POSITIVE
            }
        };

        return true;
    case INFIX_OP_SQUIGGLY_ARROW:
        *expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_EQUALITY_MAP,
            .equality_map = {
                .e1 = dy_core_expr_new(left),
                .e2 = dy_core_expr_new(right),
                .is_implicit = false,
                .polarity = DY_CORE_POLARITY_NEGATIVE
            }
        };

        return true;
    case INFIX_OP_AT_STRAIGHT_ARROW:
        *expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_EQUALITY_MAP,
            .equality_map = {
                .e1 = dy_core_expr_new(left),
                .e2 = dy_core_expr_new(right),
                .is_implicit = true,
                .polarity = DY_CORE_POLARITY_POSITIVE
            }
        };

        return true;
    case INFIX_OP_AT_SQUIGGLY_ARROW:
        *expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_EQUALITY_MAP,
            .equality_map = {
                .e1 = dy_core_expr_new(left),
                .e2 = dy_core_expr_new(right),
                .is_implicit = true,
                .polarity = DY_CORE_POLARITY_NEGATIVE
            }
        };

        return true;
    case INFIX_OP_JUXTAPOSITION: {
        struct dy_core_expr inference_var_expr = {
            .tag = DY_CORE_EXPR_VARIABLE,
            .variable_id = ctx->core_ctx.running_id++
        };

        struct dy_core_expr elim = {
            .tag = DY_CORE_EXPR_EQUALITY_MAP_ELIM,
            .equality_map_elim = {
                .expr = dy_core_expr_new(left),
                .map = {
                    .e1 = dy_core_expr_new(right),
                    .e2 = dy_core_expr_new(inference_var_expr),
                    .polarity = DY_CORE_POLARITY_NEGATIVE,
                    .is_implicit = false,
                },
                .check_result = DY_MAYBE,
            }
        };

        struct dy_core_expr any = {
            .tag = DY_CORE_EXPR_END,
            .end_polarity = DY_CORE_POLARITY_NEGATIVE
        };

        *expr = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_INFERENCE_TYPE_MAP,
            .inference_type_map = {
                .id = inference_var_expr.variable_id,
                .type = dy_core_expr_new(any),
                .expr = dy_core_expr_new(elim),
                .polarity = DY_CORE_POLARITY_POSITIVE,
            }
        };

        return true;
    }
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
