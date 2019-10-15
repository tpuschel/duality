/*
 * Copyright 2017-2019 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <duality/syntax/parser.h>

#include <duality/support/assert.h>

enum infix_op {
    INFIX_OP_STRAIGHT_ARROW,
    INFIX_OP_SQUIGGLY_ARROW
};

enum assoc {
    ASSOC_LEFT,
    ASSOC_RIGHT
};

static bool left_op_is_first(enum infix_op left, enum infix_op right);

static int op_precedence(enum infix_op op);

static enum assoc op_assoc(enum infix_op op);

static bool parse_expr_non_left_recursive(struct dy_parser_ctx ctx, struct dy_ast_expr *expr);

static bool parse_expr_further1(struct dy_parser_ctx ctx, struct dy_ast_expr left, struct dy_ast_expr *expr);

static bool parse_expr_further2(struct dy_parser_ctx ctx, struct dy_ast_expr left, enum infix_op op, struct dy_ast_expr *expr);

static bool parse_expr_further3(struct dy_parser_ctx ctx, struct dy_ast_expr left, enum infix_op op, struct dy_ast_expr right, struct dy_ast_expr *expr);

static bool parse_expr_further4(struct dy_parser_ctx ctx, struct dy_ast_expr left, enum infix_op left_op, struct dy_ast_expr right, enum infix_op right_op, struct dy_ast_expr *expr);

static struct dy_ast_expr combine_infix(struct dy_parser_ctx ctx, struct dy_ast_expr left, enum infix_op op, struct dy_ast_expr right);

static bool parse_expr_parenthesized(struct dy_parser_ctx ctx, struct dy_ast_expr *expr);

static bool parse_arg(struct dy_parser_ctx ctx, struct dy_ast_arg *arg);

static struct dy_ast_expr *alloc_expr(struct dy_parser_ctx ctx, struct dy_ast_expr expr);

static void skip_whitespace_except_newline(struct dy_parser_ctx ctx);

static bool parse_do_block_stmnt(struct dy_parser_ctx ctx, struct dy_ast_do_block_stmnt *stmnt);

static void skip_whitespace(struct dy_parser_ctx ctx);

static bool is_at_end(struct dy_parser_ctx ctx);

static bool get_char(struct dy_parser_ctx ctx, char *c);

static bool parse_one_of(struct dy_parser_ctx ctx, dy_string_t chars);

static bool parse_exactly_one(struct dy_parser_ctx ctx, char c);

static bool parse_bare_list(struct dy_parser_ctx ctx, struct dy_ast_list *list);

static bool parse_let_stmnt(struct dy_parser_ctx ctx, struct dy_ast_do_block_stmnt *stmnt);

static bool parse_stmnt_equal(struct dy_parser_ctx ctx, struct dy_ast_do_block_stmnt *stmnt);

static bool parse_elimination(struct dy_parser_ctx ctx, struct dy_ast_expr left, struct dy_ast_expr *expr);

static bool parse_positive_value_map(struct dy_parser_ctx ctx, struct dy_ast_expr left, struct dy_ast_expr *expr);

static bool parse_negative_value_map(struct dy_parser_ctx ctx, struct dy_ast_expr left, struct dy_ast_expr *expr);

static bool parse_elimination_further(struct dy_parser_ctx ctx, struct dy_ast_expr left, enum infix_op op, struct dy_ast_expr right, struct dy_ast_expr *expr);

static bool parse_positive_value_map_further(struct dy_parser_ctx ctx, struct dy_ast_expr left, enum infix_op op, struct dy_ast_expr right, struct dy_ast_expr *expr);

static bool parse_negative_value_map_further(struct dy_parser_ctx ctx, struct dy_ast_expr left, enum infix_op op, struct dy_ast_expr right, struct dy_ast_expr *expr);

static bool skip_line_comment(struct dy_parser_ctx ctx);

static bool skip_block_comment(struct dy_parser_ctx ctx);

static bool parse_type_map(struct dy_parser_ctx ctx, dy_string_t arrow_literal, struct dy_ast_type_map *type_map);

struct dy_parser_transaction_multiple dy_parse_multiple_start(struct dy_parser_ctx *ctx)
{
    size_t *out = ctx->index_out;
    ctx->index_out = &ctx->index_in;

    return (struct dy_parser_transaction_multiple){
        .index_out = out
    };
}

void dy_parse_multiple_succeeded(struct dy_parser_ctx ctx, struct dy_parser_transaction_multiple transaction)
{
    *transaction.index_out = ctx.index_in;
}

struct dy_parser_transaction_choice dy_parse_choice_start(struct dy_parser_ctx ctx)
{
    return (struct dy_parser_transaction_choice){
        .initial_error_cnt = dy_array_size(ctx.errors)
    };
}

void dy_parse_choice_failed(struct dy_parser_ctx ctx, struct dy_parser_transaction_choice transaction)
{
    dy_array_set_size(ctx.errors, transaction.initial_error_cnt);
}

bool dy_parse_literal(struct dy_parser_ctx ctx, dy_string_t s)
{
    struct dy_parser_transaction_multiple tr = dy_parse_multiple_start(&ctx);

    for (size_t i = 0; i < s.size; ++i) {
        if (!parse_exactly_one(ctx, s.ptr[i])) {
            return false;
        }
    }

    dy_parse_multiple_succeeded(ctx, tr);

    return true;
}

bool dy_parse_variable(struct dy_parser_ctx ctx, dy_string_t *var)
{
    struct dy_parser_transaction_multiple tr = dy_parse_multiple_start(&ctx);

    const void *data = ctx.text.ptr + ctx.index_in;
    size_t count = 0;

    if (!parse_one_of(ctx, DY_STR_LIT("abcdefghijklmnopqrstuvwxyz"))) {
        return false;
    }

    ++count;

    for (;;) {
        if (!parse_one_of(ctx, DY_STR_LIT("abcdefghijklmnopqrstuvwxyz0123456789-?"))) {
            *var = (dy_string_t){
                .ptr = data,
                .size = count
            };

            dy_parse_multiple_succeeded(ctx, tr);

            return true;
        }

        ++count;
    }
}

bool skip_line_comment(struct dy_parser_ctx ctx)
{
    struct dy_parser_transaction_multiple tr = dy_parse_multiple_start(&ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT("//"))) {
        return false;
    }

    for (;;) {
        char c;
        if (!get_char(ctx, &c)) {
            dy_parse_multiple_succeeded(ctx, tr);
            return true;
        }

        if (c == '\n') {
            dy_parse_multiple_succeeded(ctx, tr);
            return true;
        }
    }
}

bool skip_block_comment(struct dy_parser_ctx ctx)
{
    struct dy_parser_transaction_multiple tr = dy_parse_multiple_start(&ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT("/*"))) {
        return false;
    }

    for (;;) {
        struct dy_parser_transaction_choice ctr = dy_parse_choice_start(ctx);

        if (skip_block_comment(ctx)) {
            continue;
        }

        dy_parse_choice_failed(ctx, ctr);

        if (dy_parse_literal(ctx, DY_STR_LIT("*/"))) {
            dy_parse_multiple_succeeded(ctx, tr);
            return true;
        }

        dy_parse_choice_failed(ctx, ctr);

        char c;
        if (!get_char(ctx, &c)) {
            return false;
        }
    }
}

