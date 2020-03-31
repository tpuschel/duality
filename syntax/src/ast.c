/*
 * Copyright 2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <duality/syntax/ast.h>

#include <duality/support/assert.h>

struct dy_ast_expr *dy_ast_expr_new(dy_obj_pool_t *pool, struct dy_ast_expr expr)
{
    return dy_obj_pool_new(pool, &expr);
}

struct dy_ast_do_block *dy_ast_do_block_new(dy_obj_pool_t *pool, struct dy_ast_do_block do_block)
{
    union {
        struct dy_ast_expr e;
        struct dy_ast_do_block d;
    } dummy = {
        .d = do_block
    };

    return dy_obj_pool_new(pool, &dummy);
}

struct dy_ast_list *dy_ast_list_new(dy_obj_pool_t *pool, struct dy_ast_list list)
{
    union {
        struct dy_ast_expr e;
        struct dy_ast_list l;
    } dummy = {
        .l = list
    };

    return dy_obj_pool_new(pool, &dummy);
}

struct dy_ast_expr *dy_ast_expr_retain_ptr(dy_obj_pool_t *pool, const struct dy_ast_expr *expr)
{
    return dy_obj_pool_retain(pool, expr);
}

struct dy_ast_expr dy_ast_expr_retain(dy_obj_pool_t *pool, struct dy_ast_expr expr)
{
    switch (expr.tag) {
    case DY_AST_EXPR_POSITIVE_TYPE_MAP:
        if (expr.positive_type_map.arg.has_type) {
            dy_ast_expr_retain_ptr(pool, expr.positive_type_map.arg.type);
        }
        dy_ast_expr_retain_ptr(pool, expr.positive_type_map.expr);
        return expr;
    case DY_AST_EXPR_NEGATIVE_TYPE_MAP:
        if (expr.negative_type_map.arg.has_type) {
            dy_ast_expr_retain_ptr(pool, expr.negative_type_map.arg.type);
        }
        dy_ast_expr_retain_ptr(pool, expr.negative_type_map.expr);
        return expr;
    case DY_AST_EXPR_LIST:
        dy_ast_list_retain(pool, expr.list);
        return expr;
    case DY_AST_EXPR_POSITIVE_EXPR_MAP:
        dy_ast_expr_retain_ptr(pool, expr.positive_expr_map.e1);
        dy_ast_expr_retain_ptr(pool, expr.positive_expr_map.e2);
        return expr;
    case DY_AST_EXPR_NEGATIVE_EXPR_MAP:
        dy_ast_expr_retain_ptr(pool, expr.negative_expr_map.e1);
        dy_ast_expr_retain_ptr(pool, expr.negative_expr_map.e2);
        return expr;
    case DY_AST_EXPR_DO_BLOCK:
        dy_ast_do_block_retain(pool, expr.do_block);
        return expr;
    case DY_AST_EXPR_CHOICE:
        dy_ast_list_retain(pool, expr.choice);
        return expr;
    case DY_AST_EXPR_TRY_BLOCK:
        dy_ast_list_retain(pool, expr.try_block);
        return expr;
    case DY_AST_EXPR_EXPR_MAP_ELIM:
        dy_ast_expr_retain_ptr(pool, expr.expr_map_elim.expr);
        dy_ast_expr_retain_ptr(pool, expr.expr_map_elim.expr_map.e1);
        dy_ast_expr_retain_ptr(pool, expr.expr_map_elim.expr_map.e2);
        return expr;
    case DY_AST_EXPR_TYPE_MAP_ELIM:
        dy_ast_expr_retain_ptr(pool, expr.type_map_elim.expr);
        if (expr.type_map_elim.type_map.arg.has_type) {
            dy_ast_expr_retain_ptr(pool, expr.type_map_elim.type_map.arg.type);
        }
        dy_ast_expr_retain_ptr(pool, expr.type_map_elim.type_map.expr);
        return expr;
    case DY_AST_EXPR_VARIABLE:
        return expr;
    case DY_AST_EXPR_JUXTAPOSITION:
        dy_ast_expr_retain_ptr(pool, expr.juxtaposition.left);
        dy_ast_expr_retain_ptr(pool, expr.juxtaposition.right);
        return expr;
    case DY_AST_EXPR_STRING:
        return expr;
    case DY_AST_EXPR_TYPE_STRING:
        return expr;
    case DY_AST_EXPR_ALL:
        return expr;
    case DY_AST_EXPR_NOTHING:
        return expr;
    }

    DY_IMPOSSIBLE_ENUM();
}

void dy_ast_expr_release_ptr(dy_obj_pool_t *pool, const struct dy_ast_expr *expr)
{
    struct dy_ast_expr e = *expr;
    if (dy_obj_pool_release(pool, expr) == 0) {
        dy_ast_expr_release(pool, e);
    }
}

void dy_ast_expr_release(dy_obj_pool_t *pool, struct dy_ast_expr expr)
{
    switch (expr.tag) {
    case DY_AST_EXPR_POSITIVE_TYPE_MAP:
        if (expr.positive_type_map.arg.has_type) {
            dy_ast_expr_release_ptr(pool, expr.positive_type_map.arg.type);
        }
        dy_ast_expr_release_ptr(pool, expr.positive_type_map.expr);
        return;
    case DY_AST_EXPR_NEGATIVE_TYPE_MAP:
        if (expr.negative_type_map.arg.has_type) {
            dy_ast_expr_release_ptr(pool, expr.negative_type_map.arg.type);
        }
        dy_ast_expr_release_ptr(pool, expr.negative_type_map.expr);
        return;
    case DY_AST_EXPR_LIST:
        dy_ast_list_release(pool, expr.list);
        return;
    case DY_AST_EXPR_POSITIVE_EXPR_MAP:
        dy_ast_expr_release_ptr(pool, expr.positive_expr_map.e1);
        dy_ast_expr_release_ptr(pool, expr.positive_expr_map.e2);
        return;
    case DY_AST_EXPR_NEGATIVE_EXPR_MAP:
        dy_ast_expr_release_ptr(pool, expr.negative_expr_map.e1);
        dy_ast_expr_release_ptr(pool, expr.negative_expr_map.e2);
        return;
    case DY_AST_EXPR_DO_BLOCK:
        dy_ast_do_block_release(pool, expr.do_block);
        return;
    case DY_AST_EXPR_CHOICE:
        dy_ast_list_release(pool, expr.choice);
        return;
    case DY_AST_EXPR_TRY_BLOCK:
        dy_ast_list_release(pool, expr.try_block);
        return;
    case DY_AST_EXPR_EXPR_MAP_ELIM:
        dy_ast_expr_release_ptr(pool, expr.expr_map_elim.expr);
        dy_ast_expr_release_ptr(pool, expr.expr_map_elim.expr_map.e1);
        dy_ast_expr_release_ptr(pool, expr.expr_map_elim.expr_map.e2);
        return;
    case DY_AST_EXPR_TYPE_MAP_ELIM:
        dy_ast_expr_release_ptr(pool, expr.type_map_elim.expr);
        if (expr.type_map_elim.type_map.arg.has_type) {
            dy_ast_expr_release_ptr(pool, expr.type_map_elim.type_map.arg.type);
        }
        dy_ast_expr_release_ptr(pool, expr.type_map_elim.type_map.expr);
        return;
    case DY_AST_EXPR_VARIABLE:
        return;
    case DY_AST_EXPR_JUXTAPOSITION:
        dy_ast_expr_release_ptr(pool, expr.juxtaposition.left);
        dy_ast_expr_release_ptr(pool, expr.juxtaposition.right);
        return;
    case DY_AST_EXPR_STRING:
        return;
    case DY_AST_EXPR_TYPE_STRING:
        return;
    case DY_AST_EXPR_ALL:
        return;
    case DY_AST_EXPR_NOTHING:
        return;
    }

    DY_IMPOSSIBLE_ENUM();
}

void dy_ast_do_block_release_ptr(dy_obj_pool_t *pool, const struct dy_ast_do_block *do_block)
{
    struct dy_ast_do_block d = *do_block;
    if (dy_obj_pool_release(pool, do_block) == 0) {
        dy_ast_do_block_release(pool, d);
    }
}

void dy_ast_do_block_retain_ptr(dy_obj_pool_t *pool, const struct dy_ast_do_block *do_block)
{
    dy_obj_pool_retain(pool, do_block);
}

void dy_ast_do_block_retain(dy_obj_pool_t *pool, struct dy_ast_do_block do_block)
{
    switch (do_block.tag) {
    case DY_AST_DO_BLOCK_END_EXPR:
        dy_ast_expr_retain_ptr(pool, do_block.end_expr);
        return;
    case DY_AST_DO_BLOCK_EQUALITY:
        dy_ast_expr_retain_ptr(pool, do_block.equality.e1);
        dy_ast_expr_retain_ptr(pool, do_block.equality.e2);
        dy_ast_do_block_retain_ptr(pool, do_block.equality.rest);
        return;
    case DY_AST_DO_BLOCK_IGNORED_EXPR:
        dy_ast_expr_retain_ptr(pool, do_block.ignored_expr.expr);
        dy_ast_do_block_retain_ptr(pool, do_block.ignored_expr.rest);
        return;
    case DY_AST_DO_BLOCK_LET:
        dy_ast_expr_retain_ptr(pool, do_block.let.expr);
        dy_ast_do_block_retain_ptr(pool, do_block.let.rest);
        return;
    }

    DY_IMPOSSIBLE_ENUM();
}

void dy_ast_do_block_release(dy_obj_pool_t *pool, struct dy_ast_do_block do_block)
{
    switch (do_block.tag) {
    case DY_AST_DO_BLOCK_END_EXPR:
        dy_ast_expr_release_ptr(pool, do_block.end_expr);
        return;
    case DY_AST_DO_BLOCK_EQUALITY:
        dy_ast_expr_release_ptr(pool, do_block.equality.e1);
        dy_ast_expr_release_ptr(pool, do_block.equality.e2);
        dy_ast_do_block_release_ptr(pool, do_block.equality.rest);
        return;
    case DY_AST_DO_BLOCK_IGNORED_EXPR:
        dy_ast_expr_release_ptr(pool, do_block.ignored_expr.expr);
        dy_ast_do_block_release_ptr(pool, do_block.ignored_expr.rest);
        return;
    case DY_AST_DO_BLOCK_LET:
        dy_ast_expr_release_ptr(pool, do_block.let.expr);
        dy_ast_do_block_release_ptr(pool, do_block.let.rest);
        return;
    }

    DY_IMPOSSIBLE_ENUM();
}

void dy_ast_list_retain(dy_obj_pool_t *pool, struct dy_ast_list list)
{
    dy_ast_expr_retain_ptr(pool, list.expr);
    if (list.next_or_null) {
        dy_ast_list_retain_ptr(pool, list.next_or_null);
    }
}

void dy_ast_list_retain_ptr(dy_obj_pool_t *pool, const struct dy_ast_list *list)
{
    dy_obj_pool_retain(pool, list);
}

void dy_ast_list_release(dy_obj_pool_t *pool, struct dy_ast_list list)
{
    dy_ast_expr_release_ptr(pool, list.expr);
    if (list.next_or_null) {
        dy_ast_list_release_ptr(pool, list.next_or_null);
    }
}

void dy_ast_list_release_ptr(dy_obj_pool_t *pool, const struct dy_ast_list *list)
{
    struct dy_ast_list l = *list;
    if (dy_obj_pool_release(pool, list) == 0) {
        dy_ast_list_release(pool, l);
    }
}
