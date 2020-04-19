/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_CORE_H
#define DY_CORE_H

#include "../support/string.h"
#include "../support/array.h"
#include "../support/rc.h"
#include "../support/range.h"

typedef enum dy_ternary {
    DY_YES,
    DY_NO,
    DY_MAYBE
} dy_ternary_t;

struct dy_core_expr;

enum dy_core_polarity {
    DY_CORE_POLARITY_POSITIVE,
    DY_CORE_POLARITY_NEGATIVE
};

struct dy_core_variable {
    size_t id;
    const struct dy_core_expr *type;
};

struct dy_core_expr_map {
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

struct dy_core_expr_map_elim {
    struct dy_range text_range;
    bool has_text_range;

    const struct dy_core_expr *expr;
    struct dy_core_expr_map map;
    dy_ternary_t check_result;
};

struct dy_core_type_map_elim {
    struct dy_range text_range;
    bool has_text_range;

    const struct dy_core_expr *expr;
    struct dy_core_type_map map;
    dy_ternary_t check_result;
};

struct dy_core_both {
    const struct dy_core_expr *e1;
    const struct dy_core_expr *e2;
    enum dy_core_polarity polarity;
};

struct dy_core_one_of {
    const struct dy_core_expr *first;
    const struct dy_core_expr *second;
};

struct dy_core_expr_invalid {
    struct dy_range text_range;
    bool has_text_range;
};

struct dy_core_recursion {
    struct dy_core_type_map map;
    dy_ternary_t check_result;
};

enum dy_core_expr_tag {
    DY_CORE_EXPR_EXPR_MAP,
    DY_CORE_EXPR_TYPE_MAP,
    DY_CORE_EXPR_EXPR_MAP_ELIM,
    DY_CORE_EXPR_TYPE_MAP_ELIM,
    DY_CORE_EXPR_BOTH,
    DY_CORE_EXPR_ONE_OF,
    DY_CORE_EXPR_VARIABLE,
    DY_CORE_EXPR_END,
    DY_CORE_EXPR_RECURSION,
    DY_CORE_EXPR_INFERENCE_VARIABLE,
    DY_CORE_EXPR_INFERENCE_TYPE_MAP,
    DY_CORE_EXPR_INVALID,
    DY_CORE_EXPR_STRING,
    DY_CORE_EXPR_TYPE_OF_STRINGS,
    DY_CORE_EXPR_PRINT
};

struct dy_core_expr {
    union {
        struct dy_core_expr_map expr_map;
        struct dy_core_type_map type_map;
        struct dy_core_expr_map_elim expr_map_elim;
        struct dy_core_type_map_elim type_map_elim;
        struct dy_core_both both;
        struct dy_core_one_of one_of;
        struct dy_core_variable variable;
        struct dy_core_variable inference_variable;
        struct dy_core_type_map inference_type_map;
        struct dy_core_recursion recursion;
        dy_string_t string;
        enum dy_core_polarity end_polarity;
        struct dy_core_expr_invalid invalid;
    };