bool dy_parse_expr(struct dy_parser_ctx ctx, struct dy_ast_expr *expr)
{
    struct dy_parser_transaction_multiple tr = dy_parse_multiple_start(&ctx);

    struct dy_ast_expr left;
    if (!parse_expr_non_left_recursive(ctx, &left)) {
        return false;
    }

    skip_whitespace_except_newline(ctx);

    if (!parse_expr_further1(ctx, left, expr)) {
        return false;
    }

    dy_parse_multiple_succeeded(ctx, tr);

    return true;
}

bool parse_expr_non_left_recursive(struct dy_parser_ctx ctx, struct dy_ast_expr *expr)
{
    struct dy_parser_transaction_choice tr = dy_parse_choice_start(ctx);

    struct dy_ast_type_map positive_type_map;
    if (dy_parse_positive_type_map(ctx, &positive_type_map)) {
        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_POSITIVE_TYPE_MAP,
            .positive_type_map = positive_type_map
        };

        return true;
    }

    dy_parse_choice_failed(ctx, tr);

    struct dy_ast_type_map negative_type_map;
    if (dy_parse_negative_type_map(ctx, &negative_type_map)) {
        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_NEGATIVE_TYPE_MAP,
            .negative_type_map = negative_type_map
        };

        return true;
    }

    dy_parse_choice_failed(ctx, tr);

    struct dy_ast_list list;
    if (dy_parse_list(ctx, &list)) {
        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_LIST,
            .list = list
        };

        return true;
    }

    dy_parse_choice_failed(ctx, tr);

    struct dy_ast_list choice;
    if (dy_parse_choice(ctx, &choice)) {
        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_CHOICE,
            .choice = choice
        };

        return true;
    }

    dy_parse_choice_failed(ctx, tr);

    struct dy_ast_list try_block;
    if (dy_parse_try_block(ctx, &try_block)) {
        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_TRY_BLOCK,
            .try_block = try_block
        };

        return true;
    }

    dy_parse_choice_failed(ctx, tr);

    struct dy_ast_do_block do_block;
    if (dy_parse_do_block(ctx, &do_block)) {
        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_DO_BLOCK,
            .do_block = do_block
        };

        return true;
    }

    dy_parse_choice_failed(ctx, tr);

    if (parse_expr_parenthesized(ctx, expr)) {
        return true;
    }

    dy_parse_choice_failed(ctx, tr);

    if (dy_parse_literal(ctx, DY_STR_LIT("Type"))) {
        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_TYPE_TYPE,
        };

        return true;
    }

    dy_parse_choice_failed(ctx, tr);

    if (dy_parse_literal(ctx, DY_STR_LIT("String"))) {
        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_TYPE_STRING
        };

        return true;
    }

    dy_parse_choice_failed(ctx, tr);

    dy_string_t var;
    if (dy_parse_variable(ctx, &var)) {
        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_VARIABLE,
            .variable = var
        };

        return true;
    }

    dy_parse_choice_failed(ctx, tr);

    dy_string_t string;
    if (dy_parse_string(ctx, &string)) {
        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_STRING,
            .string = string
        };

        return true;
    }

    dy_parse_choice_failed(ctx, tr);

    // TODO: Error

    return false;
}

