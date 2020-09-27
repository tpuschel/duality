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
 * Context passed to pretty much all functions in core.
 */
struct dy_core_ctx {
    size_t running_id;

    dy_array_t bound_inference_vars;

    dy_array_t already_visited_ids;

    dy_array_t subtype_assumption_cache;

    dy_array_t supertype_assumption_cache;

    dy_array_t bindings;
    
    dy_array_t equal_variables;
    
    dy_array_t subtype_implicits;
    
    dy_array_t free_ids_arrays;

    dy_array_t constraints;

    dy_array_t custom_shared;
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

struct dy_core_equality_map {
    const struct dy_core_expr *e1;
    const struct dy_core_expr *e2;
    enum dy_core_polarity polarity;
    bool is_implicit;
};

struct dy_core_type_map {
    size_t id;
    const struct dy_core_expr *type;
    const struct dy_core_expr *expr;
    enum dy_core_polarity polarity;
    bool is_implicit;
};

struct dy_core_inference_type_map {
    size_t id;
    const struct dy_core_expr *type;
    const struct dy_core_expr *expr;
    enum dy_core_polarity polarity;
};

struct dy_core_equality_map_elim {
    const struct dy_core_expr *expr;
    struct dy_core_equality_map map;
    dy_ternary_t check_result;
};

struct dy_core_type_map_elim {
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
    const struct dy_core_expr *type;
    const struct dy_core_expr *expr;
    enum dy_core_polarity polarity;
    dy_ternary_t check_result;
};

struct dy_core_custom {
    size_t id;
    void *data;
};

struct dy_core_custom_shared {
    bool can_be_eliminated;

    struct dy_core_expr (*type_of)(void *data, struct dy_core_ctx *ctx);

    dy_ternary_t (*is_equal)(void *data, struct dy_core_ctx *ctx, struct dy_core_expr expr);

    bool (*check)(void *data, struct dy_core_ctx *ctx, struct dy_core_expr *result);

    bool (*remove_mentions_in_subtype)(void *data, struct dy_core_ctx *ctx, size_t id, struct dy_core_expr *result);

    bool (*remove_mentions_in_supertype)(void *data, struct dy_core_ctx *ctx, size_t id, struct dy_core_expr *result);

    struct dy_core_expr (*eval)(void *data, struct dy_core_ctx *ctx, bool *is_value);

    bool (*substitute)(void *data, struct dy_core_ctx *ctx, size_t id, struct dy_core_expr sub, struct dy_core_expr *result);

