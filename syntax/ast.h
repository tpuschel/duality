/*
 * Copyright 2021 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "../support/array.h"
#include "../support/rc.h"
#include "../support/bail.h"

enum dy_ast_argument_tag {
    DY_AST_ARGUMENT_EXPR,
    DY_AST_ARGUMENT_INDEX,
    DY_AST_ARGUMENT_UNFOLD,
    DY_AST_ARGUMENT_UNWRAP
};

struct dy_ast_argument_index {
    size_t value;
    bool is_maximum;
};

struct dy_ast_argument {
    union {
        struct dy_ast_expr *expr;
        struct dy_ast_argument_index index;
    };

    enum dy_ast_argument_tag tag;
};

struct dy_ast_pattern_simple {
    struct dy_ast_argument arg;
    struct dy_ast_binding *binding;
    bool is_implicit;
};

struct dy_ast_pattern_list_body {
    struct dy_ast_binding *binding;
    struct dy_ast_pattern_list_body *next;
};

struct dy_ast_pattern_list {
    struct dy_ast_pattern_list_body body;
    bool is_implicit;
};

enum dy_ast_pattern_tag {
    DY_AST_PATTERN_SIMPLE,
    DY_AST_PATTERN_LIST
};

struct dy_ast_pattern {
    union {
        struct dy_ast_pattern_simple simple;
        struct dy_ast_pattern_list list;
    };

    enum dy_ast_pattern_tag tag;
};

enum dy_ast_binding_tag {
    DY_AST_BINDING_TYPE,
    DY_AST_BINDING_PATTERN,
    DY_AST_BINDING_NOTHING
};

struct dy_ast_binding {
    dy_array_t name;
    bool have_name;
    union {
        struct dy_ast_expr *type;
        struct dy_ast_pattern pattern;
    };

    enum dy_ast_binding_tag tag;
};

struct dy_ast_function {
    struct dy_ast_binding binding;
    struct dy_ast_expr *expr;
    bool is_implicit;
    bool is_some;
};

struct dy_ast_recursion {
    dy_array_t name;
    struct dy_ast_expr *expr;
    bool is_fin;
    bool is_implicit;
};

struct dy_ast_list_body {
    struct dy_ast_expr *expr;
    struct dy_ast_list_body *next; // Can be NULL.
};

struct dy_ast_list {
    struct dy_ast_list_body body;
    bool is_either;
    bool is_implicit;
};

struct dy_ast_do_block_stmnt_expr {
    struct dy_ast_expr *expr;
    bool is_inverted;
};

struct dy_ast_do_block_stmnt_let {
    struct dy_ast_binding binding;
    struct dy_ast_expr *expr;
    bool is_inverted;
};

struct dy_ast_do_block_stmnt_def {
    dy_array_t name;
    struct dy_ast_expr *expr;
};

enum dy_ast_do_block_stmnt_tag {
    DY_AST_DO_BLOCK_STMNT_EXPR,
    DY_AST_DO_BLOCK_STMNT_LET,
    DY_AST_DO_BLOCK_STMNT_DEF,
};

struct dy_ast_do_block_stmnt {
    union {
        struct dy_ast_do_block_stmnt_expr expr;
        struct dy_ast_do_block_stmnt_let let;
        struct dy_ast_do_block_stmnt_def def;
    };

    enum dy_ast_do_block_stmnt_tag tag;
};

struct dy_ast_do_block {
    struct dy_ast_do_block_stmnt stmnt;
    struct dy_ast_do_block *rest; // Can be NULL. Then stmnt.tag == DY_AST_DO_BLOCK_STMNT_EXPR
};

struct dy_ast_juxtaposition {
    struct dy_ast_expr *left;
    struct dy_ast_argument right;
    struct dy_ast_expr *type; // Can be NULL.
    bool is_implicit;
};

struct dy_ast_simple {
    struct dy_ast_argument argument;
    struct dy_ast_expr *expr;
    bool is_implicit;
    bool is_negative;
};

struct dy_ast_map_some {
    dy_array_t name;
    struct dy_ast_expr *type;
    struct dy_ast_binding binding;
    struct dy_ast_expr *expr;
    bool is_implicit;
};

struct dy_ast_map_either_body {
    struct dy_ast_binding binding;
    struct dy_ast_expr *expr;
    struct dy_ast_map_either_body *next; // Can be NULL.
};

struct dy_ast_map_either {
    struct dy_ast_map_either_body body;
    bool is_implicit;
};

struct dy_ast_map_fin {
    dy_array_t name;
    struct dy_ast_binding binding;
    struct dy_ast_expr *expr;
    bool is_implicit;
};

enum dy_ast_expr_tag {
    DY_AST_EXPR_VARIABLE,
    DY_AST_EXPR_FUNCTION,
    DY_AST_EXPR_RECURSION,
    DY_AST_EXPR_LIST,
    DY_AST_EXPR_DO_BLOCK,
    DY_AST_EXPR_STRING,
    DY_AST_EXPR_STRING_TYPE,
    DY_AST_EXPR_ANY,
    DY_AST_EXPR_VOID,
    DY_AST_EXPR_JUXTAPOSITION,
    DY_AST_EXPR_SIMPLE,
    DY_AST_EXPR_MAP_SOME,
    DY_AST_EXPR_MAP_EITHER,
    DY_AST_EXPR_MAP_FIN
};

struct dy_ast_expr {
    union {
        dy_array_t variable;
        struct dy_ast_function function;
        struct dy_ast_recursion recursion;
        struct dy_ast_list list;
        struct dy_ast_do_block do_block;
        dy_array_t string;
        struct dy_ast_juxtaposition juxtaposition;
        struct dy_ast_simple simple;
        struct dy_ast_map_some map_some;
        struct dy_ast_map_either map_either;
        struct dy_ast_map_fin map_fin;
    };

    enum dy_ast_expr_tag tag;
};

static inline struct dy_ast_expr *dy_ast_expr_new(struct dy_ast_expr expr);

static inline struct dy_ast_expr dy_ast_expr_retain(struct dy_ast_expr expr);
static inline struct dy_ast_expr *dy_ast_expr_retain_ptr(struct dy_ast_expr *expr);

static inline void dy_ast_expr_release(struct dy_ast_expr expr);
static inline void dy_ast_expr_release_ptr(struct dy_ast_expr *expr);


static inline struct dy_ast_pattern *dy_ast_pattern_new(struct dy_ast_pattern pattern);

static inline struct dy_ast_pattern dy_ast_pattern_retain(struct dy_ast_pattern pattern);
static inline struct dy_ast_pattern *dy_ast_pattern_retain_ptr(struct dy_ast_pattern *pattern);

static inline void dy_ast_pattern_release(struct dy_ast_pattern pattern);
static inline void dy_ast_pattern_release_ptr(struct dy_ast_pattern *pattern);


static inline struct dy_ast_do_block *dy_ast_do_block_new(struct dy_ast_do_block do_block);

static inline struct dy_ast_do_block dy_ast_do_block_retain(struct dy_ast_do_block do_block);
static inline struct dy_ast_do_block *dy_ast_do_block_retain_ptr(struct dy_ast_do_block *do_block);

static inline void dy_ast_do_block_release(struct dy_ast_do_block do_block);
static inline void dy_ast_do_block_release_ptr(struct dy_ast_do_block *do_block);


static inline struct dy_ast_do_block_stmnt dy_ast_do_block_stmnt_retain(struct dy_ast_do_block_stmnt stmnt);

static inline void dy_ast_do_block_stmnt_release(struct dy_ast_do_block_stmnt stmnt);


static inline struct dy_ast_argument dy_ast_argument_retain(struct dy_ast_argument arg);

static inline void dy_ast_argument_release(struct dy_ast_argument arg);


static inline struct dy_ast_list_body *dy_ast_list_body_new(struct dy_ast_list_body list_body);

static inline struct dy_ast_list_body dy_ast_list_body_retain(struct dy_ast_list_body list_body);
static inline struct dy_ast_list_body *dy_ast_list_body_retain_ptr(struct dy_ast_list_body *list_body);

static inline void dy_ast_list_body_release(struct dy_ast_list_body list_body);
static inline void dy_ast_list_body_release_ptr(struct dy_ast_list_body *list_body);


static inline struct dy_ast_pattern_list_body *dy_ast_pattern_list_body_new(struct dy_ast_pattern_list_body list_body);

static inline struct dy_ast_pattern_list_body dy_ast_pattern_list_body_retain(struct dy_ast_pattern_list_body list_body);
static inline struct dy_ast_pattern_list_body *dy_ast_pattern_list_body_retain_ptr(struct dy_ast_pattern_list_body *list_body);

static inline void dy_ast_pattern_list_body_release(struct dy_ast_pattern_list_body list_body);
static inline void dy_ast_pattern_list_body_release_ptr(struct dy_ast_pattern_list_body *list_body);


static inline struct dy_ast_binding *dy_ast_binding_new(struct dy_ast_binding binding);

static inline struct dy_ast_binding dy_ast_binding_retain(struct dy_ast_binding binding);
static inline struct dy_ast_binding *dy_ast_binding_retain_ptr(struct dy_ast_binding *binding);

static inline void dy_ast_binding_release(struct dy_ast_binding binding);
static inline void dy_ast_binding_release_ptr(struct dy_ast_binding *binding);


static inline struct dy_ast_map_either_body *dy_ast_map_either_body_new(struct dy_ast_map_either_body map_either_body);

static inline struct dy_ast_map_either_body dy_ast_map_either_body_retain(struct dy_ast_map_either_body map_either_body);
static inline struct dy_ast_map_either_body *dy_ast_map_either_body_retain_ptr(struct dy_ast_map_either_body *map_either_body);

static inline void dy_ast_map_either_body_release(struct dy_ast_map_either_body map_either_body);
static inline void dy_ast_map_either_body_release_ptr(struct dy_ast_map_either_body *map_either_body);


struct dy_ast_expr *dy_ast_expr_new(struct dy_ast_expr expr)
{
    return dy_rc_new(&expr, sizeof(expr), DY_ALIGNOF(struct dy_ast_expr));
}

struct dy_ast_expr dy_ast_expr_retain(struct dy_ast_expr expr)
{
    switch (expr.tag) {
    case DY_AST_EXPR_VARIABLE:
        dy_array_retain(&expr.variable);
        return expr;
    case DY_AST_EXPR_FUNCTION:
        dy_ast_binding_retain(expr.function.binding);
        dy_ast_expr_retain_ptr(expr.function.expr);
        return expr;
    case DY_AST_EXPR_RECURSION:
        dy_array_retain(&expr.recursion.name);
        dy_ast_expr_retain_ptr(expr.recursion.expr);
        return expr;
    case DY_AST_EXPR_LIST:
        dy_ast_list_body_retain(expr.list.body);
        return expr;
    case DY_AST_EXPR_DO_BLOCK:
        dy_ast_do_block_retain(expr.do_block);
        return expr;
    case DY_AST_EXPR_STRING:
        dy_array_retain(&expr.string);
        return expr;
    case DY_AST_EXPR_ANY:
    case DY_AST_EXPR_VOID:
    case DY_AST_EXPR_STRING_TYPE:
        return expr;
    case DY_AST_EXPR_JUXTAPOSITION:
        dy_ast_expr_retain_ptr(expr.juxtaposition.left);
        dy_ast_argument_retain(expr.juxtaposition.right);
        if (expr.juxtaposition.type) {
            dy_ast_expr_retain_ptr(expr.juxtaposition.type);
        }
        return expr;
    case DY_AST_EXPR_SIMPLE:
        dy_ast_argument_retain(expr.simple.argument);
        dy_ast_expr_retain_ptr(expr.simple.expr);
        return expr;
    case DY_AST_EXPR_MAP_SOME:
        dy_array_retain(&expr.map_some.name);
        dy_ast_expr_retain_ptr(expr.map_some.type);
        dy_ast_binding_retain(expr.map_some.binding);
        dy_ast_expr_retain_ptr(expr.map_some.expr);
        return expr;
    case DY_AST_EXPR_MAP_EITHER:
        dy_ast_map_either_body_retain(expr.map_either.body);
        return expr;
    case DY_AST_EXPR_MAP_FIN:
        dy_array_retain(&expr.map_fin.name);
        dy_ast_binding_retain(expr.map_fin.binding);
        dy_ast_expr_retain_ptr(expr.map_fin.expr);
        return expr;
    }

    dy_bail("impossible");
}

struct dy_ast_expr *dy_ast_expr_retain_ptr(struct dy_ast_expr *expr)
{
    return dy_rc_retain(expr, DY_ALIGNOF(struct dy_ast_expr));
}

void dy_ast_expr_release(struct dy_ast_expr expr)
{
    switch (expr.tag) {
    case DY_AST_EXPR_VARIABLE:
        dy_array_release(&expr.variable);
        return;
    case DY_AST_EXPR_FUNCTION:
        dy_ast_binding_release(expr.function.binding);
        dy_ast_expr_release_ptr(expr.function.expr);
        return;
    case DY_AST_EXPR_RECURSION:
        dy_array_release(&expr.recursion.name);
        dy_ast_expr_release_ptr(expr.recursion.expr);
        return;
    case DY_AST_EXPR_LIST:
        dy_ast_list_body_release(expr.list.body);
        return;
    case DY_AST_EXPR_DO_BLOCK:
        dy_ast_do_block_release(expr.do_block);
        return;
    case DY_AST_EXPR_STRING:
        dy_array_release(&expr.string);
        return;
    case DY_AST_EXPR_ANY:
    case DY_AST_EXPR_VOID:
    case DY_AST_EXPR_STRING_TYPE:
        return;
    case DY_AST_EXPR_JUXTAPOSITION:
        dy_ast_expr_release_ptr(expr.juxtaposition.left);
        dy_ast_argument_release(expr.juxtaposition.right);
        if (expr.juxtaposition.type) {
            dy_ast_expr_release_ptr(expr.juxtaposition.type);
        }
        return;
    case DY_AST_EXPR_SIMPLE:
        dy_ast_argument_release(expr.simple.argument);
        dy_ast_expr_release_ptr(expr.simple.expr);
        return;
    case DY_AST_EXPR_MAP_SOME:
        dy_array_release(&expr.map_some.name);
        dy_ast_expr_release_ptr(expr.map_some.type);
        dy_ast_binding_release(expr.map_some.binding);
        dy_ast_expr_release_ptr(expr.map_some.expr);
        return;
    case DY_AST_EXPR_MAP_EITHER:
        dy_ast_map_either_body_release(expr.map_either.body);
        return;
    case DY_AST_EXPR_MAP_FIN:
        dy_array_release(&expr.map_fin.name);
        dy_ast_binding_release(expr.map_fin.binding);
        dy_ast_expr_release_ptr(expr.map_fin.expr);
        return;
    }

    dy_bail("impossible");
}

void dy_ast_expr_release_ptr(struct dy_ast_expr *expr)
{
    struct dy_ast_expr e = *expr;
    if (dy_rc_release(expr, DY_ALIGNOF(struct dy_ast_expr)) == 0) {
        dy_ast_expr_release(e);
    }
}


struct dy_ast_pattern *dy_ast_pattern_new(struct dy_ast_pattern pattern)
{
    return dy_rc_new(&pattern, sizeof(pattern), DY_ALIGNOF(struct dy_ast_pattern));
}

struct dy_ast_pattern dy_ast_pattern_retain(struct dy_ast_pattern pattern)
{
    switch (pattern.tag) {
    case DY_AST_PATTERN_SIMPLE:
        dy_ast_argument_retain(pattern.simple.arg);
        dy_ast_binding_retain_ptr(pattern.simple.binding);
        return pattern;
    case DY_AST_PATTERN_LIST:
        dy_ast_pattern_list_body_retain(pattern.list.body);
        return pattern;
    }

    dy_bail("impossible");
}

struct dy_ast_pattern *dy_ast_pattern_retain_ptr(struct dy_ast_pattern *pattern)
{
    return dy_rc_retain(pattern, DY_ALIGNOF(struct dy_ast_pattern));
}

void dy_ast_pattern_release(struct dy_ast_pattern pattern)
{
    switch (pattern.tag) {
    case DY_AST_PATTERN_SIMPLE:
        dy_ast_argument_release(pattern.simple.arg);
        dy_ast_binding_release_ptr(pattern.simple.binding);
        return;
    case DY_AST_PATTERN_LIST:
        dy_ast_pattern_list_body_release(pattern.list.body);
        return;
    }

    dy_bail("impossible");
}

void dy_ast_pattern_release_ptr(struct dy_ast_pattern *pattern)
{
    struct dy_ast_pattern p = *pattern;
    if (dy_rc_release(pattern, DY_ALIGNOF(struct dy_ast_pattern)) == 0) {
        dy_ast_pattern_release(p);
    }
}


struct dy_ast_do_block *dy_ast_do_block_new(struct dy_ast_do_block do_block)
{
    return dy_rc_new(&do_block, sizeof(do_block), DY_ALIGNOF(struct dy_ast_do_block));
}

struct dy_ast_do_block dy_ast_do_block_retain(struct dy_ast_do_block do_block)
{
    dy_ast_do_block_stmnt_retain(do_block.stmnt);

    if (do_block.rest) {
        dy_ast_do_block_retain_ptr(do_block.rest);
    }

    return do_block;
}

struct dy_ast_do_block *dy_ast_do_block_retain_ptr(struct dy_ast_do_block *do_block)
{
    return dy_rc_retain(do_block, DY_ALIGNOF(struct dy_ast_do_block));
}

void dy_ast_do_block_release(struct dy_ast_do_block do_block)
{
    dy_ast_do_block_stmnt_release(do_block.stmnt);

    if (do_block.rest) {
        dy_ast_do_block_release_ptr(do_block.rest);
    }
}

void dy_ast_do_block_release_ptr(struct dy_ast_do_block *do_block)
{
    struct dy_ast_do_block d = *do_block;
    if (dy_rc_release(do_block, DY_ALIGNOF(struct dy_ast_do_block)) == 0) {
        dy_ast_do_block_release(d);
    }
}

struct dy_ast_do_block_stmnt dy_ast_do_block_stmnt_retain(struct dy_ast_do_block_stmnt stmnt)
{
    switch (stmnt.tag){
    case DY_AST_DO_BLOCK_STMNT_EXPR:
        dy_ast_expr_retain_ptr(stmnt.expr.expr);
        return stmnt;
    case DY_AST_DO_BLOCK_STMNT_LET:
        dy_ast_binding_retain(stmnt.let.binding);
        dy_ast_expr_retain_ptr(stmnt.let.expr);
        return stmnt;
    case DY_AST_DO_BLOCK_STMNT_DEF:
        dy_array_retain(&stmnt.def.name);
        dy_ast_expr_retain_ptr(stmnt.def.expr);
        return stmnt;
    }

    dy_bail("impossible");
}


void dy_ast_do_block_stmnt_release(struct dy_ast_do_block_stmnt stmnt)
{
    switch (stmnt.tag){
    case DY_AST_DO_BLOCK_STMNT_EXPR:
        dy_ast_expr_release_ptr(stmnt.expr.expr);
        return;
    case DY_AST_DO_BLOCK_STMNT_LET:
        dy_ast_binding_release(stmnt.let.binding);
        dy_ast_expr_release_ptr(stmnt.let.expr);
        return;
    case DY_AST_DO_BLOCK_STMNT_DEF:
        dy_array_release(&stmnt.def.name);
        dy_ast_expr_release_ptr(stmnt.def.expr);
        return;
    }

    dy_bail("impossible");
}


struct dy_ast_argument dy_ast_argument_retain(struct dy_ast_argument arg)
{
    if (arg.tag == DY_AST_ARGUMENT_EXPR) {
        dy_ast_expr_retain_ptr(arg.expr);
    }

    return arg;
}

void dy_ast_argument_release(struct dy_ast_argument arg)
{
    if (arg.tag == DY_AST_ARGUMENT_EXPR) {
        dy_ast_expr_release_ptr(arg.expr);
    }
}


struct dy_ast_list_body *dy_ast_list_body_new(struct dy_ast_list_body list_body)
{
    return dy_rc_new(&list_body, sizeof(list_body), DY_ALIGNOF(struct dy_ast_list_body));
}

struct dy_ast_list_body dy_ast_list_body_retain(struct dy_ast_list_body list_body)
{
    dy_ast_expr_retain_ptr(list_body.expr);
    if (list_body.next) {
        dy_ast_list_body_retain_ptr(list_body.next);
    }
    return list_body;
}

struct dy_ast_list_body *dy_ast_list_body_retain_ptr(struct dy_ast_list_body *list_body)
{
    return dy_rc_retain(list_body, DY_ALIGNOF(struct dy_ast_list_body));
}

void dy_ast_list_body_release(struct dy_ast_list_body list_body)
{
    dy_ast_expr_release_ptr(list_body.expr);
    if (list_body.next) {
        dy_ast_list_body_release_ptr(list_body.next);
    }
}

void dy_ast_list_body_release_ptr(struct dy_ast_list_body *list_body)
{
    struct dy_ast_list_body l = *list_body;
    if (dy_rc_release(list_body, DY_ALIGNOF(struct dy_ast_list_body)) == 0) {
        dy_ast_list_body_release(l);
    }
}

struct dy_ast_pattern_list_body *dy_ast_pattern_list_body_new(struct dy_ast_pattern_list_body list_body)
{
    return dy_rc_new(&list_body, sizeof(list_body), DY_ALIGNOF(struct dy_ast_pattern_list_body));
}

struct dy_ast_pattern_list_body dy_ast_pattern_list_body_retain(struct dy_ast_pattern_list_body list_body)
{
    dy_ast_binding_retain_ptr(list_body.binding);
    if (list_body.next) {
        dy_ast_pattern_list_body_retain_ptr(list_body.next);
    }

    return list_body;
}

struct dy_ast_pattern_list_body *dy_ast_pattern_list_body_retain_ptr(struct dy_ast_pattern_list_body *list_body)
{
    return dy_rc_retain(list_body, DY_ALIGNOF(struct dy_ast_pattern_list_body));
}

void dy_ast_pattern_list_body_release(struct dy_ast_pattern_list_body list_body)
{
    dy_ast_binding_release_ptr(list_body.binding);
    if (list_body.next) {
        dy_ast_pattern_list_body_release_ptr(list_body.next);
    }
}

void dy_ast_pattern_list_body_release_ptr(struct dy_ast_pattern_list_body *list_body)
{
    struct dy_ast_pattern_list_body l = *list_body;
    if (dy_rc_release(list_body, DY_ALIGNOF(struct dy_ast_pattern_list_body)) == 0) {
        dy_ast_pattern_list_body_release(l);
    }
}

struct dy_ast_binding *dy_ast_binding_new(struct dy_ast_binding binding)
{
    return dy_rc_new(&binding, sizeof(binding), DY_ALIGNOF(struct dy_ast_binding));
}

struct dy_ast_binding dy_ast_binding_retain(struct dy_ast_binding binding)
{
    if (binding.have_name) {
        dy_array_retain(&binding.name);
    }

    switch (binding.tag) {
    case DY_AST_BINDING_TYPE:
        dy_ast_expr_retain_ptr(binding.type);
        return binding;
    case DY_AST_BINDING_PATTERN:
        dy_ast_pattern_retain(binding.pattern);
        return binding;
    case DY_AST_BINDING_NOTHING:
        return binding;
    }

    dy_bail("impossible");
}

struct dy_ast_binding *dy_ast_binding_retain_ptr(struct dy_ast_binding *binding)
{
    return dy_rc_retain(binding, DY_ALIGNOF(struct dy_ast_binding));
}

void dy_ast_binding_release(struct dy_ast_binding binding)
{
    if (binding.have_name) {
        dy_array_release(&binding.name);
    }

    switch (binding.tag) {
    case DY_AST_BINDING_TYPE:
        dy_ast_expr_release_ptr(binding.type);
        return;
    case DY_AST_BINDING_PATTERN:
        dy_ast_pattern_release(binding.pattern);
        return;
    case DY_AST_BINDING_NOTHING:
        return;
    }

    dy_bail("impossible");
}

void dy_ast_binding_release_ptr(struct dy_ast_binding *binding)
{
    struct dy_ast_binding b = *binding;
    if (dy_rc_release(binding, DY_ALIGNOF(struct dy_ast_binding)) == 0) {
        dy_ast_binding_release(b);
    }
}

struct dy_ast_map_either_body *dy_ast_map_either_body_new(struct dy_ast_map_either_body map_either_body)
{
    return dy_rc_new(&map_either_body, sizeof(map_either_body), DY_ALIGNOF(struct dy_ast_map_either_body));
}

struct dy_ast_map_either_body dy_ast_map_either_body_retain(struct dy_ast_map_either_body map_either_body)
{
    dy_ast_binding_retain(map_either_body.binding);
    dy_ast_expr_retain_ptr(map_either_body.expr);
    if (map_either_body.next) {
        dy_ast_map_either_body_retain_ptr(map_either_body.next);
    }

    return map_either_body;
}

struct dy_ast_map_either_body *dy_ast_map_either_body_retain_ptr(struct dy_ast_map_either_body *map_either_body)
{
    return dy_rc_retain(map_either_body, DY_ALIGNOF(struct dy_ast_map_either_body));
}

void dy_ast_map_either_body_release(struct dy_ast_map_either_body map_either_body)
{
    dy_ast_binding_release(map_either_body.binding);
    dy_ast_expr_release_ptr(map_either_body.expr);
    if (map_either_body.next) {
        dy_ast_map_either_body_release_ptr(map_either_body.next);
    }
}

void dy_ast_map_either_body_release_ptr(struct dy_ast_map_either_body *map_either_body)
{
    struct dy_ast_map_either_body b = *map_either_body;
    if (dy_rc_release(map_either_body, DY_ALIGNOF(struct dy_ast_map_either_body)) == 0) {
        dy_ast_map_either_body_release(b);
    }
}
