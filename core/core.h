/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "../support/string.h"
#include "../support/array.h"
#include "../support/rc.h"
#include "../support/range.h"
#include "../support/bail.h"

/**
 * This file implements the data structure that represents Core,
 * and any associated auxiliary functions.
 */

/**
 * Context passed to functions who need to generate new type maps
 * or need access to currently bound constraints.
 */
struct dy_core_ctx {
    size_t running_id;

    dy_array_t bound_constraints;

    dy_array_t already_visited_ids;

    dy_array_t subtype_assumption_cache;

    dy_array_t supertype_assumption_cache;
};

/** Hopefully self-explanatory :) */
typedef enum dy_ternary {
    DY_YES,
    DY_NO,
    DY_MAYBE
} dy_ternary_t;

struct dy_core_expr;
struct dy_constraint;

/**
 * A lot of pairs of objects in Core are essentially the same,
 * differing only in 1 bit of information. This difference is
 * called 'polarity'.
 */
enum dy_core_polarity {
    DY_CORE_POLARITY_POSITIVE,
    DY_CORE_POLARITY_NEGATIVE
};

struct dy_core_variable {
    size_t id;
    const struct dy_core_expr *type;
};

struct dy_core_equality_map {
    const struct dy_core_expr *e1;
    const struct dy_core_expr *e2;
    enum dy_core_polarity polarity;
    bool is_implicit;
};

struct dy_core_type_map {
    struct dy_core_variable binding;
    const struct dy_core_expr *expr;
    enum dy_core_polarity polarity;
    bool is_implicit;
};

struct dy_core_equality_map_elim {
    struct dy_range text_range;
    bool has_text_range;

    const struct dy_core_expr *expr;
    struct dy_core_equality_map map;
    dy_ternary_t check_result;
};

struct dy_core_type_map_elim {
    struct dy_range text_range;
    bool has_text_range;

    const struct dy_core_expr *expr;
    struct dy_core_type_map map;
    dy_ternary_t check_result;
};

struct dy_core_junction {
    const struct dy_core_expr *e1;
    const struct dy_core_expr *e2;
    enum dy_core_polarity polarity;
};

struct dy_core_alternative {
    const struct dy_core_expr *first;
    const struct dy_core_expr *second;
};

struct dy_core_recursion {
    size_t id;
    const struct dy_core_expr *expr;
    enum dy_core_polarity polarity;
};

struct dy_core_custom {
    size_t id;
    void *data;
    bool can_be_eliminated;

    struct dy_core_expr (*type_of)(void *data, struct dy_core_ctx *ctx);

    dy_ternary_t (*is_equal)(void *data, struct dy_core_expr expr);

    struct dy_core_expr (*check)(void *data, struct dy_core_ctx *ctx, struct dy_constraint *constraint, bool *did_generate_constraint);

    struct dy_core_expr (*remove_mentions_in_subtype)(void *data, struct dy_core_ctx *ctx, size_t id);

    struct dy_core_expr (*remove_mentions_in_supertype)(void *data, struct dy_core_ctx *ctx, size_t id);

    struct dy_core_expr (*eval)(void *data, struct dy_core_ctx *ctx, bool *is_value);

    struct dy_core_expr (*substitute)(void *data, size_t id, struct dy_core_expr sub);

    dy_ternary_t (*is_subtype)(void *data, struct dy_core_ctx *ctx, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr);

    dy_ternary_t (*is_supertype)(void *data, struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr);

    struct dy_core_expr (*eliminate)(void *data, struct dy_core_ctx *ctx, struct dy_core_expr expr, bool *is_value);

    bool (*is_computation)(void *data);

    size_t (*num_occurrences)(void *data, size_t id);

    void *(*retain)(void *data);

    void (*release)(void *data);

    void (*to_string)(void *data, dy_array_t *string);
};

