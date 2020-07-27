/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "../support/string.h"
#include "../support/range.h"
#include "../support/rc.h"
#include "../support/bail.h"

/**
 * The Abstract Syntax Tree (AST) of Duality.
 */

struct dy_ast_expr;

struct dy_ast_literal {
    struct dy_range text_range;
    dy_string_t value;
};

struct dy_ast_arg {
    struct dy_ast_literal name;
    bool has_name;
    const struct dy_ast_expr *type;
    bool has_type;
};

struct dy_ast_equality_map {
    struct dy_range text_range;
    const struct dy_ast_expr *e1;
    const struct dy_ast_expr *e2;
    bool is_implicit;
};

struct dy_ast_type_map {
    struct dy_range text_range;
    struct dy_ast_arg arg;
    const struct dy_ast_expr *expr;
    bool is_implicit;
};

struct dy_ast_do_block_equality {
    struct dy_range text_range;
    const struct dy_ast_expr *e1;
    const struct dy_ast_expr *e2;
    const struct dy_ast_do_block_body *rest;
};

struct dy_ast_do_block_let {
    struct dy_range text_range;
    struct dy_ast_literal arg_name;
    const struct dy_ast_expr *expr;
    const struct dy_ast_do_block_body *rest;
};

struct dy_ast_do_block_ignored_expr {
    struct dy_range text_range;
    const struct dy_ast_expr *expr;
    const struct dy_ast_do_block_body *rest;
};

struct dy_ast_do_block_def {
    struct dy_ast_literal name;
    const struct dy_ast_expr *expr;
    const struct dy_ast_do_block_body *rest;
};

enum dy_ast_do_block_tag {
    DY_AST_DO_BLOCK_EQUALITY,
    DY_AST_DO_BLOCK_LET,
    DY_AST_DO_BLOCK_IGNORED_EXPR,
    DY_AST_DO_BLOCK_END_EXPR,
    DY_AST_DO_BLOCK_DEF
};

struct dy_ast_do_block_body {
    union {
        struct dy_ast_do_block_equality equality;
        struct dy_ast_do_block_let let;
        struct dy_ast_do_block_ignored_expr ignored_expr;
        const struct dy_ast_expr *end_expr;
        struct dy_ast_do_block_def def;
    };

    enum dy_ast_do_block_tag tag;
};

struct dy_ast_do_block {
    struct dy_range text_range;
    struct dy_ast_do_block_body body;
};

struct dy_ast_list_inner {
    const struct dy_ast_expr *expr;
    const struct dy_ast_list_inner *next_or_null;
};

struct dy_ast_list {
    struct dy_range text_range;
    struct dy_ast_list_inner inner;
};

struct dy_ast_equality_map_elim {
    struct dy_range text_range;
    const struct dy_ast_expr *expr;
    struct dy_ast_equality_map equality_map;
};

struct dy_ast_type_map_elim {
    struct dy_range text_range;
    const struct dy_ast_expr *expr;
    struct dy_ast_type_map type_map;
};

struct dy_ast_juxtaposition {
    struct dy_range text_range;
    const struct dy_ast_expr *left;
    const struct dy_ast_expr *right;
};

struct dy_ast_recursion {
    struct dy_range text_range;
    struct dy_ast_arg arg;
    const struct dy_ast_expr *expr;
};

enum dy_ast_expr_tag {
    DY_AST_EXPR_VARIABLE,
    DY_AST_EXPR_LIST,
    DY_AST_EXPR_CHOICE,
    DY_AST_EXPR_TRY_BLOCK,
    DY_AST_EXPR_POSITIVE_EQUALITY_MAP,
    DY_AST_EXPR_NEGATIVE_EQUALITY_MAP,
    DY_AST_EXPR_POSITIVE_TYPE_MAP,
    DY_AST_EXPR_NEGATIVE_TYPE_MAP,
    DY_AST_EXPR_EQUALITY_MAP_ELIM,
    DY_AST_EXPR_TYPE_MAP_ELIM,
    DY_AST_EXPR_DO_BLOCK,
    DY_AST_EXPR_ALL,
    DY_AST_EXPR_ANY,
    DY_AST_EXPR_JUXTAPOSITION,
    DY_AST_EXPR_POSITIVE_RECURSION,
    DY_AST_EXPR_NEGATIVE_RECURSION,
    DY_AST_EXPR_SYMBOL
};

