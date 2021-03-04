/*
 * Copyright 2017-2021 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "../support/string.h"
#include "../support/array.h"
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

    dy_array_t free_variables;

    dy_array_t captured_inference_vars;

    dy_array_t recovered_negative_inference_ids;

    dy_array_t recovered_positive_inference_ids;

    dy_array_t past_subtype_checks;

    dy_array_t constraints;

    dy_array_t equal_variables;

    dy_array_t free_ids_arrays;

    dy_array_t custom_shared;
};

typedef enum dy_ternary {
    DY_YES,
    DY_NO,
    DY_MAYBE
} dy_ternary_t;

struct dy_core_expr;

struct dy_core_assumption {
    size_t id;
    struct dy_core_expr *type;
    struct dy_core_expr *expr;
};

struct dy_core_choice {
    struct dy_core_expr *left;
    struct dy_core_expr *right;
};

struct dy_core_recursion {
    size_t id;
    struct dy_core_expr *expr;
};

enum dy_core_complex_tag {
    DY_CORE_COMPLEX_ASSUMPTION,
    DY_CORE_COMPLEX_CHOICE,
    DY_CORE_COMPLEX_RECURSION
};

struct dy_core_complex {
    union {
        struct dy_core_assumption assumption;
        struct dy_core_choice choice;
        struct dy_core_recursion recursion;
    };

    enum dy_core_complex_tag tag;
};

enum dy_direction {
    DY_LEFT,
    DY_RIGHT
};

enum dy_core_simple_tag {
    DY_CORE_SIMPLE_PROOF,
    DY_CORE_SIMPLE_DECISION,
    DY_CORE_SIMPLE_UNFOLD,
    DY_CORE_SIMPLE_UNWRAP
};

struct dy_core_simple {
    union {
        struct dy_core_expr *proof;
        enum dy_direction direction;
    };

    enum dy_core_simple_tag tag;

    struct dy_core_expr *out;
};

enum dy_polarity {
    DY_POLARITY_POSITIVE,
    DY_POLARITY_NEGATIVE
};

enum dy_core_intro_tag {
    DY_CORE_INTRO_COMPLEX,
    DY_CORE_INTRO_SIMPLE
};

struct dy_core_intro {
    union {
        struct dy_core_complex complex;
        struct dy_core_simple simple;
    };

    enum dy_core_intro_tag tag;

    enum dy_polarity polarity;
    bool is_implicit;
};

struct dy_core_elim {
    struct dy_core_expr *expr;
    struct dy_core_simple simple;
    bool is_implicit;
    dy_ternary_t check_result;
    bool eval_immediately;
};

enum dy_core_map_dependence {
    DY_CORE_MAP_DEPENDENCE_NOT_CHECKED,
    DY_CORE_MAP_DEPENDENCE_DEPENDENT,
    DY_CORE_MAP_DEPENDENCE_INDEPENDENT
};

struct dy_core_map_assumption {
    size_t id;
    struct dy_core_expr *type;
    struct dy_core_assumption assumption;
    enum dy_core_map_dependence dependence;
};

struct dy_core_map_choice {
    struct dy_core_assumption assumption_left;
    struct dy_core_assumption assumption_right;
    enum dy_core_map_dependence left_dependence;
    enum dy_core_map_dependence right_dependence;
};

struct dy_core_map_recursion {
    size_t id;
    struct dy_core_assumption assumption;
    enum dy_core_map_dependence dependence;
};

enum dy_core_map_tag {
    DY_CORE_MAP_ASSUMPTION,
    DY_CORE_MAP_CHOICE,
    DY_CORE_MAP_RECURSION
};

struct dy_core_map {
    union {
        struct dy_core_map_assumption assumption;
        struct dy_core_map_choice choice;
        struct dy_core_map_recursion recursion;
    };

    enum dy_core_map_tag tag;

    bool is_implicit;
};

struct dy_core_inference_ctx {
    size_t id;
    enum dy_polarity polarity;
    struct dy_core_expr *expr;
};

struct dy_core_custom {
    size_t id;
    void *data;
};

struct dy_core_custom_shared {
    struct dy_core_expr (*type_of)(struct dy_core_ctx *ctx, void *data);

    dy_ternary_t (*is_equal)(struct dy_core_ctx *ctx, void *data1, void *data2);

    bool (*check)(struct dy_core_ctx *ctx, void *data, struct dy_core_expr *result);

    bool (*remove_mentions_in_type)(struct dy_core_ctx *ctx, void *data, size_t id, enum dy_polarity current_polarity, struct dy_core_expr *result);

    bool (*eval)(struct dy_core_ctx *ctx, void *data, bool *is_value, struct dy_core_expr *result);

    bool (*substitute)(struct dy_core_ctx *ctx, void *data, size_t id, struct dy_core_expr sub, struct dy_core_expr *result);

    dy_ternary_t (*is_subtype)(struct dy_core_ctx *ctx, void *subtype, void *supertype, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

    bool (*contains_this_variable)(struct dy_core_ctx *ctx, void *data, size_t id);

    void (*variable_appears_in_polarity)(struct dy_core_ctx *ctx, void *data, size_t id, enum dy_polarity current_polarity, bool *positive, bool *negative);

    void *(*retain)(struct dy_core_ctx *ctx, void *data);

    void (*release)(struct dy_core_ctx *ctx, void *data);

    void (*to_string)(struct dy_core_ctx *ctx, void *data, dy_array_t *string);
};

enum dy_core_expr_tag {
    DY_CORE_EXPR_INTRO,
    DY_CORE_EXPR_ELIM,
    DY_CORE_EXPR_MAP,
    DY_CORE_EXPR_VARIABLE,
    DY_CORE_EXPR_ANY,
    DY_CORE_EXPR_VOID,
    DY_CORE_EXPR_INFERENCE_CTX,
    DY_CORE_EXPR_INFERENCE_VAR,
    DY_CORE_EXPR_CUSTOM
};

struct dy_core_expr {
    union {
        struct dy_core_intro intro;
        struct dy_core_elim elim;
        struct dy_core_map map;
        size_t variable_id;
        struct dy_core_inference_ctx inference_ctx;
        size_t inference_var_id;
        struct dy_core_custom custom;
    };

    enum dy_core_expr_tag tag;
};

struct dy_free_var {
    size_t id;
    struct dy_core_expr type;
};

struct dy_captured_inference_var {
    size_t id;
    dy_array_t captor_ids;
};

struct dy_equal_variables {
    size_t id1;
    size_t id2;
};

struct dy_core_past_subtype_check {
    struct dy_core_expr subtype;
    struct dy_core_expr supertype;
    size_t substitute_var_id;
    bool have_substitute_var_id;
};

static inline struct dy_core_expr *dy_core_expr_new(struct dy_core_expr expr);

static inline struct dy_core_expr dy_core_expr_retain(struct dy_core_ctx *ctx, struct dy_core_expr expr);
static inline struct dy_core_expr *dy_core_expr_retain_ptr(struct dy_core_ctx *ctx, struct dy_core_expr *expr);
static inline struct dy_core_assumption dy_core_assumption_retain(struct dy_core_ctx *ctx, struct dy_core_assumption assumption);
static inline struct dy_core_simple dy_core_simple_retain(struct dy_core_ctx *ctx, struct dy_core_simple simple);


static inline void dy_core_expr_release(struct dy_core_ctx *ctx, struct dy_core_expr expr);
static inline void dy_core_expr_release_ptr(struct dy_core_ctx *ctx, struct dy_core_expr *expr);
static inline void dy_core_assumption_release(struct dy_core_ctx *ctx, struct dy_core_assumption assumption);
static inline void dy_core_simple_release(struct dy_core_ctx *ctx, struct dy_core_simple simple);

static inline enum dy_polarity dy_flip_polarity(enum dy_polarity polarity);

static inline void dy_variable_appears_in_polarity(struct dy_core_ctx *ctx, struct dy_core_expr expr, size_t id, enum dy_polarity current_polarity, bool *positive, bool *negative);

/** Returns whether 'id' occurs in expr at all. */
static inline bool dy_core_expr_contains_this_variable(struct dy_core_ctx *ctx, size_t id, struct dy_core_expr expr);
static inline bool dy_core_assumption_contains_this_variable(struct dy_core_ctx *ctx, size_t id, struct dy_core_assumption assumption);