enum dy_core_expr_tag {
    DY_CORE_EXPR_EQUALITY_MAP,
    DY_CORE_EXPR_TYPE_MAP,
    DY_CORE_EXPR_EQUALITY_MAP_ELIM,
    DY_CORE_EXPR_TYPE_MAP_ELIM,
    DY_CORE_EXPR_JUNCTION,
    DY_CORE_EXPR_ALTERNATIVE,
    DY_CORE_EXPR_VARIABLE,
    DY_CORE_EXPR_END,
    DY_CORE_EXPR_RECURSION,
    DY_CORE_EXPR_INFERENCE_VARIABLE,
    DY_CORE_EXPR_INFERENCE_TYPE_MAP,
    DY_CORE_EXPR_SYMBOL,
    DY_CORE_EXPR_CUSTOM
};

struct dy_core_expr {
    union {
        struct dy_core_equality_map equality_map;
        struct dy_core_type_map type_map;
        struct dy_core_equality_map_elim equality_map_elim;
        struct dy_core_type_map_elim type_map_elim;
        struct dy_core_junction junction;
        struct dy_core_alternative alternative;
        struct dy_core_variable variable;
        struct dy_core_variable inference_variable;
        struct dy_core_type_map inference_type_map;
        struct dy_core_recursion recursion;
        enum dy_core_polarity end_polarity;
        struct dy_core_custom custom;
    };

    enum dy_core_expr_tag tag;
};

struct dy_bound_constraint {
    size_t id;
    struct dy_core_expr type;
    dy_array_t binding_ids;
};

struct dy_subtype_assumption {
    struct dy_core_expr type;
    size_t rec_binding_id;
};

/**
 * Determines if an expression is syntactically a value or a computation.
 */
static inline bool dy_core_expr_is_computation(struct dy_core_expr expr);

/** How often 'id' is mentioned in 'expr'. */
static inline size_t dy_core_expr_num_occurrences(size_t id, struct dy_core_expr expr);

/** Returns whether 'id' occurs in expr at all. */
static inline bool dy_core_expr_is_bound(size_t id, struct dy_core_expr expr);

/** Appends a utf8 represention of expr to 'string'. */
static inline void dy_core_expr_to_string(struct dy_core_expr expr, dy_array_t *string);


/** Boring ref-count stuff. */
static const size_t dy_core_expr_pre_padding = DY_RC_PRE_PADDING(struct dy_core_expr);
static const size_t dy_core_expr_post_padding = DY_RC_POST_PADDING(struct dy_core_expr);

static inline const struct dy_core_expr *dy_core_expr_new(struct dy_core_expr expr);

static inline struct dy_core_expr dy_core_expr_retain(struct dy_core_expr expr);

static inline const struct dy_core_expr *dy_core_expr_retain_ptr(const struct dy_core_expr *expr);

static inline void dy_core_expr_release(struct dy_core_expr expr);

static inline void dy_core_expr_release_ptr(const struct dy_core_expr *expr);

/** Helper to add a string literal to a dynamic char array. */
static inline void add_string(dy_array_t *string, dy_string_t s);

static inline void add_size_t_decimal(dy_array_t *string, size_t x);

bool dy_core_expr_is_computation(struct dy_core_expr expr)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_EQUALITY_MAP:
        return dy_core_expr_is_computation(*expr.equality_map.e1);
    case DY_CORE_EXPR_TYPE_MAP:
        return dy_core_expr_is_computation(*expr.type_map.binding.type);
    case DY_CORE_EXPR_EQUALITY_MAP_ELIM:
        return true;
    case DY_CORE_EXPR_TYPE_MAP_ELIM:
        return true;
    case DY_CORE_EXPR_JUNCTION:
        return dy_core_expr_is_computation(*expr.junction.e1) || dy_core_expr_is_computation(*expr.junction.e2);
    case DY_CORE_EXPR_ALTERNATIVE:
        return dy_core_expr_is_computation(*expr.alternative.first) || dy_core_expr_is_computation(*expr.alternative.second);
    case DY_CORE_EXPR_VARIABLE:
        return false;
    case DY_CORE_EXPR_INFERENCE_VARIABLE:
        return false;
    case DY_CORE_EXPR_INFERENCE_TYPE_MAP:
        dy_bail("should never be reached");
    case DY_CORE_EXPR_RECURSION:
        return dy_core_expr_is_computation(*expr.recursion.expr);
    case DY_CORE_EXPR_END:
        // fallthrough
    case DY_CORE_EXPR_SYMBOL:
        return false;
    case DY_CORE_EXPR_CUSTOM:
        return expr.custom.is_computation(expr.custom.data);
    }

    dy_bail("Impossible object type.");
}