    dy_ternary_t (*is_subtype)(void *data, struct dy_core_ctx *ctx, struct dy_core_expr supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

    dy_ternary_t (*is_supertype)(void *data, struct dy_core_ctx *ctx, struct dy_core_expr subtype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

    struct dy_core_expr (*eliminate)(void *data, struct dy_core_ctx *ctx, struct dy_core_expr expr, bool *is_value);

    bool (*is_computation)(void *data, struct dy_core_ctx *ctx);

    bool (*is_bound)(void *data, struct dy_core_ctx *ctx, size_t id);

    void (*appears_in_polarity)(void *data, struct dy_core_ctx *ctx, size_t id, enum dy_core_polarity current_polarity, bool *in_positive, bool *in_negative);

    void *(*retain)(void *data, struct dy_core_ctx *ctx);

    void (*release)(void *data, struct dy_core_ctx *ctx);

    void (*to_string)(void *data, struct dy_core_ctx *ctx, dy_array_t *string);
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
        size_t variable_id;
        struct dy_core_inference_type_map inference_type_map;
        struct dy_core_recursion recursion;
        enum dy_core_polarity end_polarity;
        struct dy_core_custom custom;
    };

    enum dy_core_expr_tag tag;
};

struct dy_bound_inference_var {
    size_t id;
    struct dy_core_expr type;
    enum dy_core_polarity polarity;
    dy_array_t binding_ids;
};

struct dy_subtype_assumption {
    struct dy_core_expr type;
    size_t rec_binding_id;
};

struct dy_equal_variables {
    size_t id1;
    size_t id2;
};

struct dy_core_binding {
    size_t id;
    struct dy_core_expr type;
    bool is_inference_var;
    enum dy_core_polarity polarity;
};

static inline enum dy_core_polarity flip_polarity(enum dy_core_polarity polarity);

/**
 * Determines if an expression is syntactically a value or a computation.
 */
static inline bool dy_core_expr_is_computation(struct dy_core_ctx *ctx, struct dy_core_expr expr);

/** Returns whether 'id' occurs in expr at all. */
static inline bool dy_core_expr_is_bound(struct dy_core_ctx *ctx, size_t id, struct dy_core_expr expr);

/** Appends a utf8 represention of expr to 'string'. */
static inline void dy_core_expr_to_string(struct dy_core_ctx *ctx, struct dy_core_expr expr, dy_array_t *string);

static inline void dy_core_appears_in_polarity(struct dy_core_ctx *ctx, size_t id, struct dy_core_expr expr, enum dy_core_polarity current_polarity, bool *in_positive, bool *in_negative);


/** Boring ref-count stuff. */
static const size_t dy_core_expr_pre_padding = DY_RC_PRE_PADDING(struct dy_core_expr);
static const size_t dy_core_expr_post_padding = DY_RC_POST_PADDING(struct dy_core_expr);

static inline const struct dy_core_expr *dy_core_expr_new(struct dy_core_expr expr);

static inline struct dy_core_expr dy_core_expr_retain(struct dy_core_ctx *ctx, struct dy_core_expr expr);

static inline const struct dy_core_expr *dy_core_expr_retain_ptr(const struct dy_core_expr *expr);

static inline void dy_core_expr_release(struct dy_core_ctx *ctx, struct dy_core_expr expr);

static inline void dy_core_expr_release_ptr(struct dy_core_ctx *ctx, const struct dy_core_expr *expr);

/** Helper to add a string literal to a dynamic char array. */
static inline void add_string(dy_array_t *string, dy_string_t s);

static inline void add_size_t_decimal(dy_array_t *string, size_t x);

enum dy_core_polarity flip_polarity(enum dy_core_polarity polarity)
{
    switch (polarity) {
    case DY_CORE_POLARITY_POSITIVE:
        return DY_CORE_POLARITY_NEGATIVE;
    case DY_CORE_POLARITY_NEGATIVE:
        return DY_CORE_POLARITY_POSITIVE;
    }

    dy_bail("Impossible polarity.");
}

bool dy_core_expr_is_computation(struct dy_core_ctx *ctx, struct dy_core_expr expr)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_EQUALITY_MAP:
        return dy_core_expr_is_computation(ctx, *expr.equality_map.e1) || dy_core_expr_is_computation(ctx, *expr.equality_map.e2);
    case DY_CORE_EXPR_TYPE_MAP:
        return dy_core_expr_is_computation(ctx, *expr.type_map.type) || dy_core_expr_is_computation(ctx, *expr.type_map.expr);
    case DY_CORE_EXPR_EQUALITY_MAP_ELIM:
        return true;
    case DY_CORE_EXPR_TYPE_MAP_ELIM:
        return true;
    case DY_CORE_EXPR_JUNCTION:
        return dy_core_expr_is_computation(ctx, *expr.junction.e1) || dy_core_expr_is_computation(ctx, *expr.junction.e2);
    case DY_CORE_EXPR_ALTERNATIVE:
        return dy_core_expr_is_computation(ctx, *expr.alternative.first) || dy_core_expr_is_computation(ctx, *expr.alternative.second);
    case DY_CORE_EXPR_VARIABLE:
        return false;
    case DY_CORE_EXPR_INFERENCE_TYPE_MAP:
        dy_bail("should never be reached");
    case DY_CORE_EXPR_RECURSION:
        return dy_core_expr_is_computation(ctx, *expr.recursion.expr);
    case DY_CORE_EXPR_END:
        // fallthrough
    case DY_CORE_EXPR_SYMBOL:
        return false;
    case DY_CORE_EXPR_CUSTOM: {
        const struct dy_core_custom_shared *s = dy_array_pos(ctx->custom_shared, expr.custom.id);
        return s->is_computation(expr.custom.data, ctx);
    }
    }