/**
 * Removes all mentions of 'id' in 'constraint', which may involve lowering/raising the subtype/supertype bounds.
 */
static inline void dy_remove_mentions_in_constraints(struct dy_core_ctx *ctx, size_t id, size_t start);

/**
 * Removes all mentions of 'id' in 'type'.
 */
static inline bool dy_remove_mentions_in_type(struct dy_core_ctx *ctx, size_t id, enum dy_polarity current_polarity, struct dy_core_expr type, struct dy_core_expr *result);

/** Appends a utf8 represention of expr to 'string'. */
static inline void dy_core_expr_to_string(struct dy_core_ctx *ctx, struct dy_core_expr expr, dy_array_t *string);

/** Helper to add a string literal to a dynamic char array. */
static inline void add_string(dy_array_t *string, dy_string_t s);

static inline void add_size_t_decimal(dy_array_t *string, size_t x);

struct dy_core_expr *dy_core_expr_new(struct dy_core_expr expr)
{
    return dy_rc_new(&expr, sizeof expr, DY_ALIGNOF(struct dy_core_expr));
}

struct dy_core_expr dy_core_expr_retain(struct dy_core_ctx *ctx, struct dy_core_expr expr)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_INTRO:
        switch (expr.intro.tag) {
        case DY_CORE_INTRO_COMPLEX:
            switch (expr.intro.complex.tag) {
            case DY_CORE_COMPLEX_ASSUMPTION:
                dy_core_assumption_retain(ctx, expr.intro.complex.assumption);
                return expr;
            case DY_CORE_COMPLEX_CHOICE:
                dy_core_expr_retain_ptr(ctx, expr.intro.complex.choice.left);
                dy_core_expr_retain_ptr(ctx, expr.intro.complex.choice.right);
                return expr;
            case DY_CORE_COMPLEX_RECURSION:
                dy_core_expr_retain_ptr(ctx, expr.intro.complex.recursion.expr);
                return expr;
            }
            dy_bail("impossible");
        case DY_CORE_INTRO_SIMPLE:
            dy_core_simple_retain(ctx, expr.intro.simple);
            return expr;
        }

        dy_bail("impossible");
    case DY_CORE_EXPR_ELIM:
        dy_core_expr_retain_ptr(ctx, expr.elim.expr);
        dy_core_simple_retain(ctx, expr.elim.simple);
        return expr;
    case DY_CORE_EXPR_MAP:
        switch (expr.map.tag) {
        case DY_CORE_MAP_ASSUMPTION:
            dy_core_expr_retain_ptr(ctx, expr.map.assumption.type);
            dy_core_assumption_retain(ctx, expr.map.assumption.assumption);
            return expr;
        case DY_CORE_MAP_CHOICE:
            dy_core_assumption_retain(ctx, expr.map.choice.assumption_left);
            dy_core_assumption_retain(ctx, expr.map.choice.assumption_right);
            return expr;
        case DY_CORE_MAP_RECURSION:
            dy_core_assumption_retain(ctx, expr.map.recursion.assumption);
            return expr;
        }

        dy_bail("impossible");
    case DY_CORE_EXPR_VARIABLE:
    case DY_CORE_EXPR_ANY:
    case DY_CORE_EXPR_VOID:
    case DY_CORE_EXPR_INFERENCE_VAR:
        return expr;
    case DY_CORE_EXPR_INFERENCE_CTX:
        dy_core_expr_retain_ptr(ctx, expr.inference_ctx.expr);
        return expr;
    case DY_CORE_EXPR_CUSTOM: {
        const struct dy_core_custom_shared *s = dy_array_pos(&ctx->custom_shared, expr.custom.id);
        s->retain(ctx, expr.custom.data);
        return expr;
    }
    }

    dy_bail("Impossible object type.");
}