    enum dy_core_expr_tag tag;
};


static inline bool dy_core_expr_is_computation(struct dy_core_expr expr);

static inline size_t dy_core_expr_num_ocurrences(size_t id, struct dy_core_expr expr);

static inline bool dy_core_expr_is_bound(size_t id, struct dy_core_expr expr);

static inline void dy_core_expr_to_string(struct dy_core_expr expr, dy_array_t *string);


static const size_t dy_core_expr_rc_offset = DY_RC_OFFSET_OF_TYPE(struct dy_core_expr);

static inline const struct dy_core_expr *dy_core_expr_new(struct dy_core_expr expr);

static inline struct dy_core_expr dy_core_expr_retain(struct dy_core_expr expr);

static inline const struct dy_core_expr *dy_core_expr_retain_ptr(const struct dy_core_expr *expr);

static inline void dy_core_expr_release(struct dy_core_expr expr);

static inline void dy_core_expr_release_ptr(const struct dy_core_expr *expr);


static inline void add_string(dy_array_t *string, dy_string_t s);

#ifdef _WIN32
static inline int asprintf(char **ret, const char *format, ...);
#endif

bool dy_core_expr_is_computation(struct dy_core_expr expr)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_EXPR_MAP:
        return dy_core_expr_is_computation(*expr.expr_map.e1);
    case DY_CORE_EXPR_TYPE_MAP:
        return dy_core_expr_is_computation(*expr.type_map.binding.type);
    case DY_CORE_EXPR_EXPR_MAP_ELIM:
        return true;
    case DY_CORE_EXPR_TYPE_MAP_ELIM:
        return true;
    case DY_CORE_EXPR_BOTH:
        return dy_core_expr_is_computation(*expr.both.e1) || dy_core_expr_is_computation(*expr.both.e2);
    case DY_CORE_EXPR_ONE_OF:
        return dy_core_expr_is_computation(*expr.one_of.first) || dy_core_expr_is_computation(*expr.one_of.second);
    case DY_CORE_EXPR_VARIABLE:
        return false;
    case DY_CORE_EXPR_INFERENCE_VARIABLE:
        return false;
    case DY_CORE_EXPR_INFERENCE_TYPE_MAP:
        dy_bail("should never be reached");
    case DY_CORE_EXPR_RECURSION:
        return dy_core_expr_is_computation(*expr.recursion.map.binding.type) || dy_core_expr_is_computation(*expr.recursion.map.expr);
    case DY_CORE_EXPR_STRING:
        // fallthrough
    case DY_CORE_EXPR_TYPE_OF_STRINGS:
        // fallthrough
    case DY_CORE_EXPR_END:
        // fallthrough
    case DY_CORE_EXPR_PRINT:
        // fallthrough
    case DY_CORE_EXPR_INVALID:
        return false;
    }

    DY_IMPOSSIBLE_ENUM();
}

bool dy_core_expr_is_bound(size_t id, struct dy_core_expr expr)
{
    // TODO: Optimize by not using num_ocurrences (can stop traversing after finding one instance).
    return dy_core_expr_num_ocurrences(id, expr) > 0;
}

size_t dy_core_expr_num_ocurrences(size_t id, struct dy_core_expr expr)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_EXPR_MAP:
        return dy_core_expr_num_ocurrences(id, *expr.expr_map.e1) + dy_core_expr_num_ocurrences(id, *expr.expr_map.e2);
    case DY_CORE_EXPR_TYPE_MAP: {
        size_t x = dy_core_expr_num_ocurrences(id, *expr.type_map.binding.type);

        if (id == expr.type_map.binding.id) {
            return x;
        }

        return x + dy_core_expr_num_ocurrences(id, *expr.type_map.expr);
    }
    case DY_CORE_EXPR_EXPR_MAP_ELIM:
        return dy_core_expr_num_ocurrences(id, *expr.expr_map_elim.expr) + dy_core_expr_num_ocurrences(id, *expr.expr_map_elim.map.e1) + dy_core_expr_num_ocurrences(id, *expr.expr_map_elim.map.e2);
    case DY_CORE_EXPR_TYPE_MAP_ELIM: {
        size_t x = dy_core_expr_num_ocurrences(id, *expr.type_map_elim.expr) + dy_core_expr_num_ocurrences(id, *expr.type_map_elim.map.binding.type);

        if (id == expr.type_map_elim.map.binding.id) {
            return x;
        }

        return x + dy_core_expr_num_ocurrences(id, *expr.type_map_elim.map.expr);
    }
    case DY_CORE_EXPR_BOTH:
        return dy_core_expr_num_ocurrences(id, *expr.both.e1) + dy_core_expr_num_ocurrences(id, *expr.both.e2);
    case DY_CORE_EXPR_ONE_OF:
        return dy_core_expr_num_ocurrences(id, *expr.one_of.first) + dy_core_expr_num_ocurrences(id, *expr.one_of.second);
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
        size_t x = dy_core_expr_num_ocurrences(id, *expr.recursion.map.binding.type);

        if (id == expr.recursion.map.binding.id) {
            return x;
        }

        return x + dy_core_expr_num_ocurrences(id, *expr.recursion.map.expr);
    }
    case DY_CORE_EXPR_STRING:
        // fallthrough
    case DY_CORE_EXPR_TYPE_OF_STRINGS:
        // fallthrough
    case DY_CORE_EXPR_END:
        // fallthrough
    case DY_CORE_EXPR_PRINT:
        // fallthrough
    case DY_CORE_EXPR_INVALID:
        return 0;
    }

    DY_IMPOSSIBLE_ENUM();
}