    dy_bail("Impossible object type.");
}

bool dy_core_expr_is_bound(struct dy_core_ctx *ctx, size_t id, struct dy_core_expr expr)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_EQUALITY_MAP:
        return dy_core_expr_is_bound(ctx, id, *expr.equality_map.e1) || dy_core_expr_is_bound(ctx, id, *expr.equality_map.e2);
    case DY_CORE_EXPR_TYPE_MAP: {
        if (dy_core_expr_is_bound(ctx, id, *expr.type_map.type)) {
            return true;
        }

        if (id == expr.type_map.id) {
            return false;
        }

        return dy_core_expr_is_bound(ctx, id, *expr.type_map.expr);
    }
    case DY_CORE_EXPR_EQUALITY_MAP_ELIM:
        return dy_core_expr_is_bound(ctx, id, *expr.equality_map_elim.expr) || dy_core_expr_is_bound(ctx, id, *expr.equality_map_elim.map.e1) || dy_core_expr_is_bound(ctx, id, *expr.equality_map_elim.map.e2);
    case DY_CORE_EXPR_TYPE_MAP_ELIM: {
        if (dy_core_expr_is_bound(ctx, id, *expr.type_map_elim.expr) || dy_core_expr_is_bound(ctx, id, *expr.type_map_elim.map.type)) {
            return true;
        }

        if (id == expr.type_map_elim.map.id) {
            return false;
        }

        return dy_core_expr_is_bound(ctx, id, *expr.type_map_elim.map.expr);
    }
    case DY_CORE_EXPR_JUNCTION:
        return dy_core_expr_is_bound(ctx, id, *expr.junction.e1) || dy_core_expr_is_bound(ctx, id, *expr.junction.e2);
    case DY_CORE_EXPR_ALTERNATIVE:
        return dy_core_expr_is_bound(ctx, id, *expr.alternative.first) || dy_core_expr_is_bound(ctx, id, *expr.alternative.second);
    case DY_CORE_EXPR_VARIABLE:
        return expr.variable_id == id;
    case DY_CORE_EXPR_INFERENCE_TYPE_MAP:
        dy_bail("should never be reached");
    case DY_CORE_EXPR_RECURSION: {
        if (dy_core_expr_is_bound(ctx, id, *expr.recursion.type)) {
            return true;
        }

        if (id == expr.recursion.id) {
            return false;
        }

        return dy_core_expr_is_bound(ctx, id, *expr.recursion.expr);
    }
    case DY_CORE_EXPR_END:
        // fallthrough
    case DY_CORE_EXPR_SYMBOL:
        return false;
    case DY_CORE_EXPR_CUSTOM: {
        const struct dy_core_custom_shared *s = dy_array_pos(ctx->custom_shared, expr.custom.id);
        return s->is_bound(expr.custom.data, ctx, id);
    }
    }

    dy_bail("Impossible object type.");
}