struct dy_core_expr *dy_core_expr_retain_ptr(struct dy_core_ctx *ctx, struct dy_core_expr *expr)
{
    return dy_rc_retain(expr, DY_ALIGNOF(struct dy_core_expr));
}

struct dy_core_assumption dy_core_assumption_retain(struct dy_core_ctx *ctx, struct dy_core_assumption assumption)
{
    dy_core_expr_retain_ptr(ctx, assumption.type);
    dy_core_expr_retain_ptr(ctx, assumption.expr);
    return assumption;
}

struct dy_core_simple dy_core_simple_retain(struct dy_core_ctx *ctx, struct dy_core_simple simple)
{
    if (simple.tag == DY_CORE_SIMPLE_PROOF) {
        dy_core_expr_retain_ptr(ctx, simple.proof);
    }

    dy_core_expr_retain_ptr(ctx, simple.out);
    return simple;
}

void dy_core_expr_release(struct dy_core_ctx *ctx, struct dy_core_expr expr)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_INTRO:
        switch (expr.intro.tag) {
        case DY_CORE_INTRO_COMPLEX:
            switch (expr.intro.complex.tag) {
            case DY_CORE_COMPLEX_ASSUMPTION:
                dy_core_expr_release_ptr(ctx, expr.intro.complex.assumption.type);
                dy_core_expr_release_ptr(ctx, expr.intro.complex.assumption.expr);
                return;
            case DY_CORE_COMPLEX_CHOICE:
                dy_core_expr_release_ptr(ctx, expr.intro.complex.choice.left);
                dy_core_expr_release_ptr(ctx, expr.intro.complex.choice.right);
                return;
            case DY_CORE_COMPLEX_RECURSION:
                dy_core_expr_release_ptr(ctx, expr.intro.complex.recursion.expr);
                return;
            }

            dy_bail("impossible");
        case DY_CORE_INTRO_SIMPLE:
            dy_core_simple_release(ctx, expr.intro.simple);
            return;
        }

        dy_bail("impossible");
    case DY_CORE_EXPR_ELIM:
        dy_core_expr_release_ptr(ctx, expr.elim.expr);
        dy_core_simple_release(ctx, expr.elim.simple);
        return;
    case DY_CORE_EXPR_MAP:
        switch (expr.map.tag) {
        case DY_CORE_MAP_ASSUMPTION:
            dy_core_expr_release_ptr(ctx, expr.map.assumption.type);
            dy_core_assumption_release(ctx, expr.map.assumption.assumption);
            return;
        case DY_CORE_MAP_CHOICE:
            dy_core_assumption_release(ctx, expr.map.choice.assumption_left);
            dy_core_assumption_release(ctx, expr.map.choice.assumption_right);
            return;
        case DY_CORE_MAP_RECURSION:
            dy_core_assumption_release(ctx, expr.map.recursion.assumption);
            return;
        }

        dy_bail("impossible");
    case DY_CORE_EXPR_VARIABLE:
    case DY_CORE_EXPR_ANY:
    case DY_CORE_EXPR_VOID:
    case DY_CORE_EXPR_INFERENCE_VAR:
        return;
    case DY_CORE_EXPR_INFERENCE_CTX:
        dy_core_expr_release_ptr(ctx, expr.inference_ctx.expr);
        return;
    case DY_CORE_EXPR_CUSTOM: {
        const struct dy_core_custom_shared *s = dy_array_pos(&ctx->custom_shared, expr.custom.id);
        s->release(ctx, expr.custom.data);
        return;
    }
    }

    dy_bail("Impossible object type.");
}