bool dy_parse_string(struct dy_parser_ctx ctx, dy_string_t *string)
{
    struct dy_parser_transaction_multiple tr = dy_parse_multiple_start(&ctx);

    if (!parse_exactly_one(ctx, '\"')) {
        return false;
    }

    const char *ptr = ctx.text.ptr + ctx.index_in;
    size_t count = 0;

    for (;;) {
        char c;
        if (!get_char(ctx, &c)) {
            return false;
        }

        if (c == '\"') {
            *string = (dy_string_t){
                .ptr = ptr,
                .size = count
            };

            dy_parse_multiple_succeeded(ctx, tr);

            return true;
        }

        ++count;
    }
}

bool parse_expr_further1(struct dy_parser_ctx ctx, struct dy_ast_expr left, struct dy_ast_expr *expr)
{
    struct dy_parser_transaction_choice tr = dy_parse_choice_start(ctx);

    if (parse_elimination(ctx, left, expr)) {
        return true;
    }

    dy_parse_choice_failed(ctx, tr);

    if (parse_positive_value_map(ctx, left, expr)) {
        return true;
    }

    dy_parse_choice_failed(ctx, tr);

    if (parse_negative_value_map(ctx, left, expr)) {
        return true;
    }

    dy_parse_choice_failed(ctx, tr);

    *expr = left;

    return true;
}

bool parse_positive_value_map(struct dy_parser_ctx ctx, struct dy_ast_expr left, struct dy_ast_expr *expr)
{
    struct dy_parser_transaction_multiple tr = dy_parse_multiple_start(&ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT("->"))) {
        return false;
    }

    skip_whitespace_except_newline(ctx);

    if (!parse_expr_further2(ctx, left, INFIX_OP_STRAIGHT_ARROW, expr)) {
        return false;
    }

    dy_parse_multiple_succeeded(ctx, tr);

    return true;
}

bool parse_negative_value_map(struct dy_parser_ctx ctx, struct dy_ast_expr left, struct dy_ast_expr *expr)
{
    struct dy_parser_transaction_multiple tr = dy_parse_multiple_start(&ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT("~>"))) {
        return false;
    }

    skip_whitespace_except_newline(ctx);

    if (!parse_expr_further2(ctx, left, INFIX_OP_SQUIGGLY_ARROW, expr)) {
        return false;
    }

    dy_parse_multiple_succeeded(ctx, tr);

    return true;
}