struct dy_ast_expr {
    union {
        struct dy_ast_literal variable;
        struct dy_ast_literal string;
        struct dy_ast_equality_map positive_equality_map;
        struct dy_ast_equality_map negative_equality_map;
        struct dy_ast_type_map positive_type_map;
        struct dy_ast_type_map negative_type_map;
        struct dy_ast_equality_map_elim equality_map_elim;
        struct dy_ast_type_map_elim type_map_elim;
        struct dy_ast_do_block do_block;
        struct dy_ast_list list;
        struct dy_ast_list try_block;
        struct dy_ast_list choice;
        struct dy_ast_juxtaposition juxtaposition;
        struct dy_ast_recursion positive_recursion;
        struct dy_ast_recursion negative_recursion;
        struct dy_range static_literal_text_range;
    };

    enum dy_ast_expr_tag tag;
};

/** Boring ref-count stuff. */

static const size_t dy_ast_expr_pre_padding = DY_RC_PRE_PADDING(struct dy_ast_expr);
static const size_t dy_ast_expr_post_padding = DY_RC_POST_PADDING(struct dy_ast_expr);

static inline const struct dy_ast_expr *dy_ast_expr_new(struct dy_ast_expr expr);

static inline struct dy_ast_expr dy_ast_expr_retain(struct dy_ast_expr expr);

static inline const struct dy_ast_expr *dy_ast_expr_retain_ptr(const struct dy_ast_expr *expr);

static inline void dy_ast_expr_release(struct dy_ast_expr expr);

static inline void dy_ast_expr_release_ptr(const struct dy_ast_expr *expr);


static const size_t dy_ast_do_block_pre_padding = DY_RC_PRE_PADDING(struct dy_ast_do_block_body);
static const size_t dy_ast_do_block_post_padding = DY_RC_POST_PADDING(struct dy_ast_do_block_body);

static inline const struct dy_ast_do_block_body *dy_ast_do_block_new(struct dy_ast_do_block_body do_block);

static inline void dy_ast_do_block_retain(struct dy_ast_do_block_body do_block);

static inline void dy_ast_do_block_retain_ptr(const struct dy_ast_do_block_body *do_block);

static inline void dy_ast_do_block_release(struct dy_ast_do_block_body do_block);

static inline void dy_ast_do_block_release_ptr(const struct dy_ast_do_block_body *do_block);


static const size_t dy_ast_list_pre_padding = DY_RC_PRE_PADDING(struct dy_ast_list_inner);
static const size_t dy_ast_list_post_padding = DY_RC_POST_PADDING(struct dy_ast_list_inner);

static inline const struct dy_ast_list_inner *dy_ast_list_new(struct dy_ast_list_inner list);

static inline void dy_ast_list_retain(struct dy_ast_list_inner list);

static inline void dy_ast_list_retain_ptr(const struct dy_ast_list_inner *list);

static inline void dy_ast_list_release(struct dy_ast_list_inner list);

static inline void dy_ast_list_release_ptr(const struct dy_ast_list_inner *list);


const struct dy_ast_expr *dy_ast_expr_new(struct dy_ast_expr expr)
{
    return dy_rc_new(&expr, sizeof expr, dy_ast_expr_pre_padding, dy_ast_expr_post_padding);
}

const struct dy_ast_do_block_body *dy_ast_do_block_new(struct dy_ast_do_block_body do_block)
{
    return dy_rc_new(&do_block, sizeof do_block, dy_ast_do_block_pre_padding, dy_ast_do_block_post_padding);
}