const struct dy_core_expr *dy_core_expr_new(struct dy_core_expr expr)
{
    return dy_rc_new(&expr, sizeof expr, dy_core_expr_rc_offset);
}

const struct dy_core_expr *dy_core_expr_retain_ptr(const struct dy_core_expr *expr)
{
    return dy_rc_retain(expr, dy_core_expr_rc_offset);
}

struct dy_core_expr dy_core_expr_retain(struct dy_core_expr expr)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_EXPR_MAP:
        dy_core_expr_retain_ptr(expr.expr_map.e1);
        dy_core_expr_retain_ptr(expr.expr_map.e2);

        return expr;
    case DY_CORE_EXPR_TYPE_MAP:
        dy_core_expr_retain_ptr(expr.type_map.binding.type);
        dy_core_expr_retain_ptr(expr.type_map.expr);

        return expr;
    case DY_CORE_EXPR_EXPR_MAP_ELIM:
        dy_core_expr_retain_ptr(expr.expr_map_elim.expr);
        dy_core_expr_retain_ptr(expr.expr_map_elim.map.e1);
        dy_core_expr_retain_ptr(expr.expr_map_elim.map.e2);

        return expr;
    case DY_CORE_EXPR_TYPE_MAP_ELIM:
        dy_core_expr_retain_ptr(expr.type_map_elim.expr);
        dy_core_expr_retain_ptr(expr.type_map_elim.map.binding.type);
        dy_core_expr_retain_ptr(expr.type_map_elim.map.expr);

        return expr;
    case DY_CORE_EXPR_BOTH:
        dy_core_expr_retain_ptr(expr.both.e1);
        dy_core_expr_retain_ptr(expr.both.e2);

        return expr;
    case DY_CORE_EXPR_ONE_OF:
        dy_core_expr_retain_ptr(expr.one_of.first);
        dy_core_expr_retain_ptr(expr.one_of.second);

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
        dy_core_expr_retain_ptr(expr.recursion.map.binding.type);
        dy_core_expr_retain_ptr(expr.recursion.map.expr);

        return expr;
    case DY_CORE_EXPR_END:
        // fallthrough
    case DY_CORE_EXPR_STRING:
        // fallthrough
    case DY_CORE_EXPR_TYPE_OF_STRINGS:
        // fallthroughs
    case DY_CORE_EXPR_PRINT:
        // fallthrough
    case DY_CORE_EXPR_INVALID:
        return expr;
    }

    DY_IMPOSSIBLE_ENUM();
}

void dy_core_expr_release_ptr(const struct dy_core_expr *expr)
{
    struct dy_core_expr shallow_copy = *expr;
    if (dy_rc_release(expr, dy_core_expr_rc_offset) == 0) {
        dy_core_expr_release(shallow_copy);
    }
}

