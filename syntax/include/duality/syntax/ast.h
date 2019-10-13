/*
 * Copyright 2017-2019 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_AST_H
#define DY_AST_H

#include <duality/support/string.h>

struct dy_ast_expr;

struct dy_ast_arg {
    dy_string_t name;
    bool has_name;
    const struct dy_ast_expr *type;
};

struct dy_ast_value_map {
    const struct dy_ast_expr *e1;
    const struct dy_ast_expr *e2;
};

struct dy_ast_type_map {
    struct dy_ast_arg arg;
    const struct dy_ast_expr *expr;
};

struct dy_ast_do_block_stmnt;

struct dy_ast_do_block {
    const struct dy_ast_do_block_stmnt *stmnts;
    size_t num_stmnts;
    const struct dy_ast_expr *last_expr;
};

struct dy_ast_list {
    const struct dy_ast_expr *exprs;
    size_t num_exprs;
};

struct dy_ast_value_map_elim {
    const struct dy_ast_expr *expr;
    struct dy_ast_value_map value_map;
};

struct dy_ast_type_map_elim {
    const struct dy_ast_expr *expr;
    struct dy_ast_type_map type_map;
};

enum dy_ast_expr_tag {
    DY_AST_EXPR_VARIABLE,
    DY_AST_EXPR_LIST,
    DY_AST_EXPR_CHOICE,
    DY_AST_EXPR_TRY_BLOCK,
    DY_AST_EXPR_POSITIVE_VALUE_MAP,
    DY_AST_EXPR_NEGATIVE_VALUE_MAP,
    DY_AST_EXPR_POSITIVE_TYPE_MAP,
    DY_AST_EXPR_NEGATIVE_TYPE_MAP,
    DY_AST_EXPR_VALUE_MAP_ELIM,
    DY_AST_EXPR_TYPE_MAP_ELIM,
    DY_AST_EXPR_DO_BLOCK,
    DY_AST_EXPR_TYPE_TYPE,
    DY_AST_EXPR_STRING,
    DY_AST_EXPR_TYPE_STRING
};

struct dy_ast_expr {
    union {
        dy_string_t string;
        dy_string_t variable;
        struct dy_ast_value_map positive_value_map;
        struct dy_ast_value_map negative_value_map;
        struct dy_ast_type_map positive_type_map;
        struct dy_ast_type_map negative_type_map;
        struct dy_ast_value_map_elim value_map_elim;
        struct dy_ast_type_map_elim type_map_elim;
        struct dy_ast_do_block do_block;
        struct dy_ast_list list;
        struct dy_ast_list try_block;
        struct dy_ast_list choice;
    };

    enum dy_ast_expr_tag tag;
};

enum dy_ast_do_block_stmnt_tag {
    DY_AST_DO_BLOCK_STMNT_LET,
    DY_AST_DO_BLOCK_STMNT_EQUALITY,
    DY_AST_DO_BLOCK_STMNT_BARE_EXPR
};

struct dy_ast_do_block_stmnt {
    union {
        dy_string_t let_arg_name;
        struct dy_ast_expr left_expr;
    };

    struct dy_ast_expr expr;

    enum dy_ast_do_block_stmnt_tag tag;
};

#endif // DY_AST_H