bool parse_elimination(struct dy_parser_ctx ctx, struct dy_ast_expr left, struct dy_ast_expr *expr)
{
    struct dy_parser_transaction_multiple tr = dy_parse_multiple_start(&ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT("!"))) {
        return false;
    }

    skip_whitespace_except_newline(ctx);

    struct dy_ast_expr map;
    if (!dy_parse_expr(ctx, &map)) {
        return false;
    }

    if (map.tag == DY_AST_EXPR_NEGATIVE_VALUE_MAP) {
        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_VALUE_MAP_ELIM,
            .value_map_elim = {
                .expr = alloc_expr(ctx, left),
                .value_map = map.negative_value_map }
        };

        dy_parse_multiple_succeeded(ctx, tr);

        return true;
    }

    if (map.tag == DY_AST_EXPR_NEGATIVE_TYPE_MAP) {
        *expr = (struct dy_ast_expr){
            .tag = DY_AST_EXPR_TYPE_MAP_ELIM,
            .type_map_elim = {
                .expr = alloc_expr(ctx, left),
                .type_map = map.negative_type_map }
        };

        dy_parse_multiple_succeeded(ctx, tr);

        return true;
    }

    return false;
}

bool parse_expr_further2(struct dy_parser_ctx ctx, struct dy_ast_expr left, enum infix_op op, struct dy_ast_expr *expr)
{
    struct dy_parser_transaction_multiple tr = dy_parse_multiple_start(&ctx);

    struct dy_ast_expr right;
    if (!parse_expr_non_left_recursive(ctx, &right)) {
        return false;
    }

    skip_whitespace_except_newline(ctx);

    if (!parse_expr_further3(ctx, left, op, right, expr)) {
        return false;
    }

    dy_parse_multiple_succeeded(ctx, tr);

    return true;
}

bool parse_expr_further3(struct dy_parser_ctx ctx, struct dy_ast_expr left, enum infix_op op, struct dy_ast_expr right, struct dy_ast_expr *expr)
{
    struct dy_parser_transaction_choice tr = dy_parse_choice_start(ctx);

    if (parse_elimination_further(ctx, left, op, right, expr)) {
        return true;
    }

    dy_parse_choice_failed(ctx, tr);

    if (parse_positive_value_map_further(ctx, left, op, right, expr)) {
        return true;
    }

    dy_parse_choice_failed(ctx, tr);

    if (parse_negative_value_map_further(ctx, left, op, right, expr)) {
        return true;
    }

    dy_parse_choice_failed(ctx, tr);

    *expr = combine_infix(ctx, left, op, right);

    return true;
}

bool parse_elimination_further(struct dy_parser_ctx ctx, struct dy_ast_expr left, enum infix_op op, struct dy_ast_expr right, struct dy_ast_expr *expr)
{
    struct dy_parser_transaction_multiple tr = dy_parse_multiple_start(&ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT("!"))) {
        return false;
    }

    struct dy_ast_expr rest;
    if (!parse_elimination(ctx, right, &rest)) {
        return false;
    }

    if (!parse_expr_further3(ctx, left, op, rest, expr)) {
        return false;
    }

    dy_parse_multiple_succeeded(ctx, tr);

    return true;
}

bool parse_positive_value_map_further(struct dy_parser_ctx ctx, struct dy_ast_expr left, enum infix_op op, struct dy_ast_expr right, struct dy_ast_expr *expr)
{
    struct dy_parser_transaction_multiple tr = dy_parse_multiple_start(&ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT("->"))) {
        return false;
    }

    if (!parse_expr_further4(ctx, left, op, right, INFIX_OP_STRAIGHT_ARROW, expr)) {
        return false;
    }

    dy_parse_multiple_succeeded(ctx, tr);

    return true;
}

bool parse_negative_value_map_further(struct dy_parser_ctx ctx, struct dy_ast_expr left, enum infix_op op, struct dy_ast_expr right, struct dy_ast_expr *expr)
{
    struct dy_parser_transaction_multiple tr = dy_parse_multiple_start(&ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT("~>"))) {
        return false;
    }

    if (!parse_expr_further4(ctx, left, op, right, INFIX_OP_SQUIGGLY_ARROW, expr)) {
        return false;
    }

    dy_parse_multiple_succeeded(ctx, tr);

    return true;
}