bool dy_core_expr_is_bound(size_t id, struct dy_core_expr expr)
{
    // TODO: Optimize by not using num_occurrences (can stop traversing after finding one instance).
    return dy_core_expr_num_occurrences(id, expr) > 0;
}

size_t dy_core_expr_num_occurrences(size_t id, struct dy_core_expr expr)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_EQUALITY_MAP:
        return dy_core_expr_num_occurrences(id, *expr.equality_map.e1) + dy_core_expr_num_occurrences(id, *expr.equality_map.e2);
    case DY_CORE_EXPR_TYPE_MAP: {
        size_t x = dy_core_expr_num_occurrences(id, *expr.type_map.binding.type);

        if (id == expr.type_map.binding.id) {
            return x;
        }

        return x + dy_core_expr_num_occurrences(id, *expr.type_map.expr);
    }
    case DY_CORE_EXPR_EQUALITY_MAP_ELIM:
        return dy_core_expr_num_occurrences(id, *expr.equality_map_elim.expr) + dy_core_expr_num_occurrences(id, *expr.equality_map_elim.map.e1) + dy_core_expr_num_occurrences(id, *expr.equality_map_elim.map.e2);
    case DY_CORE_EXPR_TYPE_MAP_ELIM: {
        size_t x = dy_core_expr_num_occurrences(id, *expr.type_map_elim.expr) + dy_core_expr_num_occurrences(id, *expr.type_map_elim.map.binding.type);

        if (id == expr.type_map_elim.map.binding.id) {
            return x;
        }

        return x + dy_core_expr_num_occurrences(id, *expr.type_map_elim.map.expr);
    }
    case DY_CORE_EXPR_JUNCTION:
        return dy_core_expr_num_occurrences(id, *expr.junction.e1) + dy_core_expr_num_occurrences(id, *expr.junction.e2);
    case DY_CORE_EXPR_ALTERNATIVE:
        return dy_core_expr_num_occurrences(id, *expr.alternative.first) + dy_core_expr_num_occurrences(id, *expr.alternative.second);
    case DY_CORE_EXPR_VARIABLE:
        if (expr.variable.id == id) {
            return 1;
        } else {
            return 0;
        }
    case DY_CORE_EXPR_INFERENCE_VARIABLE:
        if (expr.inference_variable.id == id) {
            return 1;
        } else {
            return 0;
        }
    case DY_CORE_EXPR_INFERENCE_TYPE_MAP:
        dy_bail("should never be reached");
    case DY_CORE_EXPR_RECURSION: {
        if (id == expr.recursion.id) {
            return 0;
        }

        return dy_core_expr_num_occurrences(id, *expr.recursion.expr);
    }
    case DY_CORE_EXPR_END:
        // fallthrough
    case DY_CORE_EXPR_SYMBOL:
        return 0;
    case DY_CORE_EXPR_CUSTOM:
        return expr.custom.num_occurrences(expr.custom.data, id);
    }

    dy_bail("Impossible object type.");
}

const struct dy_core_expr *dy_core_expr_new(struct dy_core_expr expr)
{
    return dy_rc_new(&expr, sizeof expr, dy_core_expr_pre_padding, dy_core_expr_post_padding);
}

const struct dy_core_expr *dy_core_expr_retain_ptr(const struct dy_core_expr *expr)
{
    return dy_rc_retain(expr, dy_core_expr_pre_padding, dy_core_expr_post_padding);
}