void dy_core_expr_release_ptr(struct dy_core_ctx *ctx, struct dy_core_expr *expr)
{
    struct dy_core_expr e = *expr;
    if (dy_rc_release(expr, DY_ALIGNOF(struct dy_core_expr)) == 0) {
        dy_core_expr_release(ctx, e);
    }
}

void dy_core_assumption_release(struct dy_core_ctx *ctx, struct dy_core_assumption assumption)
{
    dy_core_expr_release_ptr(ctx, assumption.type);
    dy_core_expr_release_ptr(ctx, assumption.expr);
}

void dy_core_simple_release(struct dy_core_ctx *ctx, struct dy_core_simple simple)
{
    if (simple.tag == DY_CORE_SIMPLE_PROOF) {
        dy_core_expr_release_ptr(ctx, simple.proof);
    }

    dy_core_expr_release_ptr(ctx, simple.out);
}

enum dy_polarity dy_flip_polarity(enum dy_polarity polarity)
{
    switch (polarity) {
    case DY_POLARITY_POSITIVE:
        return DY_POLARITY_NEGATIVE;
    case DY_POLARITY_NEGATIVE:
        return DY_POLARITY_POSITIVE;
    }

    dy_bail("Impossible polarity.");
}

void dy_variable_appears_in_polarity(struct dy_core_ctx *ctx, struct dy_core_expr expr, size_t id, enum dy_polarity current_polarity, bool *positive, bool *negative)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_INTRO:
        switch (expr.intro.tag) {
        case DY_CORE_INTRO_COMPLEX:
            switch (expr.intro.complex.tag) {
            case DY_CORE_COMPLEX_ASSUMPTION:
                dy_variable_appears_in_polarity(ctx, *expr.intro.complex.assumption.type, id, expr.intro.polarity == DY_POLARITY_POSITIVE ? dy_flip_polarity(current_polarity) : current_polarity, positive, negative);
                if (*positive && *negative) {
                    return;
                }
                dy_variable_appears_in_polarity(ctx, *expr.intro.complex.assumption.expr, id, current_polarity, positive, negative);
                return;
            case DY_CORE_COMPLEX_CHOICE:
                dy_variable_appears_in_polarity(ctx, *expr.intro.complex.choice.left, id, current_polarity, positive, negative);
                if (*positive && *negative) {
                    return;
                }
                dy_variable_appears_in_polarity(ctx, *expr.intro.complex.choice.right, id, current_polarity, positive, negative);
                return;
            case DY_CORE_COMPLEX_RECURSION:
                dy_variable_appears_in_polarity(ctx, *expr.intro.complex.recursion.expr, id, current_polarity, positive, negative);
                return;
            }

            dy_bail("impossible");
        case DY_CORE_INTRO_SIMPLE:
            dy_variable_appears_in_polarity(ctx, *expr.intro.simple.out, id, current_polarity, positive, negative);
            return;
        }

        dy_bail("Impossible");
    case DY_CORE_EXPR_ELIM:
    case DY_CORE_EXPR_MAP:
        return;
    case DY_CORE_EXPR_VARIABLE:
        if (expr.variable_id == id) {
            if (current_polarity == DY_POLARITY_POSITIVE) {
                *positive = true;
            } else {
                *negative = true;
            }
        }

        return;
    case DY_CORE_EXPR_INFERENCE_VAR:
        if (expr.inference_var_id == id) {
            if (current_polarity == DY_POLARITY_POSITIVE) {
                *positive = true;
            } else {
                *negative = true;
            }
        }

        return;
    case DY_CORE_EXPR_ANY:
    case DY_CORE_EXPR_VOID:
        return;
    case DY_CORE_EXPR_INFERENCE_CTX:
        dy_bail("impossible");
    case DY_CORE_EXPR_CUSTOM: {
        const struct dy_core_custom_shared *vtab = dy_array_pos(&ctx->custom_shared, expr.custom.id);
        vtab->variable_appears_in_polarity(ctx, expr.custom.data, id, current_polarity, positive, negative);
        return;
    }
    }

    dy_bail("Impossible");
}