bool parse_expr_further4(struct dy_parser_ctx ctx, struct dy_ast_expr left, enum infix_op left_op, struct dy_ast_expr right, enum infix_op right_op, struct dy_ast_expr *expr)
{
    if (left_op_is_first(left_op, right_op)) {
        struct dy_ast_expr new_left = combine_infix(ctx, left, left_op, right);

        return parse_expr_further2(ctx, new_left, right_op, expr);
    } else {
        struct dy_ast_expr new_right;
        if (!parse_expr_further2(ctx, right, right_op, &new_right)) {
            return false;
        }

        *expr = combine_infix(ctx, left, left_op, new_right);

        return true;
    }
}

bool parse_bare_list(struct dy_parser_ctx ctx, struct dy_ast_list *list)
{
    struct dy_parser_transaction_multiple tr = dy_parse_multiple_start(&ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT("{"))) {
        return false;
    }

    skip_whitespace(ctx);

    dy_array_t *exprs = dy_array_create(ctx.allocator, sizeof(struct dy_ast_expr), 4);

    for (;;) {
        struct dy_ast_expr expr;
        if (!dy_parse_expr(ctx, &expr)) {
            return false;
        }

        dy_array_add(exprs, &expr);

        skip_whitespace_except_newline(ctx);

        struct dy_parser_transaction_choice ctr = dy_parse_choice_start(ctx);

        if (dy_parse_literal(ctx, DY_STR_LIT("}"))) {
            break;
        }

        dy_parse_choice_failed(ctx, ctr);

        if (dy_parse_literal(ctx, DY_STR_LIT(","))) {
            skip_whitespace(ctx);
            continue;
        }

        if (dy_parse_literal(ctx, DY_STR_LIT("\n"))) {
            skip_whitespace(ctx);

            if (dy_parse_literal(ctx, DY_STR_LIT("}"))) {
                break;
            } else {
                continue;
            }
        }

        dy_parse_choice_failed(ctx, ctr);

        return false;
    }

    *list = (struct dy_ast_list){
        .exprs = dy_array_buffer(exprs),
        .num_exprs = dy_array_size(exprs)
    };

    dy_parse_multiple_succeeded(ctx, tr);

    dy_array_destroy_instance(exprs);

    return true;
}

bool dy_parse_list(struct dy_parser_ctx ctx, struct dy_ast_list *list)
{
    struct dy_parser_transaction_multiple tr = dy_parse_multiple_start(&ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT("list"))) {
        return false;
    }

    skip_whitespace(ctx);

    if (parse_bare_list(ctx, list)) {
        dy_parse_multiple_succeeded(ctx, tr);
        return true;
    } else {
        return false;
    }
}

bool dy_parse_choice(struct dy_parser_ctx ctx, struct dy_ast_list *choice)
{
    struct dy_parser_transaction_multiple tr = dy_parse_multiple_start(&ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT("choice"))) {
        return false;
    }

    skip_whitespace(ctx);

    if (parse_bare_list(ctx, choice)) {
        dy_parse_multiple_succeeded(ctx, tr);
        return true;
    } else {
        return false;
    }
}

bool dy_parse_try_block(struct dy_parser_ctx ctx, struct dy_ast_list *try_block)
{
    struct dy_parser_transaction_multiple tr = dy_parse_multiple_start(&ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT("try"))) {
        return false;
    }

    skip_whitespace(ctx);

    if (parse_bare_list(ctx, try_block)) {
        dy_parse_multiple_succeeded(ctx, tr);
        return true;
    } else {
        return false;
    }
}