struct dy_core_expr dy_core_expr_retain(struct dy_core_expr expr)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_EQUALITY_MAP:
        dy_core_expr_retain_ptr(expr.equality_map.e1);
        dy_core_expr_retain_ptr(expr.equality_map.e2);

        return expr;
    case DY_CORE_EXPR_TYPE_MAP:
        dy_core_expr_retain_ptr(expr.type_map.binding.type);
        dy_core_expr_retain_ptr(expr.type_map.expr);

        return expr;
    case DY_CORE_EXPR_EQUALITY_MAP_ELIM:
        dy_core_expr_retain_ptr(expr.equality_map_elim.expr);
        dy_core_expr_retain_ptr(expr.equality_map_elim.map.e1);
        dy_core_expr_retain_ptr(expr.equality_map_elim.map.e2);

        return expr;
    case DY_CORE_EXPR_TYPE_MAP_ELIM:
        dy_core_expr_retain_ptr(expr.type_map_elim.expr);
        dy_core_expr_retain_ptr(expr.type_map_elim.map.binding.type);
        dy_core_expr_retain_ptr(expr.type_map_elim.map.expr);

        return expr;
    case DY_CORE_EXPR_JUNCTION:
        dy_core_expr_retain_ptr(expr.junction.e1);
        dy_core_expr_retain_ptr(expr.junction.e2);

        return expr;
    case DY_CORE_EXPR_ALTERNATIVE:
        dy_core_expr_retain_ptr(expr.alternative.first);
        dy_core_expr_retain_ptr(expr.alternative.second);

        return expr;
    case DY_CORE_EXPR_VARIABLE:
        dy_core_expr_retain_ptr(expr.variable.type);

        return expr;
    case DY_CORE_EXPR_INFERENCE_VARIABLE:
        dy_core_expr_retain_ptr(expr.inference_variable.type);

        return expr;
    case DY_CORE_EXPR_INFERENCE_TYPE_MAP:
        dy_core_expr_retain_ptr(expr.inference_type_map.binding.type);
        dy_core_expr_retain_ptr(expr.inference_type_map.expr);

        return expr;
    case DY_CORE_EXPR_RECURSION:
        dy_core_expr_retain_ptr(expr.recursion.expr);

        return expr;
    case DY_CORE_EXPR_END:
        // fallthrough
    case DY_CORE_EXPR_SYMBOL:
        return expr;
    case DY_CORE_EXPR_CUSTOM:
        expr.custom.retain(expr.custom.data);
        return expr;
    }

    dy_bail("Impossible object type.");
}

void dy_core_expr_release_ptr(const struct dy_core_expr *expr)
{
    struct dy_core_expr shallow_copy = *expr;
    if (dy_rc_release(expr, dy_core_expr_pre_padding, dy_core_expr_post_padding) == 0) {
        dy_core_expr_release(shallow_copy);
    }
}

void dy_core_expr_release(struct dy_core_expr expr)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_EQUALITY_MAP:
        dy_core_expr_release_ptr(expr.equality_map.e1);
        dy_core_expr_release_ptr(expr.equality_map.e2);

        return;
    case DY_CORE_EXPR_TYPE_MAP:
        dy_core_expr_release_ptr(expr.type_map.binding.type);
        dy_core_expr_release_ptr(expr.type_map.expr);

        return;
    case DY_CORE_EXPR_EQUALITY_MAP_ELIM:
        dy_core_expr_release_ptr(expr.equality_map_elim.expr);
        dy_core_expr_release_ptr(expr.equality_map_elim.map.e1);
        dy_core_expr_release_ptr(expr.equality_map_elim.map.e2);

        return;
    case DY_CORE_EXPR_TYPE_MAP_ELIM:
        dy_core_expr_release_ptr(expr.type_map_elim.expr);
        dy_core_expr_release_ptr(expr.type_map_elim.map.binding.type);
        dy_core_expr_release_ptr(expr.type_map_elim.map.expr);

        return;
    case DY_CORE_EXPR_JUNCTION:
        dy_core_expr_release_ptr(expr.junction.e1);
        dy_core_expr_release_ptr(expr.junction.e2);

        return;
    case DY_CORE_EXPR_ALTERNATIVE:
        dy_core_expr_release_ptr(expr.alternative.first);
        dy_core_expr_release_ptr(expr.alternative.second);

        return;
    case DY_CORE_EXPR_VARIABLE:
        dy_core_expr_release_ptr(expr.variable.type);

        return;
    case DY_CORE_EXPR_INFERENCE_VARIABLE:
        dy_core_expr_release_ptr(expr.inference_variable.type);

        return;
    case DY_CORE_EXPR_INFERENCE_TYPE_MAP:
        dy_core_expr_release_ptr(expr.inference_type_map.binding.type);
        dy_core_expr_release_ptr(expr.inference_type_map.expr);

        return;
    case DY_CORE_EXPR_RECURSION:
        dy_core_expr_release_ptr(expr.recursion.expr);

        return;
    case DY_CORE_EXPR_END:
        // fallthrough
    case DY_CORE_EXPR_SYMBOL:
        return;
    case DY_CORE_EXPR_CUSTOM:
        expr.custom.release(expr.custom.data);
        return;
    }

    dy_bail("Impossible object type.");
}

