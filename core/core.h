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

struct dy_core_function {
    size_t id;
    struct dy_core_expr *type;
    struct dy_core_expr *expr;
};

struct dy_core_pair {
    struct dy_core_expr *left;
    struct dy_core_expr *right;
};

struct dy_core_recursion {
    size_t id;
    struct dy_core_expr *expr;
};

enum dy_core_tag {
    DY_CORE_FUNCTION,
    DY_CORE_PAIR,
    DY_CORE_RECURSION
};

enum dy_polarity {
    DY_POLARITY_POSITIVE,
    DY_POLARITY_NEGATIVE
};

struct dy_core_problem {
    union {
        struct dy_core_function function;
        struct dy_core_pair pair;
        struct dy_core_recursion recursion;
    };

    enum dy_core_tag tag;

    enum dy_polarity polarity;
    bool is_implicit;
};

enum dy_direction {
    DY_LEFT,
    DY_RIGHT
};

struct dy_core_solution {
    union {
        struct dy_core_expr *expr;
        enum dy_direction direction;
    };

    enum dy_core_tag tag;

    struct dy_core_expr *out;

    bool is_implicit;
};

struct dy_core_application {
    struct dy_core_expr *expr;
    struct dy_core_solution solution;
    dy_ternary_t check_result;
    bool eval_immediately;
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
    DY_CORE_EXPR_PROBLEM,
    DY_CORE_EXPR_SOLUTION,
    DY_CORE_EXPR_APPLICATION,
    DY_CORE_EXPR_VARIABLE,
    DY_CORE_EXPR_ANY,
    DY_CORE_EXPR_VOID,
    DY_CORE_EXPR_INFERENCE_CTX,
    DY_CORE_EXPR_INFERENCE_VAR,
    DY_CORE_EXPR_CUSTOM
};

struct dy_core_expr {
    union {
        struct dy_core_problem problem;
        struct dy_core_solution solution;
        struct dy_core_application application;
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
static inline struct dy_core_solution dy_core_solution_retain(struct dy_core_ctx *ctx, struct dy_core_solution solution);

static inline void dy_core_expr_release(struct dy_core_ctx *ctx, struct dy_core_expr expr);
static inline void dy_core_expr_release_ptr(struct dy_core_ctx *ctx, struct dy_core_expr *expr);
static inline void dy_core_solution_release(struct dy_core_ctx *ctx, struct dy_core_solution solution);

static inline enum dy_polarity dy_flip_polarity(enum dy_polarity polarity);

static inline void dy_variable_appears_in_polarity(struct dy_core_ctx *ctx, struct dy_core_expr expr, size_t id, enum dy_polarity current_polarity, bool *positive, bool *negative);

/** Returns whether 'id' occurs in expr at all. */
static inline bool dy_core_expr_contains_this_variable(struct dy_core_ctx *ctx, size_t id, struct dy_core_expr expr);

/** Appends a utf8 represention of expr to 'string'. */
static inline void dy_core_expr_to_string(struct dy_core_ctx *ctx, struct dy_core_expr expr, dy_array_t *string);

static inline void dy_core_solution_to_string(struct dy_core_ctx *ctx, struct dy_core_solution solution, dy_array_t *string);

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
    case DY_CORE_EXPR_PROBLEM:
        switch (expr.problem.tag) {
        case DY_CORE_FUNCTION:
            dy_core_expr_retain_ptr(ctx, expr.problem.function.type);
            dy_core_expr_retain_ptr(ctx, expr.problem.function.expr);
            return expr;
        case DY_CORE_PAIR:
            dy_core_expr_retain_ptr(ctx, expr.problem.pair.left);
            dy_core_expr_retain_ptr(ctx, expr.problem.pair.right);
            return expr;
        case DY_CORE_RECURSION:
            dy_core_expr_retain_ptr(ctx, expr.problem.recursion.expr);
            return expr;
        }

        dy_bail("impossible");
    case DY_CORE_EXPR_SOLUTION:
        dy_core_solution_retain(ctx, expr.solution);
        return expr;
    case DY_CORE_EXPR_APPLICATION:
        dy_core_expr_retain_ptr(ctx, expr.application.expr);
        dy_core_solution_retain(ctx, expr.application.solution);
        return expr;
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

struct dy_core_solution dy_core_solution_retain(struct dy_core_ctx *ctx, struct dy_core_solution solution)
{
    if (solution.tag == DY_CORE_FUNCTION) {
        dy_core_expr_retain_ptr(ctx, solution.expr);
    }

    dy_core_expr_retain_ptr(ctx, solution.out);