bool dy_parse_do_block(struct dy_parser_ctx ctx, struct dy_ast_do_block *do_block)
{
    struct dy_parser_transaction_multiple tr = dy_parse_multiple_start(&ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT("do"))) {
        return false;
    }

    skip_whitespace(ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT("{"))) {
        return false;
    }

    skip_whitespace(ctx);

    dy_array_t *stmnts = dy_array_create(ctx.allocator, sizeof(struct dy_ast_do_block_stmnt), 4);

    for (;;) {
        struct dy_ast_do_block_stmnt stmnt;
        if (!parse_do_block_stmnt(ctx, &stmnt)) {
            return false;
        }

        dy_array_add(stmnts, &stmnt);

        skip_whitespace_except_newline(ctx);

        struct dy_parser_transaction_choice ctr = dy_parse_choice_start(ctx);

        if (dy_parse_literal(ctx, DY_STR_LIT("}"))) {
            break;
        }

        dy_parse_choice_failed(ctx, ctr);

        if (dy_parse_literal(ctx, DY_STR_LIT(";"))) {
            skip_whitespace(ctx);
            continue;
        }

        dy_parse_choice_failed(ctx, ctr);

        if (dy_parse_literal(ctx, DY_STR_LIT("\n"))) {
            skip_whitespace(ctx);

            if (dy_parse_literal(ctx, DY_STR_LIT("}"))) {
                break;
            } else {
                continue;
            }
        }

        dy_parse_choice_failed(ctx, ctr);

        return false;
    }

    struct dy_ast_do_block_stmnt last_stmnt;
    dy_array_pop(stmnts, &last_stmnt);

    if (last_stmnt.tag != DY_AST_DO_BLOCK_STMNT_BARE_EXPR) {
        // TODO: Error
        return false;
    }

    *do_block = (struct dy_ast_do_block){
        .stmnts = dy_array_buffer(stmnts),
        .num_stmnts = dy_array_size(stmnts),
        .last_expr = alloc_expr(ctx, last_stmnt.expr)
    };

    dy_parse_multiple_succeeded(ctx, tr);

    dy_array_destroy_instance(stmnts);

    return true;
}

bool dy_parse_file(struct dy_parser_ctx ctx, struct dy_ast_do_block *do_block)
{
    struct dy_parser_transaction_multiple tr = dy_parse_multiple_start(&ctx);

    skip_whitespace(ctx);

    dy_array_t *stmnts = dy_array_create(ctx.allocator, sizeof(struct dy_ast_do_block_stmnt), 4);

    for (;;) {
        struct dy_ast_do_block_stmnt stmnt;
        if (!parse_do_block_stmnt(ctx, &stmnt)) {
            return false;
        }

        dy_array_add(stmnts, &stmnt);

        skip_whitespace_except_newline(ctx);

        if (is_at_end(ctx)) {
            break;
        }

        struct dy_parser_transaction_choice ctr = dy_parse_choice_start(ctx);

        if (dy_parse_literal(ctx, DY_STR_LIT(";"))) {
            skip_whitespace(ctx);
            continue;
        }

        dy_parse_choice_failed(ctx, ctr);

        if (dy_parse_literal(ctx, DY_STR_LIT("\n"))) {
            skip_whitespace(ctx);

            if (is_at_end(ctx)) {
                break;
            } else {
                continue;
            }
        }

        dy_parse_choice_failed(ctx, ctr);

        return false;
    }

    struct dy_ast_do_block_stmnt last_stmnt;
    dy_array_pop(stmnts, &last_stmnt);

    if (last_stmnt.tag != DY_AST_DO_BLOCK_STMNT_BARE_EXPR) {
        // TODO: Error
        return false;
    }

    *do_block = (struct dy_ast_do_block){
        .stmnts = dy_array_buffer(stmnts),
        .num_stmnts = dy_array_size(stmnts),
        .last_expr = alloc_expr(ctx, last_stmnt.expr)
    };

    dy_parse_multiple_succeeded(ctx, tr);

    dy_array_destroy_instance(stmnts);

    return true;
}

bool parse_do_block_stmnt(struct dy_parser_ctx ctx, struct dy_ast_do_block_stmnt *stmnt)
{
    struct dy_parser_transaction_choice tr = dy_parse_choice_start(ctx);

    if (parse_let_stmnt(ctx, stmnt)) {
        return true;
    }

    dy_parse_choice_failed(ctx, tr);

    if (parse_stmnt_equal(ctx, stmnt)) {
        return true;
    }

    dy_parse_choice_failed(ctx, tr);

    struct dy_ast_expr expr;
    if (dy_parse_expr(ctx, &expr)) {
        *stmnt = (struct dy_ast_do_block_stmnt){
            .tag = DY_AST_DO_BLOCK_STMNT_BARE_EXPR,
            .expr = expr,
        };

        return true;
    }

    dy_parse_choice_failed(ctx, tr);

    // TODO: Error

    return false;
}

bool parse_let_stmnt(struct dy_parser_ctx ctx, struct dy_ast_do_block_stmnt *stmnt)
{
    struct dy_parser_transaction_multiple tr = dy_parse_multiple_start(&ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT("let"))) {
        return false;
    }

    skip_whitespace(ctx);

    dy_string_t name;
    if (!dy_parse_variable(ctx, &name)) {
        return false;
    }

    skip_whitespace(ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT("="))) {
        return false;
    }

    skip_whitespace(ctx);

    struct dy_ast_expr expr;
    if (!dy_parse_expr(ctx, &expr)) {
        return false;
    }

    dy_parse_multiple_succeeded(ctx, tr);

    *stmnt = (struct dy_ast_do_block_stmnt){
        .tag = DY_AST_DO_BLOCK_STMNT_LET,
        .let_arg_name = name,
        .expr = expr
    };

    return true;
}