void dy_core_expr_release(struct dy_core_expr expr)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_EXPR_MAP:
        dy_core_expr_release_ptr(expr.expr_map.e1);
        dy_core_expr_release_ptr(expr.expr_map.e2);

        return;
    case DY_CORE_EXPR_TYPE_MAP:
        dy_core_expr_release_ptr(expr.type_map.binding.type);
        dy_core_expr_release_ptr(expr.type_map.expr);

        return;
    case DY_CORE_EXPR_EXPR_MAP_ELIM:
        dy_core_expr_release_ptr(expr.expr_map_elim.expr);
        dy_core_expr_release_ptr(expr.expr_map_elim.map.e1);
        dy_core_expr_release_ptr(expr.expr_map_elim.map.e2);

        return;
    case DY_CORE_EXPR_TYPE_MAP_ELIM:
        dy_core_expr_release_ptr(expr.type_map_elim.expr);
        dy_core_expr_release_ptr(expr.type_map_elim.map.binding.type);
        dy_core_expr_release_ptr(expr.type_map_elim.map.expr);

        return;
    case DY_CORE_EXPR_BOTH:
        dy_core_expr_release_ptr(expr.both.e1);
        dy_core_expr_release_ptr(expr.both.e2);

        return;
    case DY_CORE_EXPR_ONE_OF:
        dy_core_expr_release_ptr(expr.one_of.first);
        dy_core_expr_release_ptr(expr.one_of.second);

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
        dy_core_expr_release_ptr(expr.recursion.map.binding.type);
        dy_core_expr_release_ptr(expr.recursion.map.expr);

        return;
    case DY_CORE_EXPR_END:
        // fallthrough
    case DY_CORE_EXPR_STRING:
        // fallthrough
    case DY_CORE_EXPR_TYPE_OF_STRINGS:
        // fallthrough
    case DY_CORE_EXPR_PRINT:
        // fallthrough
    case DY_CORE_EXPR_INVALID:
        return;
    }

    DY_IMPOSSIBLE_ENUM();
}