void dy_core_appears_in_polarity(struct dy_core_ctx *ctx, size_t id, struct dy_core_expr expr, enum dy_core_polarity current_polarity, bool *in_positive, bool *in_negative)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_EQUALITY_MAP:
        dy_core_appears_in_polarity(ctx, id, *expr.equality_map.e2, current_polarity, in_positive, in_negative);
        return;
    case DY_CORE_EXPR_TYPE_MAP:
        dy_core_appears_in_polarity(ctx, id, *expr.type_map.type, flip_polarity(current_polarity), in_positive, in_negative);
        if (*in_negative && *in_positive) {
            return;
        }

        dy_core_appears_in_polarity(ctx, id, *expr.type_map.expr, current_polarity, in_positive, in_negative);

        return;
    case DY_CORE_EXPR_EQUALITY_MAP_ELIM:
        return;
    case DY_CORE_EXPR_TYPE_MAP_ELIM:
        return;
    case DY_CORE_EXPR_JUNCTION:
        dy_core_appears_in_polarity(ctx, id, *expr.junction.e1, current_polarity, in_positive, in_negative);
        if (*in_positive && *in_negative) {
            return;
        }

        dy_core_appears_in_polarity(ctx, id, *expr.junction.e2, current_polarity, in_positive, in_negative);

        return;
    case DY_CORE_EXPR_ALTERNATIVE:
        return;
    case DY_CORE_EXPR_VARIABLE:
        if (expr.variable_id == id) {
            switch (current_polarity) {
            case DY_CORE_POLARITY_POSITIVE:
                *in_positive = true;
                return;
            case DY_CORE_POLARITY_NEGATIVE:
                *in_negative = true;
                return;
            }

            dy_bail("Impossible polarity.");
        }

        return;
    case DY_CORE_EXPR_INFERENCE_TYPE_MAP:
        dy_bail("should never be reached");
    case DY_CORE_EXPR_RECURSION:
        dy_core_appears_in_polarity(ctx, id, *expr.recursion.type, flip_polarity(current_polarity), in_positive, in_negative);
        if (*in_positive && *in_negative) {
            return;
        }

        dy_core_appears_in_polarity(ctx, id, *expr.recursion.expr, current_polarity, in_positive, in_negative);
        return;
    case DY_CORE_EXPR_END:
        return;
    case DY_CORE_EXPR_SYMBOL:
        return;
    case DY_CORE_EXPR_CUSTOM: {
        const struct dy_core_custom_shared *s = dy_array_pos(ctx->custom_shared, expr.custom.id);
        s->appears_in_polarity(expr.custom.data, ctx, id, current_polarity, in_positive, in_negative);
        return;
    }
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

struct dy_core_expr dy_core_expr_retain(struct dy_core_ctx *ctx, struct dy_core_expr expr)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_EQUALITY_MAP:
        dy_core_expr_retain_ptr(expr.equality_map.e1);
        dy_core_expr_retain_ptr(expr.equality_map.e2);

        return expr;
    case DY_CORE_EXPR_TYPE_MAP:
        dy_core_expr_retain_ptr(expr.type_map.type);
        dy_core_expr_retain_ptr(expr.type_map.expr);

        return expr;
    case DY_CORE_EXPR_EQUALITY_MAP_ELIM:
        dy_core_expr_retain_ptr(expr.equality_map_elim.expr);
        dy_core_expr_retain_ptr(expr.equality_map_elim.map.e1);
        dy_core_expr_retain_ptr(expr.equality_map_elim.map.e2);

        return expr;
    case DY_CORE_EXPR_TYPE_MAP_ELIM:
        dy_core_expr_retain_ptr(expr.type_map_elim.expr);
        dy_core_expr_retain_ptr(expr.type_map_elim.map.type);
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
        return expr;
    case DY_CORE_EXPR_INFERENCE_TYPE_MAP:
        dy_core_expr_retain_ptr(expr.inference_type_map.type);
        dy_core_expr_retain_ptr(expr.inference_type_map.expr);

        return expr;
    case DY_CORE_EXPR_RECURSION:
        dy_core_expr_retain_ptr(expr.recursion.type);
        dy_core_expr_retain_ptr(expr.recursion.expr);

        return expr;
    case DY_CORE_EXPR_END:
        // fallthrough
    case DY_CORE_EXPR_SYMBOL:
        return expr;
    case DY_CORE_EXPR_CUSTOM: {
        const struct dy_core_custom_shared *s = dy_array_pos(ctx->custom_shared, expr.custom.id);
        s->retain(expr.custom.data, ctx);
        return expr;
    }
    }

    dy_bail("Impossible object type.");
}

void dy_core_expr_release_ptr(struct dy_core_ctx *ctx, const struct dy_core_expr *expr)
{
    struct dy_core_expr shallow_copy = *expr;
    if (dy_rc_release(expr, dy_core_expr_pre_padding, dy_core_expr_post_padding) == 0) {
        dy_core_expr_release(ctx, shallow_copy);
    }
}