const struct dy_ast_list_inner *dy_ast_list_new(struct dy_ast_list_inner list)
{
    return dy_rc_new(&list, sizeof list, dy_ast_list_pre_padding, dy_ast_list_post_padding);
}

const struct dy_ast_expr *dy_ast_expr_retain_ptr(const struct dy_ast_expr *expr)
{
    return dy_rc_retain(expr, dy_ast_expr_pre_padding, dy_ast_expr_post_padding);
}

struct dy_ast_expr dy_ast_expr_retain(struct dy_ast_expr expr)
{
    switch (expr.tag) {
    case DY_AST_EXPR_POSITIVE_TYPE_MAP:
        if (expr.positive_type_map.arg.has_type) {
            dy_ast_expr_retain_ptr(expr.positive_type_map.arg.type);
        }
        dy_ast_expr_retain_ptr(expr.positive_type_map.expr);
        return expr;
    case DY_AST_EXPR_NEGATIVE_TYPE_MAP:
        if (expr.negative_type_map.arg.has_type) {
            dy_ast_expr_retain_ptr(expr.negative_type_map.arg.type);
        }
        dy_ast_expr_retain_ptr(expr.negative_type_map.expr);
        return expr;
    case DY_AST_EXPR_LIST:
        dy_ast_list_retain(expr.list.inner);
        return expr;
    case DY_AST_EXPR_POSITIVE_EQUALITY_MAP:
        dy_ast_expr_retain_ptr(expr.positive_equality_map.e1);
        dy_ast_expr_retain_ptr(expr.positive_equality_map.e2);
        return expr;
    case DY_AST_EXPR_NEGATIVE_EQUALITY_MAP:
        dy_ast_expr_retain_ptr(expr.negative_equality_map.e1);
        dy_ast_expr_retain_ptr(expr.negative_equality_map.e2);
        return expr;
    case DY_AST_EXPR_DO_BLOCK:
        dy_ast_do_block_retain(expr.do_block.body);
        return expr;
    case DY_AST_EXPR_CHOICE:
        dy_ast_list_retain(expr.choice.inner);
        return expr;
    case DY_AST_EXPR_TRY_BLOCK:
        dy_ast_list_retain(expr.try_block.inner);
        return expr;
    case DY_AST_EXPR_EQUALITY_MAP_ELIM:
        dy_ast_expr_retain_ptr(expr.equality_map_elim.expr);
        dy_ast_expr_retain_ptr(expr.equality_map_elim.equality_map.e1);
        dy_ast_expr_retain_ptr(expr.equality_map_elim.equality_map.e2);
        return expr;
    case DY_AST_EXPR_TYPE_MAP_ELIM:
        dy_ast_expr_retain_ptr(expr.type_map_elim.expr);
        if (expr.type_map_elim.type_map.arg.has_type) {
            dy_ast_expr_retain_ptr(expr.type_map_elim.type_map.arg.type);
        }
        dy_ast_expr_retain_ptr(expr.type_map_elim.type_map.expr);
        return expr;
    case DY_AST_EXPR_VARIABLE:
        return expr;
    case DY_AST_EXPR_JUXTAPOSITION:
        dy_ast_expr_retain_ptr(expr.juxtaposition.left);
        dy_ast_expr_retain_ptr(expr.juxtaposition.right);
        return expr;
    case DY_AST_EXPR_POSITIVE_RECURSION:
        if (expr.positive_recursion.arg.has_type) {
            dy_ast_expr_retain_ptr(expr.positive_recursion.arg.type);
        }
        dy_ast_expr_retain_ptr(expr.positive_recursion.expr);
        return expr;
    case DY_AST_EXPR_NEGATIVE_RECURSION:
        if (expr.negative_recursion.arg.has_type) {
            dy_ast_expr_retain_ptr(expr.negative_recursion.arg.type);
        }
        dy_ast_expr_retain_ptr(expr.negative_recursion.expr);
        return expr;
    case DY_AST_EXPR_ALL:
        return expr;
    case DY_AST_EXPR_ANY:
        return expr;
    case DY_AST_EXPR_SYMBOL:
        return expr;
    }