bool dy_core_expr_contains_this_variable(struct dy_core_ctx *ctx, size_t id, struct dy_core_expr expr)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_INTRO:
        switch (expr.intro.tag) {
        case DY_CORE_INTRO_COMPLEX:
            switch (expr.intro.complex.tag) {
            case DY_CORE_COMPLEX_ASSUMPTION:
                return dy_core_assumption_contains_this_variable(ctx, id, expr.intro.complex.assumption);
            case DY_CORE_COMPLEX_CHOICE:
                return dy_core_expr_contains_this_variable(ctx, id, *expr.intro.complex.choice.left)
                    || dy_core_expr_contains_this_variable(ctx, id, *expr.intro.complex.choice.right);
            case DY_CORE_COMPLEX_RECURSION:
                if (expr.intro.complex.recursion.id == id) {
                    return false;
                }

                return dy_core_expr_contains_this_variable(ctx, id, *expr.intro.complex.recursion.expr);
            }

            dy_bail("impossible");
        case DY_CORE_INTRO_SIMPLE:
            if (expr.intro.simple.tag == DY_CORE_SIMPLE_PROOF && dy_core_expr_contains_this_variable(ctx, id, *expr.intro.simple.proof)) {
                return true;
            }

            return dy_core_expr_contains_this_variable(ctx, id, *expr.intro.simple.out);
        }

        dy_bail("Impossible object type.");
    case DY_CORE_EXPR_ELIM:
        if (dy_core_expr_contains_this_variable(ctx, id, *expr.elim.expr)) {
            return true;
        }

        if (expr.elim.simple.tag == DY_CORE_SIMPLE_PROOF && dy_core_expr_contains_this_variable(ctx, id, *expr.elim.simple.proof)) {
            return true;
        }

        return dy_core_expr_contains_this_variable(ctx, id, *expr.elim.simple.out);
    case DY_CORE_EXPR_MAP:
        switch (expr.map.tag) {
        case DY_CORE_MAP_ASSUMPTION:
            if (dy_core_expr_contains_this_variable(ctx, id, *expr.map.assumption.type)) {
                return true;
            }

            if (expr.map.assumption.id == id) {
                return false;
            }

            return dy_core_assumption_contains_this_variable(ctx, id, expr.map.assumption.assumption);
        case DY_CORE_MAP_CHOICE:
            return dy_core_assumption_contains_this_variable(ctx, id, expr.map.choice.assumption_left)
                || dy_core_assumption_contains_this_variable(ctx, id, expr.map.choice.assumption_right);
        case DY_CORE_MAP_RECURSION:
            if (expr.map.recursion.id == id) {
                return false;
            }

            return dy_core_assumption_contains_this_variable(ctx, id, expr.map.recursion.assumption);
        }

        dy_bail("impossible");
    case DY_CORE_EXPR_VARIABLE:
        return expr.variable_id == id;
    case DY_CORE_EXPR_INFERENCE_VAR:
        return expr.inference_var_id == id;
    case DY_CORE_EXPR_ANY:
    case DY_CORE_EXPR_VOID:
        return false;
    case DY_CORE_EXPR_INFERENCE_CTX:
        if (id == expr.inference_ctx.id) {
            return false;
        }

        return dy_core_expr_contains_this_variable(ctx, id, *expr.inference_ctx.expr);
    case DY_CORE_EXPR_CUSTOM: {
        const struct dy_core_custom_shared *vtab = dy_array_pos(&ctx->custom_shared, expr.custom.id);
        return vtab->contains_this_variable(ctx, expr.custom.data, id);
    }
    }

    dy_bail("Impossible object type.");
}