void dy_core_expr_release(struct dy_core_ctx *ctx, struct dy_core_expr expr)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_EQUALITY_MAP:
        dy_core_expr_release_ptr(ctx, expr.equality_map.e1);
        dy_core_expr_release_ptr(ctx, expr.equality_map.e2);

        return;
    case DY_CORE_EXPR_TYPE_MAP:
        dy_core_expr_release_ptr(ctx, expr.type_map.type);
        dy_core_expr_release_ptr(ctx, expr.type_map.expr);

        return;
    case DY_CORE_EXPR_EQUALITY_MAP_ELIM:
        dy_core_expr_release_ptr(ctx, expr.equality_map_elim.expr);
        dy_core_expr_release_ptr(ctx, expr.equality_map_elim.map.e1);
        dy_core_expr_release_ptr(ctx, expr.equality_map_elim.map.e2);

        return;
    case DY_CORE_EXPR_TYPE_MAP_ELIM:
        dy_core_expr_release_ptr(ctx, expr.type_map_elim.expr);
        dy_core_expr_release_ptr(ctx, expr.type_map_elim.map.type);
        dy_core_expr_release_ptr(ctx, expr.type_map_elim.map.expr);

        return;
    case DY_CORE_EXPR_JUNCTION:
        dy_core_expr_release_ptr(ctx, expr.junction.e1);
        dy_core_expr_release_ptr(ctx, expr.junction.e2);

        return;
    case DY_CORE_EXPR_ALTERNATIVE:
        dy_core_expr_release_ptr(ctx, expr.alternative.first);
        dy_core_expr_release_ptr(ctx, expr.alternative.second);

        return;
    case DY_CORE_EXPR_VARIABLE:
        return;
    case DY_CORE_EXPR_INFERENCE_TYPE_MAP:
        dy_core_expr_release_ptr(ctx, expr.inference_type_map.type);
        dy_core_expr_release_ptr(ctx, expr.inference_type_map.expr);

        return;
    case DY_CORE_EXPR_RECURSION:
        dy_core_expr_release_ptr(ctx, expr.recursion.type);
        dy_core_expr_release_ptr(ctx, expr.recursion.expr);

        return;
    case DY_CORE_EXPR_END:
        // fallthrough
    case DY_CORE_EXPR_SYMBOL:
        return;
    case DY_CORE_EXPR_CUSTOM: {
        const struct dy_core_custom_shared *s = dy_array_pos(ctx->custom_shared, expr.custom.id);
        s->release(expr.custom.data, ctx);
    }
        return;
    }

    dy_bail("Impossible object type.");
}