bool parse_stmnt_equal(struct dy_parser_ctx ctx, struct dy_ast_do_block_stmnt *stmnt)
{
    struct dy_parser_transaction_multiple tr = dy_parse_multiple_start(&ctx);

    struct dy_ast_expr e1;
    if (!dy_parse_expr(ctx, &e1)) {
        return false;
    }

    skip_whitespace(ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT("="))) {
        return false;
    }

    skip_whitespace(ctx);

    struct dy_ast_expr e2;
    if (!dy_parse_expr(ctx, &e2)) {
        return false;
    }

    dy_parse_multiple_succeeded(ctx, tr);

    *stmnt = (struct dy_ast_do_block_stmnt){
        .tag = DY_AST_DO_BLOCK_STMNT_EQUALITY,
        .left_expr = e1,
        .expr = e2
    };

    return true;
}

bool parse_expr_parenthesized(struct dy_parser_ctx ctx, struct dy_ast_expr *expr)
{
    struct dy_parser_transaction_multiple tr = dy_parse_multiple_start(&ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT("("))) {
        return false;
    }

    skip_whitespace(ctx);

    if (!dy_parse_expr(ctx, expr)) {
        return false;
    }

    skip_whitespace(ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT(")"))) {
        return false;
    }

    dy_parse_multiple_succeeded(ctx, tr);

    return true;
}

bool parse_type_map(struct dy_parser_ctx ctx, dy_string_t arrow_literal, struct dy_ast_type_map *type_map)
{
    struct dy_parser_transaction_multiple tr = dy_parse_multiple_start(&ctx);

    struct dy_ast_arg arg;
    if (!parse_arg(ctx, &arg)) {
        return false;
    }

    skip_whitespace(ctx);

    if (!dy_parse_literal(ctx, arrow_literal)) {
        return false;
    }

    skip_whitespace(ctx);

    struct dy_ast_expr expr;
    if (!dy_parse_expr(ctx, &expr)) {
        return false;
    }

    *type_map = (struct dy_ast_type_map){
        .arg = arg,
        .expr = alloc_expr(ctx, expr)
    };

    dy_parse_multiple_succeeded(ctx, tr);

    return true;
}

bool dy_parse_positive_type_map(struct dy_parser_ctx ctx, struct dy_ast_type_map *type_map)
{
    return parse_type_map(ctx, DY_STR_LIT("->"), type_map);
}

bool dy_parse_negative_type_map(struct dy_parser_ctx ctx, struct dy_ast_type_map *type_map)
{
    return parse_type_map(ctx, DY_STR_LIT("~>"), type_map);
}

bool parse_arg(struct dy_parser_ctx ctx, struct dy_ast_arg *arg)
{
    struct dy_parser_transaction_multiple tr = dy_parse_multiple_start(&ctx);

    bool has_name;
    dy_string_t name;
    if (dy_parse_literal(ctx, DY_STR_LIT("_"))) {
        has_name = false;
    } else {
        has_name = true;

        if (!dy_parse_variable(ctx, &name)) {
            return false;
        }
    }

    skip_whitespace(ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT("["))) {
        return false;
    }

    skip_whitespace(ctx);

    struct dy_ast_expr type;
    if (!dy_parse_expr(ctx, &type)) {
        return false;
    }

    skip_whitespace(ctx);

    if (!dy_parse_literal(ctx, DY_STR_LIT("]"))) {
        return false;
    }

    *arg = (struct dy_ast_arg){
        .name = name,
        .has_name = has_name,
        .type = alloc_expr(ctx, type)
    };

    dy_parse_multiple_succeeded(ctx, tr);

    return true;
}