void dy_core_expr_to_string(struct dy_core_expr expr, dy_array_t *string)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_EXPR_MAP:
        add_string(string, DY_STR_LIT("("));
        dy_core_expr_to_string(*expr.expr_map.e1, string);
        if (expr.expr_map.is_implicit) {
            add_string(string, DY_STR_LIT(" @"));
        } else {
            add_string(string, DY_STR_LIT(" "));
        }
        if (expr.expr_map.polarity == DY_CORE_POLARITY_POSITIVE) {
            add_string(string, DY_STR_LIT("-> "));
        } else {
            add_string(string, DY_STR_LIT("~> "));
        }
        dy_core_expr_to_string(*expr.expr_map.e2, string);
        add_string(string, DY_STR_LIT(")"));
        return;
    case DY_CORE_EXPR_TYPE_MAP: {
        add_string(string, DY_STR_LIT("("));

        add_string(string, DY_STR_LIT("["));
        char *c;
        dy_assert(asprintf(&c, "%zu", expr.type_map.binding.id) != -1);
        add_string(string, (dy_string_t){ .ptr = c, .size = strlen(c) });
        free(c);
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
    }
    case DY_CORE_EXPR_INFERENCE_TYPE_MAP: {
        add_string(string, DY_STR_LIT("("));

        add_string(string, DY_STR_LIT("?["));
        char *c;
        dy_assert(asprintf(&c, "%zu", expr.inference_type_map.binding.id) != -1);
        add_string(string, (dy_string_t){ .ptr = c, .size = strlen(c) });
        free(c);
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
    }
    case DY_CORE_EXPR_EXPR_MAP_ELIM: {
        add_string(string, DY_STR_LIT("("));
        dy_core_expr_to_string(*expr.expr_map_elim.expr, string);
        add_string(string, DY_STR_LIT(" ! "));
        dy_core_expr_to_string(*expr.expr_map_elim.map.e1, string);
        if (expr.expr_map_elim.map.is_implicit) {
            add_string(string, DY_STR_LIT(" @"));
        } else {
            add_string(string, DY_STR_LIT(" "));
        }
        add_string(string, DY_STR_LIT("~> "));
        dy_core_expr_to_string(*expr.expr_map_elim.map.e2, string);
        add_string(string, DY_STR_LIT(")"));
        return;
    }
    case DY_CORE_EXPR_TYPE_MAP_ELIM: {
        add_string(string, DY_STR_LIT("("));
        dy_core_expr_to_string(*expr.type_map_elim.expr, string);
        add_string(string, DY_STR_LIT(" ! "));

        char *c;
        dy_assert(asprintf(&c, "%zu [", expr.type_map_elim.map.binding.id) != -1);
        add_string(string, (dy_string_t){ .ptr = c, .size = strlen(c) });
        free(c);

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
    }
    case DY_CORE_EXPR_VARIABLE: {
        add_string(string, DY_STR_LIT("("));

        char *c;
        dy_assert(asprintf(&c, "%zu", expr.variable.id) != -1);
        add_string(string, (dy_string_t){ .ptr = c, .size = strlen(c) });
        free(c);

        add_string(string, DY_STR_LIT(" : "));
        dy_core_expr_to_string(*expr.variable.type, string);
        add_string(string, DY_STR_LIT(")"));
        return;
    }
    case DY_CORE_EXPR_INFERENCE_VARIABLE: {
        add_string(string, DY_STR_LIT("("));

        char *c;
        dy_assert(asprintf(&c, "?%zu", expr.inference_variable.id) != -1);
        add_string(string, (dy_string_t){ .ptr = c, .size = strlen(c) });
        free(c);

        add_string(string, DY_STR_LIT(" : "));
        dy_core_expr_to_string(*expr.variable.type, string);
        add_string(string, DY_STR_LIT(")"));
        return;
    }
    case DY_CORE_EXPR_END:
        if (expr.end_polarity == DY_CORE_POLARITY_POSITIVE) {
            add_string(string, DY_STR_LIT("All"));
        } else if (expr.end_polarity == DY_CORE_POLARITY_NEGATIVE) {
            add_string(string, DY_STR_LIT("Nothing"));
        } else {
            dy_bail("Invalid polarity!\n");
        }

        return;
    case DY_CORE_EXPR_BOTH:
        add_string(string, DY_STR_LIT("("));
        dy_core_expr_to_string(*expr.both.e1, string);

        if (expr.both.polarity == DY_CORE_POLARITY_POSITIVE) {
            add_string(string, DY_STR_LIT(" and "));
        } else {
            add_string(string, DY_STR_LIT(" or "));
        }

        dy_core_expr_to_string(*expr.both.e2, string);
        add_string(string, DY_STR_LIT(")"));
        return;
    case DY_CORE_EXPR_ONE_OF:
        add_string(string, DY_STR_LIT("("));
        dy_core_expr_to_string(*expr.one_of.first, string);
        add_string(string, DY_STR_LIT(" else "));
        dy_core_expr_to_string(*expr.one_of.second, string);
        add_string(string, DY_STR_LIT(")"));
        return;
    case DY_CORE_EXPR_RECURSION: {
        add_string(string, DY_STR_LIT("(rec "));

        add_string(string, DY_STR_LIT("["));
        char *c;
        dy_assert(asprintf(&c, "%zu", expr.recursion.map.binding.id) != -1);
        add_string(string, (dy_string_t){ .ptr = c, .size = strlen(c) });
        free(c);
        add_string(string, DY_STR_LIT(" "));
        dy_core_expr_to_string(*expr.recursion.map.binding.type, string);
        add_string(string, DY_STR_LIT("]"));
        if (expr.recursion.map.polarity == DY_CORE_POLARITY_POSITIVE) {
            add_string(string, DY_STR_LIT(" -> "));
        } else {
            add_string(string, DY_STR_LIT(" ~> "));
        }
        dy_core_expr_to_string(*expr.recursion.map.expr, string);
        add_string(string, DY_STR_LIT(")"));
        return;
    }
    case DY_CORE_EXPR_STRING:
        add_string(string, DY_STR_LIT("\""));
        add_string(string, expr.string);
        add_string(string, DY_STR_LIT("\""));
        return;
    case DY_CORE_EXPR_TYPE_OF_STRINGS:
        add_string(string, DY_STR_LIT("String"));
        return;
    case DY_CORE_EXPR_PRINT:
        add_string(string, DY_STR_LIT("print"));
        return;
    }

    DY_IMPOSSIBLE_ENUM();
}

void add_string(dy_array_t *string, dy_string_t s)
{
    for (size_t i = 0; i < s.size; ++i) {
        dy_array_add(string, s.ptr + i);
    }
}

#ifdef _WIN32
int asprintf(char **ret, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);

    int projected_len = _vscprintf(format, ap);
    dy_assert(projected_len >= 0);

    char *buffer = malloc((size_t)projected_len + 1 /* for '\0' */);
    dy_assert(buffer != NULL);

    int actual_len = vsprintf(buffer, format, ap);
    dy_assert(projected_len == actual_len);

    va_end(ap);

    *ret = buffer;

    return actual_len;
}
#endif

#endif // DY_CORE_H