void dy_core_expr_to_string(struct dy_core_ctx *ctx, struct dy_core_expr expr, dy_array_t *string)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_EQUALITY_MAP:
        add_string(string, DY_STR_LIT("("));
        dy_core_expr_to_string(ctx, *expr.equality_map.e1, string);
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
        dy_core_expr_to_string(ctx, *expr.equality_map.e2, string);
        add_string(string, DY_STR_LIT(")"));
        return;
    case DY_CORE_EXPR_TYPE_MAP:
        add_string(string, DY_STR_LIT("("));

        add_string(string, DY_STR_LIT("["));
        add_size_t_decimal(string, expr.type_map.id);
        add_string(string, DY_STR_LIT(" "));
        dy_core_expr_to_string(ctx, *expr.type_map.type, string);
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
        dy_core_expr_to_string(ctx, *expr.type_map.expr, string);
        add_string(string, DY_STR_LIT(")"));
        return;
    case DY_CORE_EXPR_INFERENCE_TYPE_MAP:
        add_string(string, DY_STR_LIT("("));

        add_string(string, DY_STR_LIT("?["));
        add_size_t_decimal(string, expr.inference_type_map.id);
        add_string(string, DY_STR_LIT(" "));
        dy_core_expr_to_string(ctx, *expr.inference_type_map.type, string);
        add_string(string, DY_STR_LIT("]"));
        if (expr.inference_type_map.polarity == DY_CORE_POLARITY_POSITIVE) {
            add_string(string, DY_STR_LIT(" -> "));
        } else {
            add_string(string, DY_STR_LIT(" ~> "));
        }
        dy_core_expr_to_string(ctx, *expr.inference_type_map.expr, string);
        add_string(string, DY_STR_LIT(")"));
        return;
    case DY_CORE_EXPR_EQUALITY_MAP_ELIM:
        add_string(string, DY_STR_LIT("("));
        dy_core_expr_to_string(ctx, *expr.equality_map_elim.expr, string);
        switch (expr.equality_map_elim.check_result) {
        case DY_YES:
            add_string(string, DY_STR_LIT(" O! "));
            break;
        case DY_MAYBE:
            add_string(string, DY_STR_LIT(" M! "));
            break;
        case DY_NO:
            add_string(string, DY_STR_LIT(" X! "));
            break;
        }
        dy_core_expr_to_string(ctx, *expr.equality_map_elim.map.e1, string);
        if (expr.equality_map_elim.map.is_implicit) {
            add_string(string, DY_STR_LIT(" @"));
        } else {
            add_string(string, DY_STR_LIT(" "));
        }
        add_string(string, DY_STR_LIT("~> "));
        dy_core_expr_to_string(ctx, *expr.equality_map_elim.map.e2, string);
        add_string(string, DY_STR_LIT(")"));
        return;
    case DY_CORE_EXPR_TYPE_MAP_ELIM:
        add_string(string, DY_STR_LIT("("));
        dy_core_expr_to_string(ctx, *expr.type_map_elim.expr, string);
        add_string(string, DY_STR_LIT(" ! ["));
        add_size_t_decimal(string, expr.type_map_elim.map.id);
        add_string(string, DY_STR_LIT(" "));
        dy_core_expr_to_string(ctx, *expr.type_map_elim.map.type, string);
        add_string(string, DY_STR_LIT("]"));
        if (expr.type_map_elim.map.is_implicit) {
            add_string(string, DY_STR_LIT(" @"));
        } else {
            add_string(string, DY_STR_LIT(" "));
        }
        add_string(string, DY_STR_LIT("~> "));
        dy_core_expr_to_string(ctx, *expr.type_map_elim.map.expr, string);
        add_string(string, DY_STR_LIT(")"));
        return;
    case DY_CORE_EXPR_VARIABLE:
        add_size_t_decimal(string, expr.variable_id);
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
        dy_core_expr_to_string(ctx, *expr.junction.e1, string);

        if (expr.junction.polarity == DY_CORE_POLARITY_POSITIVE) {
            add_string(string, DY_STR_LIT(" and "));
        } else {
            add_string(string, DY_STR_LIT(" or "));
        }

        dy_core_expr_to_string(ctx, *expr.junction.e2, string);
        add_string(string, DY_STR_LIT(")"));
        return;
    case DY_CORE_EXPR_ALTERNATIVE:
        add_string(string, DY_STR_LIT("("));
        dy_core_expr_to_string(ctx, *expr.alternative.first, string);
        add_string(string, DY_STR_LIT(" else "));
        dy_core_expr_to_string(ctx, *expr.alternative.second, string);
        add_string(string, DY_STR_LIT(")"));
        return;
    case DY_CORE_EXPR_RECURSION:
        if (expr.recursion.polarity == DY_CORE_POLARITY_POSITIVE) {
            add_string(string, DY_STR_LIT("(prec "));
        } else {
            add_string(string, DY_STR_LIT("(nrec "));
        }

        add_size_t_decimal(string, expr.recursion.id);
        add_string(string, DY_STR_LIT(" "));
        dy_core_expr_to_string(ctx, *expr.recursion.type, string);

        add_string(string, DY_STR_LIT(" = "));

        dy_core_expr_to_string(ctx, *expr.recursion.expr, string);
        add_string(string, DY_STR_LIT(")"));
        return;
    case DY_CORE_EXPR_SYMBOL:
        add_string(string, DY_STR_LIT("Symbol"));
        return;
    case DY_CORE_EXPR_CUSTOM: {
        const struct dy_core_custom_shared *s = dy_array_pos(ctx->custom_shared, expr.custom.id);
        s->to_string(expr.custom.data, ctx, string);
        return;
    }
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
    for (size_t i = start_size, k = string->num_elems - 1; i < k; ++i, --k) {
        char c;
        dy_array_get(*string, i, &c);
        dy_array_set(*string, i, dy_array_pos(*string, k));
        dy_array_set(*string, k, &c);
    }
}