bool dy_core_assumption_contains_this_variable(struct dy_core_ctx *ctx, size_t id, struct dy_core_assumption assumption)
{
    if (dy_core_expr_contains_this_variable(ctx, id, *assumption.type)) {
        return true;
    }

    if (assumption.id == id) {
        return false;
    }

    return dy_core_expr_contains_this_variable(ctx, id, *assumption.expr);
}

void dy_core_expr_to_string(struct dy_core_ctx *ctx, struct dy_core_expr expr, dy_array_t *string)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_INTRO:
        switch (expr.intro.tag) {
        case DY_CORE_INTRO_COMPLEX:
            switch (expr.intro.complex.tag) {
            case DY_CORE_COMPLEX_ASSUMPTION:
                if (expr.intro.polarity == DY_POLARITY_POSITIVE) {
                    add_string(string, DY_STR_LIT("fun "));
                } else {
                    add_string(string, DY_STR_LIT("some "));
                }

                if (expr.intro.is_implicit) {
                    add_string(string, DY_STR_LIT("@ "));
                }

                if (dy_core_expr_contains_this_variable(ctx, expr.intro.complex.assumption.id, *expr.intro.complex.assumption.expr)) {
                    add_size_t_decimal(string, expr.intro.complex.assumption.id);
                } else {
                    add_string(string, DY_STR_LIT("_"));
                }

                add_string(string, DY_STR_LIT(" : "));

                dy_core_expr_to_string(ctx, *expr.intro.complex.assumption.type, string);

                add_string(string, DY_STR_LIT(" => "));

                dy_core_expr_to_string(ctx, *expr.intro.complex.assumption.expr, string);
                return;
            case DY_CORE_COMPLEX_CHOICE:
                if (expr.intro.polarity == DY_POLARITY_POSITIVE) {
                    add_string(string, DY_STR_LIT("list "));
                } else {
                    add_string(string, DY_STR_LIT("either "));
                }

                if (expr.intro.is_implicit) {
                    add_string(string, DY_STR_LIT("@ "));
                }

                add_string(string, DY_STR_LIT("{ "));

                dy_core_expr_to_string(ctx, *expr.intro.complex.choice.left, string);

                add_string(string, DY_STR_LIT(", "));

                dy_core_expr_to_string(ctx, *expr.intro.complex.choice.right, string);

                add_string(string, DY_STR_LIT(" }"));
                return;
            case DY_CORE_COMPLEX_RECURSION:
                if (expr.intro.polarity == DY_POLARITY_POSITIVE) {
                    add_string(string, DY_STR_LIT("inf "));
                } else {
                    add_string(string, DY_STR_LIT("fin "));
                }

                if (expr.intro.is_implicit) {
                    add_string(string, DY_STR_LIT("@ "));
                }

                if (dy_core_expr_contains_this_variable(ctx, expr.intro.complex.recursion.id, *expr.intro.complex.recursion.expr)) {
                    add_size_t_decimal(string, expr.intro.complex.recursion.id);
                } else {
                    add_string(string, DY_STR_LIT("_"));
                }

                add_string(string, DY_STR_LIT(" = "));

                dy_core_expr_to_string(ctx, *expr.intro.complex.recursion.expr, string);
                return;
            }

            dy_bail("impossible");
        case DY_CORE_INTRO_SIMPLE:
            switch (expr.intro.simple.tag) {
            case DY_CORE_SIMPLE_PROOF:
                add_string(string, DY_STR_LIT("("));
                dy_core_expr_to_string(ctx, *expr.intro.simple.proof, string);
                add_string(string, DY_STR_LIT(")"));
                break;
            case DY_CORE_SIMPLE_DECISION:
                if (expr.intro.simple.direction == DY_LEFT) {
                    add_string(string, DY_STR_LIT("L"));
                } else {
                    add_string(string, DY_STR_LIT("R"));
                }
                break;
            case DY_CORE_SIMPLE_UNFOLD:
                add_string(string, DY_STR_LIT("Unfold"));
                break;
            case DY_CORE_SIMPLE_UNWRAP:
                add_string(string, DY_STR_LIT("Unwrap"));
                break;
            }

            if (expr.intro.polarity == DY_POLARITY_POSITIVE) {
                if (expr.intro.is_implicit) {
                    add_string(string, DY_STR_LIT(" @-> "));
                } else {
                    add_string(string, DY_STR_LIT(" -> "));
                }
            } else {
                if (expr.intro.is_implicit) {
                    add_string(string, DY_STR_LIT(" @~> "));
                } else {
                    add_string(string, DY_STR_LIT(" ~> "));
                }
            }

            dy_core_expr_to_string(ctx, *expr.intro.simple.out, string);
            return;
        }

        dy_bail("impossible");
    case DY_CORE_EXPR_ELIM:
        add_string(string, DY_STR_LIT("("));
        dy_core_expr_to_string(ctx, *expr.elim.expr, string);
        add_string(string, DY_STR_LIT(")"));

        if (expr.elim.is_implicit) {
            add_string(string, DY_STR_LIT(" @ "));
        } else {
            add_string(string, DY_STR_LIT(" "));
        }

        switch (expr.elim.simple.tag) {
        case DY_CORE_SIMPLE_PROOF:
            add_string(string, DY_STR_LIT("("));
            dy_core_expr_to_string(ctx, *expr.elim.simple.proof, string);
            add_string(string, DY_STR_LIT(")"));
            break;
        case DY_CORE_SIMPLE_DECISION:
            if (expr.elim.simple.direction == DY_LEFT) {
                add_string(string, DY_STR_LIT("L"));
            } else {
                add_string(string, DY_STR_LIT("R"));
            }
            break;
        case DY_CORE_SIMPLE_UNFOLD:
            add_string(string, DY_STR_LIT("Unfold"));
            break;
        case DY_CORE_SIMPLE_UNWRAP:
            add_string(string, DY_STR_LIT("Unwrap"));
            break;
        }

        add_string(string, DY_STR_LIT(" : "));

        if (expr.elim.eval_immediately) {
            add_string(string, DY_STR_LIT("$$$ "));
        }

        if (expr.elim.check_result == DY_NO) {
            add_string(string, DY_STR_LIT("FAIL "));
        } else if (expr.elim.check_result == DY_MAYBE) {
            add_string(string, DY_STR_LIT("MAYBE "));
        }

        dy_core_expr_to_string(ctx, *expr.elim.simple.out, string);
        return;
    case DY_CORE_EXPR_MAP:
        switch (expr.map.tag) {
        case DY_CORE_MAP_ASSUMPTION:
            add_string(string, DY_STR_LIT("map some "));

            if (expr.map.is_implicit) {
                add_string(string, DY_STR_LIT("@ "));
            }

            add_size_t_decimal(string, expr.map.assumption.id);

            add_string(string, DY_STR_LIT(" : "));

            dy_core_expr_to_string(ctx, *expr.map.assumption.type, string);

            add_string(string, DY_STR_LIT(" => "));

            add_size_t_decimal(string, expr.map.assumption.assumption.id);

            add_string(string, DY_STR_LIT(" : "));

            dy_core_expr_to_string(ctx, *expr.map.assumption.assumption.type, string);

            add_string(string, DY_STR_LIT(" => "));

            dy_core_expr_to_string(ctx, *expr.map.assumption.assumption.expr, string);

            return;
        case DY_CORE_MAP_CHOICE:
            add_string(string, DY_STR_LIT("map either "));

            if (expr.map.is_implicit) {
                add_string(string, DY_STR_LIT("@ "));
            }

            add_string(string, DY_STR_LIT("{ "));

            add_size_t_decimal(string, expr.map.choice.assumption_left.id);

            add_string(string, DY_STR_LIT(" : "));

            dy_core_expr_to_string(ctx, *expr.map.choice.assumption_left.type, string);

            add_string(string, DY_STR_LIT(" => "));

            dy_core_expr_to_string(ctx, *expr.map.choice.assumption_left.expr, string);

            add_string(string, DY_STR_LIT(", "));

            add_size_t_decimal(string, expr.map.choice.assumption_right.id);

            add_string(string, DY_STR_LIT(" : "));

            dy_core_expr_to_string(ctx, *expr.map.choice.assumption_right.type, string);

            add_string(string, DY_STR_LIT(" => "));

            dy_core_expr_to_string(ctx, *expr.map.choice.assumption_right.expr, string);

            add_string(string, DY_STR_LIT(" }"));

            return;
        case DY_CORE_MAP_RECURSION:
            add_string(string, DY_STR_LIT("map fin "));

            if (expr.map.is_implicit) {
                add_string(string, DY_STR_LIT("@ "));
            }

            add_size_t_decimal(string, expr.map.recursion.id);

            add_string(string, DY_STR_LIT(" = "));

            add_size_t_decimal(string, expr.map.recursion.assumption.id);

            add_string(string, DY_STR_LIT(" : "));

            dy_core_expr_to_string(ctx, *expr.map.recursion.assumption.type, string);

            add_string(string, DY_STR_LIT(" => "));

            dy_core_expr_to_string(ctx, *expr.map.recursion.assumption.expr, string);

            return;
        }

        dy_bail("impossible");
    case DY_CORE_EXPR_VARIABLE:
        add_size_t_decimal(string, expr.variable_id);
        return;
    case DY_CORE_EXPR_ANY:
        add_string(string, DY_STR_LIT("Any"));
        return;
    case DY_CORE_EXPR_VOID:
        add_string(string, DY_STR_LIT("Void"));
        return;
    case DY_CORE_EXPR_INFERENCE_CTX:
        add_string(string, DY_STR_LIT("[INFER "));

        add_size_t_decimal(string, expr.inference_ctx.id);

        if (expr.inference_ctx.polarity == DY_POLARITY_POSITIVE) {
            add_string(string, DY_STR_LIT("+"));
        } else {
            add_string(string, DY_STR_LIT("-"));
        }

        add_string(string, DY_STR_LIT("] "));

        dy_core_expr_to_string(ctx, *expr.inference_ctx.expr, string);

        return;
    case DY_CORE_EXPR_INFERENCE_VAR:
        add_string(string, DY_STR_LIT("?"));

        add_size_t_decimal(string, expr.inference_var_id);

        return;
    case DY_CORE_EXPR_CUSTOM: {
        const struct dy_core_custom_shared *s = dy_array_pos(&ctx->custom_shared, expr.custom.id);
        s->to_string(ctx, expr.custom.data, string);
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
        char c = *(char *)dy_array_pos(string, i);
        *(char *)dy_array_pos(string, i) = *(char *)dy_array_pos(string, k);
        *(char *)dy_array_pos(string, k) = c;
    }
}