    dy_bail("Impossible ast type.");
}

void dy_ast_expr_release_ptr(const struct dy_ast_expr *expr)
{
    struct dy_ast_expr e = *expr;
    if (dy_rc_release(expr, dy_ast_expr_pre_padding, dy_ast_expr_post_padding) == 0) {
        dy_ast_expr_release(e);
    }
}

void dy_ast_expr_release(struct dy_ast_expr expr)
{
    switch (expr.tag) {
    case DY_AST_EXPR_POSITIVE_TYPE_MAP:
        if (expr.positive_type_map.arg.has_type) {
            dy_ast_expr_release_ptr(expr.positive_type_map.arg.type);
        }
        dy_ast_expr_release_ptr(expr.positive_type_map.expr);
        return;
    case DY_AST_EXPR_NEGATIVE_TYPE_MAP:
        if (expr.negative_type_map.arg.has_type) {
            dy_ast_expr_release_ptr(expr.negative_type_map.arg.type);
        }
        dy_ast_expr_release_ptr(expr.negative_type_map.expr);
        return;
    case DY_AST_EXPR_LIST:
        dy_ast_list_release(expr.list.inner);
        return;
    case DY_AST_EXPR_POSITIVE_EQUALITY_MAP:
        dy_ast_expr_release_ptr(expr.positive_equality_map.e1);
        dy_ast_expr_release_ptr(expr.positive_equality_map.e2);
        return;
    case DY_AST_EXPR_NEGATIVE_EQUALITY_MAP:
        dy_ast_expr_release_ptr(expr.negative_equality_map.e1);
        dy_ast_expr_release_ptr(expr.negative_equality_map.e2);
        return;
    case DY_AST_EXPR_DO_BLOCK:
        dy_ast_do_block_release(expr.do_block.body);
        return;
    case DY_AST_EXPR_CHOICE:
        dy_ast_list_release(expr.choice.inner);
        return;
    case DY_AST_EXPR_TRY_BLOCK:
        dy_ast_list_release(expr.try_block.inner);
        return;
    case DY_AST_EXPR_EQUALITY_MAP_ELIM:
        dy_ast_expr_release_ptr(expr.equality_map_elim.expr);
        dy_ast_expr_release_ptr(expr.equality_map_elim.equality_map.e1);
        dy_ast_expr_release_ptr(expr.equality_map_elim.equality_map.e2);
        return;
    case DY_AST_EXPR_TYPE_MAP_ELIM:
        dy_ast_expr_release_ptr(expr.type_map_elim.expr);
        if (expr.type_map_elim.type_map.arg.has_type) {
            dy_ast_expr_release_ptr(expr.type_map_elim.type_map.arg.type);
        }
        dy_ast_expr_release_ptr(expr.type_map_elim.type_map.expr);
        return;
    case DY_AST_EXPR_VARIABLE:
        return;
    case DY_AST_EXPR_JUXTAPOSITION:
        dy_ast_expr_release_ptr(expr.juxtaposition.left);
        dy_ast_expr_release_ptr(expr.juxtaposition.right);
        return;
    case DY_AST_EXPR_POSITIVE_RECURSION:
        if (expr.positive_recursion.arg.has_type) {
            dy_ast_expr_release_ptr(expr.positive_recursion.arg.type);
        }
        dy_ast_expr_release_ptr(expr.positive_recursion.expr);
        return;
    case DY_AST_EXPR_NEGATIVE_RECURSION:
        if (expr.negative_recursion.arg.has_type) {
            dy_ast_expr_release_ptr(expr.negative_recursion.arg.type);
        }
        dy_ast_expr_release_ptr(expr.negative_recursion.expr);
        return;
    case DY_AST_EXPR_ALL:
        return;
    case DY_AST_EXPR_ANY:
        return;
    case DY_AST_EXPR_SYMBOL:
        return;
    }

    dy_bail("Impossible ast type.");
}