void dy_core_expr_to_string(struct dy_core_expr expr, dy_array_t *string)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_EQUALITY_MAP:
        add_string(string, DY_STR_LIT("("));
        dy_core_expr_to_string(*expr.equality_map.e1, string);
        if (expr.equality_map.is_implicit) {
            add_string(string, DY_STR_LIT(" @"));
        } else {
            add_string(string, DY_STR_LIT(" "));
        }
        if (expr.equality_map.polarity == DY_CORE_POLARITY_POSITIVE) {
            add_string(string, DY_STR_LIT("-> "));
        } else {
            add_string(string, DY_STR_LIT("~> "));
        }
        dy_core_expr_to_string(*expr.equality_map.e2, string);
        add_string(string, DY_STR_LIT(")"));
        return;
    case DY_CORE_EXPR_TYPE_MAP:
        add_string(string, DY_STR_LIT("("));

        add_string(string, DY_STR_LIT("["));
        add_size_t_decimal(string, expr.type_map.binding.id);
        add_string(string, DY_STR_LIT(" "));
        dy_core_expr_to_string(*expr.type_map.binding.type, string);
        add_string(string, DY_STR_LIT("]"));
        if (expr.type_map.is_implicit) {
            add_string(string, DY_STR_LIT(" @"));
        } else {
            add_string(string, DY_STR_LIT(" "));
        }
        if (expr.type_map.polarity == DY_CORE_POLARITY_POSITIVE) {
            add_string(string, DY_STR_LIT("-> "));
        } else {
            add_string(string, DY_STR_LIT("~> "));
        }
        dy_core_expr_to_string(*expr.type_map.expr, string);
        add_string(string, DY_STR_LIT(")"));
        return;
    case DY_CORE_EXPR_INFERENCE_TYPE_MAP:
        add_string(string, DY_STR_LIT("("));

        add_string(string, DY_STR_LIT("?["));
        add_size_t_decimal(string, expr.inference_type_map.binding.id);
        add_string(string, DY_STR_LIT(" "));
        dy_core_expr_to_string(*expr.inference_type_map.binding.type, string);
        add_string(string, DY_STR_LIT("]"));
        if (expr.inference_type_map.polarity == DY_CORE_POLARITY_POSITIVE) {
            add_string(string, DY_STR_LIT(" -> "));
        } else {
            add_string(string, DY_STR_LIT(" ~> "));
        }
        dy_core_expr_to_string(*expr.inference_type_map.expr, string);
        add_string(string, DY_STR_LIT(")"));
        return;
    case DY_CORE_EXPR_EQUALITY_MAP_ELIM:
        add_string(string, DY_STR_LIT("("));
        dy_core_expr_to_string(*expr.equality_map_elim.expr, string);
        switch (expr.equality_map_elim.check_result) {
        case DY_YES:
            add_string(string, DY_STR_LIT(" \x1b[32m!\x1b[0m "));
            break;
        case DY_MAYBE:
            add_string(string, DY_STR_LIT(" \x1b[33m!\x1b[0m "));
            break;
        case DY_NO:
            add_string(string, DY_STR_LIT(" \x1b[31m!\x1b[0m "));
            break;
        }
        dy_core_expr_to_string(*expr.equality_map_elim.map.e1, string);
        if (expr.equality_map_elim.map.is_implicit) {
            add_string(string, DY_STR_LIT(" @"));
        } else {
            add_string(string, DY_STR_LIT(" "));
        }
        add_string(string, DY_STR_LIT("~> "));
        dy_core_expr_to_string(*expr.equality_map_elim.map.e2, string);
        add_string(string, DY_STR_LIT(")"));
        return;
    case DY_CORE_EXPR_TYPE_MAP_ELIM:
        add_string(string, DY_STR_LIT("("));
        dy_core_expr_to_string(*expr.type_map_elim.expr, string);
        add_string(string, DY_STR_LIT(" ! ["));
        add_size_t_decimal(string, expr.type_map_elim.map.binding.id);
        add_string(string, DY_STR_LIT(" "));
        dy_core_expr_to_string(*expr.type_map_elim.map.binding.type, string);
        add_string(string, DY_STR_LIT("]"));
        if (expr.type_map_elim.map.is_implicit) {
            add_string(string, DY_STR_LIT(" @"));
        } else {
            add_string(string, DY_STR_LIT(" "));
        }
        add_string(string, DY_STR_LIT("~> "));
        dy_core_expr_to_string(*expr.type_map_elim.map.expr, string);
        add_string(string, DY_STR_LIT(")"));
        return;
    case DY_CORE_EXPR_VARIABLE:
        add_size_t_decimal(string, expr.variable.id);
        return;
    case DY_CORE_EXPR_INFERENCE_VARIABLE:
        add_size_t_decimal(string, expr.inference_variable.id);
        return;
    case DY_CORE_EXPR_END:
        if (expr.end_polarity == DY_CORE_POLARITY_POSITIVE) {
            add_string(string, DY_STR_LIT("All"));
        } else if (expr.end_polarity == DY_CORE_POLARITY_NEGATIVE) {
            add_string(string, DY_STR_LIT("Any"));
        } else {
            dy_bail("Invalid polarity!\n");
        }

        return;
    case DY_CORE_EXPR_JUNCTION:
        add_string(string, DY_STR_LIT("("));
        dy_core_expr_to_string(*expr.junction.e1, string);

        if (expr.junction.polarity == DY_CORE_POLARITY_POSITIVE) {
            add_string(string, DY_STR_LIT(" and "));
        } else {
            add_string(string, DY_STR_LIT(" or "));
        }

        dy_core_expr_to_string(*expr.junction.e2, string);
        add_string(string, DY_STR_LIT(")"));
        return;
    case DY_CORE_EXPR_ALTERNATIVE:
        add_string(string, DY_STR_LIT("("));
        dy_core_expr_to_string(*expr.alternative.first, string);
        add_string(string, DY_STR_LIT(" else "));
        dy_core_expr_to_string(*expr.alternative.second, string);
        add_string(string, DY_STR_LIT(")"));
        return;
    case DY_CORE_EXPR_RECURSION:
        if (expr.recursion.polarity == DY_CORE_POLARITY_POSITIVE) {
            add_string(string, DY_STR_LIT("(prec "));
        } else {
            add_string(string, DY_STR_LIT("(nrec "));
        }

        add_size_t_decimal(string, expr.recursion.id);

        add_string(string, DY_STR_LIT(" = "));

        dy_core_expr_to_string(*expr.recursion.expr, string);
        add_string(string, DY_STR_LIT(")"));
        return;
    case DY_CORE_EXPR_SYMBOL:
        add_string(string, DY_STR_LIT("Symbol"));
        return;
    case DY_CORE_EXPR_CUSTOM:
        expr.custom.to_string(expr.custom.data, string);
        return;
    }

    dy_bail("Impossible object type.");
}

void add_string(dy_array_t *string, dy_string_t s)
{
    for (size_t i = 0; i < s.size; ++i) {
        dy_array_add(string, s.ptr + i);
    }
}

void add_size_t_decimal(dy_array_t *string, size_t x)
{
    size_t start_size = string->num_elems;

    // Add digits in reverse order.
    for (;;) {
        char digit = x % 10 + '0';

        dy_array_add(string, &digit);

        x /= 10;

        if (x == 0) {
            break;
        }
    }

    // Reverse digits in buffer.
    for (;;) {
        size_t i = start_size, k = string->num_elems - 1;
        char c;

        if (k >= i) {
            break;
        }

        dy_array_get(*string, i, &c);
        dy_array_set(*string, i, dy_array_pos(*string, k));
        dy_array_set(*string, k, &c);
    }
}
