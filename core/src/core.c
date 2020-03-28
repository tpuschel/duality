/*
 * Copyright 2017-2020, Thorben Hasenpusch
 *
 * SPDX-License-Identifier: MIT
 */

#include <duality/core/core.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <duality/support/assert.h>

static void add_string(dy_array_t *string, dy_string_t s);

#ifdef _WIN32
static int asprintf(char **ret, const char *format, ...);
#endif

void dy_core_expr_to_string(struct dy_core_expr expr, dy_array_t *string)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_EXPR_MAP:
        add_string(string, DY_STR_LIT("("));
        if (expr.expr_map.is_implicit) {
            add_string(string, DY_STR_LIT("@"));
        }
        dy_core_expr_to_string(*expr.expr_map.e1, string);
        if (expr.expr_map.polarity == DY_CORE_POLARITY_POSITIVE) {
            add_string(string, DY_STR_LIT(" -> "));
        } else {
            add_string(string, DY_STR_LIT(" ~> "));
        }
        dy_core_expr_to_string(*expr.expr_map.e2, string);
        add_string(string, DY_STR_LIT(")"));
        return;
    case DY_CORE_EXPR_TYPE_MAP: {
        add_string(string, DY_STR_LIT("("));

        if (expr.type_map.is_implicit) {
            add_string(string, DY_STR_LIT("@["));
        } else {
            add_string(string, DY_STR_LIT("["));
        }
        char *c;
        dy_assert(asprintf(&c, "%zu", expr.type_map.arg_id) != -1);
        add_string(string, (dy_string_t){ .ptr = c, .size = strlen(c) });
        free(c);
        add_string(string, DY_STR_LIT(" "));
        dy_core_expr_to_string(*expr.type_map.arg_type, string);
        add_string(string, DY_STR_LIT("]"));
        if (expr.type_map.polarity == DY_CORE_POLARITY_POSITIVE) {
            add_string(string, DY_STR_LIT(" -> "));
        } else {
            add_string(string, DY_STR_LIT(" ~> "));
        }
        dy_core_expr_to_string(*expr.type_map.expr, string);
        add_string(string, DY_STR_LIT(")"));
        return;
    }
    case DY_CORE_EXPR_INFERENCE_CTX: {
        add_string(string, DY_STR_LIT("("));

        add_string(string, DY_STR_LIT("?["));
        char *c;
        dy_assert(asprintf(&c, "%zu", expr.inference_ctx.id) != -1);
        add_string(string, (dy_string_t){ .ptr = c, .size = strlen(c) });
        free(c);
        add_string(string, DY_STR_LIT(" "));
        dy_core_expr_to_string(*expr.inference_ctx.type, string);
        add_string(string, DY_STR_LIT("]"));
        if (expr.inference_ctx.polarity == DY_CORE_POLARITY_POSITIVE) {
            add_string(string, DY_STR_LIT(" -> "));
        } else {
            add_string(string, DY_STR_LIT(" ~> "));
        }
        dy_core_expr_to_string(*expr.inference_ctx.expr, string);
        add_string(string, DY_STR_LIT(")"));
        return;
    }
    case DY_CORE_EXPR_EXPR_MAP_ELIM: {
        dy_core_expr_to_string(*expr.expr_map_elim.expr, string);
        add_string(string, DY_STR_LIT(" ! "));
        if (expr.expr_map_elim.expr_map.is_implicit) {
            add_string(string, DY_STR_LIT("@"));
        }
        dy_core_expr_to_string(*expr.expr_map_elim.expr_map.e1, string);
        add_string(string, DY_STR_LIT(" ~> "));
        dy_core_expr_to_string(*expr.expr_map_elim.expr_map.e2, string);
        return;
    }
    case DY_CORE_EXPR_TYPE_MAP_ELIM: {
        dy_core_expr_to_string(*expr.type_map_elim.expr, string);
        add_string(string, DY_STR_LIT(" ! "));

        char *c;
        dy_assert(asprintf(&c, "%zu [", expr.type_map_elim.type_map.arg_id) != -1);
        add_string(string, (dy_string_t){ .ptr = c, .size = strlen(c) });
        free(c);

        dy_core_expr_to_string(*expr.type_map_elim.type_map.arg_type, string);
        add_string(string, DY_STR_LIT("]"));
        add_string(string, DY_STR_LIT(" ~> "));
        dy_core_expr_to_string(*expr.type_map_elim.type_map.expr, string);
        return;
    }
    case DY_CORE_EXPR_UNKNOWN: {
        if (expr.unknown.is_inference_var) {
            add_string(string, DY_STR_LIT("?"));
        }

        char *c;
        dy_assert(asprintf(&c, "%zu", expr.unknown.id) != -1);
        add_string(string, (dy_string_t){ .ptr = c, .size = strlen(c) });
        free(c);
        return;
    }
    case DY_CORE_EXPR_END:
        if (expr.end_polarity == DY_CORE_POLARITY_POSITIVE) {
            add_string(string, DY_STR_LIT("All"));
        } else {
            add_string(string, DY_STR_LIT("Nothing"));
        }

        return;
    case DY_CORE_EXPR_BOTH:
        dy_core_expr_to_string(*expr.both.e1, string);

        if (expr.both.polarity == DY_CORE_POLARITY_POSITIVE) {
            add_string(string, DY_STR_LIT(" and "));
        } else {
            add_string(string, DY_STR_LIT(" or "));
        }

        dy_core_expr_to_string(*expr.both.e2, string);
        return;
    case DY_CORE_EXPR_ONE_OF:
        dy_core_expr_to_string(*expr.one_of.first, string);
        add_string(string, DY_STR_LIT(" else "));
        dy_core_expr_to_string(*expr.one_of.second, string);
        return;
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

    dy_bail("should never be reached");
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