void dy_ast_do_block_release_ptr(const struct dy_ast_do_block_body *do_block)
{
    struct dy_ast_do_block_body d = *do_block;
    if (dy_rc_release(do_block, dy_ast_do_block_pre_padding, dy_ast_do_block_post_padding) == 0) {
        dy_ast_do_block_release(d);
    }
}

void dy_ast_do_block_retain_ptr(const struct dy_ast_do_block_body *do_block)
{
    dy_rc_retain(do_block, dy_ast_do_block_pre_padding, dy_ast_do_block_post_padding);
}

void dy_ast_do_block_retain(struct dy_ast_do_block_body do_block)
{
    switch (do_block.tag) {
    case DY_AST_DO_BLOCK_END_EXPR:
        dy_ast_expr_retain_ptr(do_block.end_expr);
        return;
    case DY_AST_DO_BLOCK_EQUALITY:
        dy_ast_expr_retain_ptr(do_block.equality.e1);
        dy_ast_expr_retain_ptr(do_block.equality.e2);
        dy_ast_do_block_retain_ptr(do_block.equality.rest);
        return;
    case DY_AST_DO_BLOCK_IGNORED_EXPR:
        dy_ast_expr_retain_ptr(do_block.ignored_expr.expr);
        dy_ast_do_block_retain_ptr(do_block.ignored_expr.rest);
        return;
    case DY_AST_DO_BLOCK_LET:
        dy_ast_expr_retain_ptr(do_block.let.expr);
        dy_ast_do_block_retain_ptr(do_block.let.rest);
        return;
    case DY_AST_DO_BLOCK_DEF:
        dy_ast_expr_retain_ptr(do_block.def.expr);
        dy_ast_do_block_retain_ptr(do_block.def.rest);
        return;
    }

    dy_bail("Impossible ast do block type.");
}

void dy_ast_do_block_release(struct dy_ast_do_block_body do_block)
{
    switch (do_block.tag) {
    case DY_AST_DO_BLOCK_END_EXPR:
        dy_ast_expr_release_ptr(do_block.end_expr);
        return;
    case DY_AST_DO_BLOCK_EQUALITY:
        dy_ast_expr_release_ptr(do_block.equality.e1);
        dy_ast_expr_release_ptr(do_block.equality.e2);
        dy_ast_do_block_release_ptr(do_block.equality.rest);
        return;
    case DY_AST_DO_BLOCK_IGNORED_EXPR:
        dy_ast_expr_release_ptr(do_block.ignored_expr.expr);
        dy_ast_do_block_release_ptr(do_block.ignored_expr.rest);
        return;
    case DY_AST_DO_BLOCK_LET:
        dy_ast_expr_release_ptr(do_block.let.expr);
        dy_ast_do_block_release_ptr(do_block.let.rest);
        return;
    case DY_AST_DO_BLOCK_DEF:
        dy_ast_expr_release_ptr(do_block.def.expr);
        dy_ast_do_block_release_ptr(do_block.def.rest);
        return;
    }

    dy_bail("Impossible ast do block type.");
}

void dy_ast_list_retain(struct dy_ast_list_inner list)
{
    dy_ast_expr_retain_ptr(list.expr);
    if (list.next_or_null) {
        dy_ast_list_retain_ptr(list.next_or_null);
    }
}

void dy_ast_list_retain_ptr(const struct dy_ast_list_inner *list)
{
    dy_rc_retain(list, dy_ast_list_pre_padding, dy_ast_list_post_padding);
}

void dy_ast_list_release(struct dy_ast_list_inner list)
{
    dy_ast_expr_release_ptr(list.expr);
    if (list.next_or_null) {
        dy_ast_list_release_ptr(list.next_or_null);
    }
}

void dy_ast_list_release_ptr(const struct dy_ast_list_inner *list)
{
    struct dy_ast_list_inner l = *list;
    if (dy_rc_release(list, dy_ast_list_pre_padding, dy_ast_list_post_padding) == 0) {
        dy_ast_list_release(l);
    }
}
