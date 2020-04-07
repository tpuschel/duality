/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_AST_H
#define DY_AST_H

#include <duality/support/string.h>
#include <duality/support/range.h>
#include <duality/support/obj_pool.h>

#include <duality/syntax/api.h>

struct dy_ast_expr;

struct dy_ast_arg {
    dy_string_t name;
    bool has_name;
    const struct dy_ast_expr *type;
    bool has_type;
};

struct dy_ast_expr_map {
    const struct dy_ast_expr *e1;
    const struct dy_ast_expr *e2;
    bool is_implicit;
};

struct dy_ast_type_map {
    struct dy_ast_arg arg;
    const struct dy_ast_expr *expr;
    bool is_implicit;
};

struct dy_ast_do_block_equality {
    const struct dy_ast_expr *e1;
    const struct dy_ast_expr *e2;
    const struct dy_ast_do_block *rest;
};

struct dy_ast_do_block_let {
    dy_string_t arg_name;
    const struct dy_ast_expr *expr;
    const struct dy_ast_do_block *rest;
};

struct dy_ast_do_block_ignored_expr {
    const struct dy_ast_expr *expr;
    const struct dy_ast_do_block *rest;
};

enum dy_ast_do_block_tag {
    DY_AST_DO_BLOCK_EQUALITY,
    DY_AST_DO_BLOCK_LET,
    DY_AST_DO_BLOCK_IGNORED_EXPR,
    DY_AST_DO_BLOCK_END_EXPR
};

struct dy_ast_do_block {
    union {
        struct dy_ast_do_block_equality equality;
        struct dy_ast_do_block_let let;
        struct dy_ast_do_block_ignored_expr ignored_expr;
        const struct dy_ast_expr *end_expr;
    };

    enum dy_ast_do_block_tag tag;
};

struct dy_ast_list {
    const struct dy_ast_expr *expr;
    const struct dy_ast_list *next_or_null;
};

struct dy_ast_expr_map_elim {
    const struct dy_ast_expr *expr;
    struct dy_ast_expr_map expr_map;
};

struct dy_ast_type_map_elim {
    const struct dy_ast_expr *expr;
    struct dy_ast_type_map type_map;
};

struct dy_ast_juxtaposition {
    const struct dy_ast_expr *left;
    const struct dy_ast_expr *right;
};

struct dy_ast_recursion {
    struct dy_ast_arg arg;
    const struct dy_ast_expr *expr;
};

enum dy_ast_expr_tag {
    DY_AST_EXPR_VARIABLE,
    DY_AST_EXPR_LIST,
    DY_AST_EXPR_CHOICE,
    DY_AST_EXPR_TRY_BLOCK,
    DY_AST_EXPR_POSITIVE_EXPR_MAP,
    DY_AST_EXPR_NEGATIVE_EXPR_MAP,
    DY_AST_EXPR_POSITIVE_TYPE_MAP,
    DY_AST_EXPR_NEGATIVE_TYPE_MAP,
    DY_AST_EXPR_EXPR_MAP_ELIM,
    DY_AST_EXPR_TYPE_MAP_ELIM,
    DY_AST_EXPR_DO_BLOCK,
    DY_AST_EXPR_ALL,
    DY_AST_EXPR_NOTHING,
    DY_AST_EXPR_STRING,
    DY_AST_EXPR_TYPE_STRING,
    DY_AST_EXPR_JUXTAPOSITION,
    DY_AST_EXPR_POSITIVE_RECURSION,
    DY_AST_EXPR_NEGATIVE_RECURSION
};

struct dy_ast_expr {
    struct dy_range text_range;

    union {
        dy_string_t string;
        dy_string_t variable;
        struct dy_ast_expr_map positive_expr_map;
        struct dy_ast_expr_map negative_expr_map;
        struct dy_ast_type_map positive_type_map;
        struct dy_ast_type_map negative_type_map;
        struct dy_ast_expr_map_elim expr_map_elim;
        struct dy_ast_type_map_elim type_map_elim;
        struct dy_ast_do_block do_block;
        struct dy_ast_list list;
        struct dy_ast_list try_block;
        struct dy_ast_list choice;
        struct dy_ast_juxtaposition juxtaposition;
        struct dy_ast_recursion positive_recursion;
        struct dy_ast_recursion negative_recursion;
    };

    enum dy_ast_expr_tag tag;
};

DY_SYNTAX_API struct dy_ast_expr *dy_ast_expr_new(dy_obj_pool_t *pool, struct dy_ast_expr expr);

DY_SYNTAX_API struct dy_ast_expr dy_ast_expr_retain(dy_obj_pool_t *pool, struct dy_ast_expr expr);

DY_SYNTAX_API struct dy_ast_expr *dy_ast_expr_retain_ptr(dy_obj_pool_t *pool, const struct dy_ast_expr *expr);

DY_SYNTAX_API void dy_ast_expr_release(dy_obj_pool_t *pool, struct dy_ast_expr expr);

DY_SYNTAX_API void dy_ast_expr_release_ptr(dy_obj_pool_t *pool, const struct dy_ast_expr *expr);

DY_SYNTAX_API struct dy_ast_do_block *dy_ast_do_block_new(dy_obj_pool_t *pool, struct dy_ast_do_block do_block);

DY_SYNTAX_API void dy_ast_do_block_retain(dy_obj_pool_t *pool, struct dy_ast_do_block do_block);

DY_SYNTAX_API void dy_ast_do_block_retain_ptr(dy_obj_pool_t *pool, const struct dy_ast_do_block *do_block);

DY_SYNTAX_API void dy_ast_do_block_release(dy_obj_pool_t *pool, struct dy_ast_do_block do_block);

DY_SYNTAX_API void dy_ast_do_block_release_ptr(dy_obj_pool_t *pool, const struct dy_ast_do_block *do_block);

DY_SYNTAX_API struct dy_ast_list *dy_ast_list_new(dy_obj_pool_t *pool, struct dy_ast_list list);

DY_SYNTAX_API void dy_ast_list_retain(dy_obj_pool_t *pool, struct dy_ast_list list);

DY_SYNTAX_API void dy_ast_list_retain_ptr(dy_obj_pool_t *pool, const struct dy_ast_list *list);

DY_SYNTAX_API void dy_ast_list_release(dy_obj_pool_t *pool, struct dy_ast_list list);

DY_SYNTAX_API void dy_ast_list_release_ptr(dy_obj_pool_t *pool, const struct dy_ast_list *list);

#endif // DY_AST_H