    return solution;
}

void dy_core_expr_release(struct dy_core_ctx *ctx, struct dy_core_expr expr)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_PROBLEM:
        switch (expr.problem.tag) {
        case DY_CORE_FUNCTION:
            dy_core_expr_release_ptr(ctx, expr.problem.function.type);
            dy_core_expr_release_ptr(ctx, expr.problem.function.expr);
            return;
        case DY_CORE_PAIR:
            dy_core_expr_release_ptr(ctx, expr.problem.pair.left);
            dy_core_expr_release_ptr(ctx, expr.problem.pair.right);
            return;
        case DY_CORE_RECURSION:
            dy_core_expr_release_ptr(ctx, expr.problem.recursion.expr);
            return;
        }

        dy_bail("impossible");
    case DY_CORE_EXPR_SOLUTION:
        dy_core_solution_release(ctx, expr.solution);
        return;
    case DY_CORE_EXPR_APPLICATION:
        dy_core_expr_release_ptr(ctx, expr.application.expr);
        dy_core_solution_release(ctx, expr.application.solution);
        return;
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

void dy_core_solution_release(struct dy_core_ctx *ctx, struct dy_core_solution solution)
{
    if (solution.tag == DY_CORE_FUNCTION) {
        dy_core_expr_release_ptr(ctx, solution.expr);
    }
    dy_core_expr_release_ptr(ctx, solution.out);
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
    case DY_CORE_EXPR_PROBLEM:
        switch (expr.problem.tag) {
        case DY_CORE_FUNCTION:
            dy_variable_appears_in_polarity(ctx, *expr.problem.function.type, id, dy_flip_polarity(current_polarity), positive, negative);
            if (*positive && *negative) {
                return;
            }
            dy_variable_appears_in_polarity(ctx, *expr.problem.function.expr, id, current_polarity, positive, negative);
            return;
        case DY_CORE_PAIR:
            dy_variable_appears_in_polarity(ctx, *expr.problem.pair.left, id, current_polarity, positive, negative);
            if (*positive && *negative) {
                return;
            }
            dy_variable_appears_in_polarity(ctx, *expr.problem.pair.right, id, current_polarity, positive, negative);
            return;
        case DY_CORE_RECURSION:
            dy_variable_appears_in_polarity(ctx, *expr.problem.recursion.expr, id, current_polarity, positive, negative);
            return;
        }

        dy_bail("Impossible");
    case DY_CORE_EXPR_SOLUTION:
        dy_variable_appears_in_polarity(ctx, *expr.solution.out, id, current_polarity, positive, negative);
        return;
    case DY_CORE_EXPR_APPLICATION:
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
    case DY_CORE_EXPR_PROBLEM:
        switch (expr.problem.tag) {
        case DY_CORE_FUNCTION:
            if (dy_core_expr_contains_this_variable(ctx, id, *expr.problem.function.type)) {
                return true;
            }

            if (expr.problem.function.id == id) {
                return false;
            }

            return dy_core_expr_contains_this_variable(ctx, id, *expr.problem.function.expr);
        case DY_CORE_PAIR:
            return dy_core_expr_contains_this_variable(ctx, id, *expr.problem.pair.left)
                || dy_core_expr_contains_this_variable(ctx, id, *expr.problem.pair.right);
        case DY_CORE_RECURSION:
            if (expr.problem.recursion.id == id) {
                return false;
            }

            return dy_core_expr_contains_this_variable(ctx, id, *expr.problem.recursion.expr);
        }

        dy_bail("Impossible object type.");
    case DY_CORE_EXPR_SOLUTION:
        if (expr.solution.tag == DY_CORE_FUNCTION && dy_core_expr_contains_this_variable(ctx, id, *expr.solution.expr)) {
            return true;
        }

        return dy_core_expr_contains_this_variable(ctx, id, *expr.solution.out);
    case DY_CORE_EXPR_APPLICATION:
        if (dy_core_expr_contains_this_variable(ctx, id, *expr.application.expr)) {
            return true;
        }

        if (expr.application.solution.tag == DY_CORE_FUNCTION && dy_core_expr_contains_this_variable(ctx, id, *expr.application.solution.expr)) {
            return true;
        }

        return dy_core_expr_contains_this_variable(ctx, id, *expr.application.solution.out);
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

void dy_core_expr_to_string(struct dy_core_ctx *ctx, struct dy_core_expr expr, dy_array_t *string)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_PROBLEM:
        switch (expr.problem.tag) {
        case DY_CORE_FUNCTION:
            if (expr.problem.polarity == DY_POLARITY_POSITIVE) {
                add_string(string, DY_STR_LIT("fun "));
            } else {
                add_string(string, DY_STR_LIT("ex "));
            }

            if (expr.problem.is_implicit) {
                add_string(string, DY_STR_LIT("@ "));
            }

            if (dy_core_expr_contains_this_variable(ctx, expr.problem.function.id, *expr.problem.function.expr)) {
                add_size_t_decimal(string, expr.problem.function.id);
                add_string(string, DY_STR_LIT(" "));
            }

            add_string(string, DY_STR_LIT(": "));

            dy_core_expr_to_string(ctx, *expr.problem.function.type, string);

            add_string(string, DY_STR_LIT(" => "));

            dy_core_expr_to_string(ctx, *expr.problem.function.expr, string);
            return;
        case DY_CORE_PAIR:
            if (expr.problem.polarity == DY_POLARITY_POSITIVE) {
                add_string(string, DY_STR_LIT("list "));
            } else {
                add_string(string, DY_STR_LIT("union "));
            }

            if (expr.problem.is_implicit) {
                add_string(string, DY_STR_LIT("@ "));
            }

            add_string(string, DY_STR_LIT("{ "));

            dy_core_expr_to_string(ctx, *expr.problem.pair.left, string);

            add_string(string, DY_STR_LIT(", "));

            dy_core_expr_to_string(ctx, *expr.problem.pair.right, string);

            add_string(string, DY_STR_LIT(" }"));
            return;
        case DY_CORE_RECURSION:
            if (expr.problem.polarity == DY_POLARITY_POSITIVE) {
                add_string(string, DY_STR_LIT("inf "));
            } else {
                add_string(string, DY_STR_LIT("fin "));
            }

            if (expr.problem.is_implicit) {
                add_string(string, DY_STR_LIT("@ "));
            }

            add_size_t_decimal(string, expr.problem.recursion.id);

            add_string(string, DY_STR_LIT(" = "));

            dy_core_expr_to_string(ctx, *expr.problem.recursion.expr, string);
            return;
        }

        dy_bail("impossible");
    case DY_CORE_EXPR_SOLUTION:
        dy_core_solution_to_string(ctx, expr.solution, string);
        return;
    case DY_CORE_EXPR_APPLICATION:
        add_string(string, DY_STR_LIT("("));
        dy_core_expr_to_string(ctx, *expr.application.expr, string);
        add_string(string, DY_STR_LIT(")"));

        if (expr.application.solution.is_implicit) {
            add_string(string, DY_STR_LIT(" @ "));
        } else {
            add_string(string, DY_STR_LIT(" "));
        }

        switch (expr.application.solution.tag) {
        case DY_CORE_FUNCTION:
            add_string(string, DY_STR_LIT("("));
            dy_core_expr_to_string(ctx, *expr.application.solution.expr, string);
            add_string(string, DY_STR_LIT(")"));
            break;
        case DY_CORE_PAIR:
            if (expr.application.solution.direction == DY_LEFT) {
                add_string(string, DY_STR_LIT("L"));
            } else {
                add_string(string, DY_STR_LIT("R"));
            }
            break;
        case DY_CORE_RECURSION:
            add_string(string, DY_STR_LIT("!"));
            break;
        }

        add_string(string, DY_STR_LIT(" : "));

        if (expr.application.eval_immediately) {
            add_string(string, DY_STR_LIT("$$$ "));
        }

        if (expr.application.check_result == DY_NO) {
            add_string(string, DY_STR_LIT("FAIL "));
        } else if (expr.application.check_result == DY_MAYBE) {
            add_string(string, DY_STR_LIT("MAYBE "));
        }

        dy_core_expr_to_string(ctx, *expr.application.solution.out, string);
        return;
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

void dy_core_solution_to_string(struct dy_core_ctx *ctx, struct dy_core_solution solution, dy_array_t *string)
{
    switch (solution.tag) {
    case DY_CORE_FUNCTION:
        add_string(string, DY_STR_LIT("("));
        dy_core_expr_to_string(ctx, *solution.expr, string);
        add_string(string, DY_STR_LIT(")"));
        break;
    case DY_CORE_PAIR:
        if (solution.direction == DY_LEFT) {
            add_string(string, DY_STR_LIT("L"));
        } else {
            add_string(string, DY_STR_LIT("R"));
        }
        break;
    case DY_CORE_RECURSION:
        add_string(string, DY_STR_LIT("!"));
        break;
    }

    if (solution.is_implicit) {
        add_string(string, DY_STR_LIT(" @-> "));
    } else {
        add_string(string, DY_STR_LIT(" -> "));
    }

    dy_core_expr_to_string(ctx, *solution.out, string);
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