struct dy_ast_expr combine_infix(struct dy_parser_ctx ctx, struct dy_ast_expr left, enum infix_op op, struct dy_ast_expr right)
{
    switch (op) {
    case INFIX_OP_STRAIGHT_ARROW:
        return (struct dy_ast_expr){
            .tag = DY_AST_EXPR_POSITIVE_VALUE_MAP,
            .positive_value_map = {
                .e1 = alloc_expr(ctx, left),
                .e2 = alloc_expr(ctx, right),
            }
        };
    case INFIX_OP_SQUIGGLY_ARROW:
        return (struct dy_ast_expr){
            .tag = DY_AST_EXPR_NEGATIVE_VALUE_MAP,
            .negative_value_map = {
                .e1 = alloc_expr(ctx, left),
                .e2 = alloc_expr(ctx, right),
            }
        };
    }

    DY_IMPOSSIBLE_ENUM();
}

bool left_op_is_first(enum infix_op left, enum infix_op right)
{
    if (op_precedence(left) > op_precedence(right)) {
        return true;
    }

    if (op_precedence(left) == op_precedence(right)) {
        dy_assert(op_assoc(left) == op_assoc(right));

        switch (op_assoc(left)) {
        case ASSOC_LEFT: return true;
        case ASSOC_RIGHT: return false;
        }

        DY_IMPOSSIBLE_ENUM();
    }

    return false;
}

int op_precedence(enum infix_op op)
{
    switch (op) {
    case INFIX_OP_STRAIGHT_ARROW:
    case INFIX_OP_SQUIGGLY_ARROW:
        return 500;
    }

    DY_IMPOSSIBLE_ENUM();
}

enum assoc op_assoc(enum infix_op op)
{
    switch (op) {
    case INFIX_OP_STRAIGHT_ARROW:
    case INFIX_OP_SQUIGGLY_ARROW:
        return ASSOC_LEFT;
    }

    DY_IMPOSSIBLE_ENUM();
}

void skip_whitespace(struct dy_parser_ctx ctx)
{
    struct dy_parser_transaction_multiple tr = dy_parse_multiple_start(&ctx);

    for (;;) {
        struct dy_parser_transaction_choice ctr = dy_parse_choice_start(ctx);

        if (parse_one_of(ctx, DY_STR_LIT(" \n\t"))) {
            continue;
        }

        dy_parse_choice_failed(ctx, ctr);

        if (skip_block_comment(ctx)) {
            continue;
        }

        dy_parse_choice_failed(ctx, ctr);

        if (skip_line_comment(ctx)) {
            continue;
        }

        dy_parse_choice_failed(ctx, ctr);

        dy_parse_multiple_succeeded(ctx, tr);

        return;
    }
}

void skip_whitespace_except_newline(struct dy_parser_ctx ctx)
{
    struct dy_parser_transaction_multiple tr = dy_parse_multiple_start(&ctx);

    for (;;) {
        struct dy_parser_transaction_choice ctr = dy_parse_choice_start(ctx);

        if (parse_one_of(ctx, DY_STR_LIT(" \t"))) {
            continue;
        }

        dy_parse_choice_failed(ctx, ctr);

        if (skip_block_comment(ctx)) {
            continue;
        }

        dy_parse_choice_failed(ctx, ctr);

        if (skip_line_comment(ctx)) {
            continue;
        }

        dy_parse_choice_failed(ctx, ctr);

        dy_parse_multiple_succeeded(ctx, tr);

        return;
    }
}

struct dy_ast_expr *alloc_expr(struct dy_parser_ctx ctx, struct dy_ast_expr expr)
{
    return dy_alloc(&expr, sizeof expr, ctx.allocator);
}

bool is_at_end(struct dy_parser_ctx ctx)
{
    return ctx.index_in >= ctx.text.size;
}

bool parse_one_of(struct dy_parser_ctx ctx, dy_string_t chars)
{
    if (is_at_end(ctx)) {
        return false;
    }

    char c = ctx.text.ptr[ctx.index_in];

    if (!dy_string_matches_one_of(c, chars)) {
        return false;
    }

    *ctx.index_out = ctx.index_in + 1;

    return true;
}

bool parse_exactly_one(struct dy_parser_ctx ctx, char c)
{
    if (is_at_end(ctx)) {
        return false;
    }

    char c2 = ctx.text.ptr[ctx.index_in];

    if (c != c2) {
        return false;
    }

    *ctx.index_out = ctx.index_in + 1;

    return true;
}

bool get_char(struct dy_parser_ctx ctx, char *c)
{
    if (is_at_end(ctx)) {
        return false;
    }

    *c = ctx.text.ptr[ctx.index_in];

    *ctx.index_out = ctx.index_in + 1;

    return true;
}
