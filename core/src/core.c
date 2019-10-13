/*
 * Copyright 2017 - 2019, Thorben Hasenpusch
 * Licensed under the MIT license.
 */

#include <duality/core/core.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <duality/support/assert.h>

static void add_string(dy_array_t *string, dy_string_t s);

#ifdef _WIN32
static int asprintf(char **ret, const char *format, ...);
#endif

void dy_core_expr_to_string(struct dy_core_expr expr, dy_array_t *string)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_VALUE_MAP:
        add_string(string, DY_STR_LIT("("));
        dy_core_expr_to_string(*expr.value_map.e1, string);
        if (expr.value_map.polarity == DY_CORE_POLARITY_POSITIVE) {
            add_string(string, DY_STR_LIT(" -> "));
        } else {
            add_string(string, DY_STR_LIT(" ~> "));
        }
        dy_core_expr_to_string(*expr.value_map.e2, string);
        add_string(string, DY_STR_LIT(")"));
        return;
    case DY_CORE_EXPR_TYPE_MAP: {
        add_string(string, DY_STR_LIT("("));

        char *c;
        dy_assert(asprintf(&c, "%zu", expr.type_map.arg.id) != -1);
        add_string(string, (dy_string_t){ .ptr = c, .size = strlen(c) });
        free(c);

        add_string(string, DY_STR_LIT(" ["));
        dy_core_expr_to_string(*expr.type_map.arg.type, string);
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
    case DY_CORE_EXPR_VALUE_MAP_ELIM:
        dy_core_expr_to_string(*expr.value_map_elim.expr, string);
        add_string(string, DY_STR_LIT(" ! "));
        dy_core_expr_to_string(*expr.value_map_elim.value_map.e1, string);
        add_string(string, DY_STR_LIT(" ~> "));
        dy_core_expr_to_string(*expr.value_map_elim.value_map.e2, string);
        return;
    case DY_CORE_EXPR_TYPE_MAP_ELIM: {
        dy_core_expr_to_string(*expr.type_map_elim.expr, string);
        add_string(string, DY_STR_LIT(" ! "));

        char *c;
        dy_assert(asprintf(&c, "%zu [", expr.type_map_elim.type_map.arg.id) != -1);
        add_string(string, (dy_string_t){ .ptr = c, .size = strlen(c) });
        free(c);

        dy_core_expr_to_string(*expr.type_map_elim.type_map.arg.type, string);
        add_string(string, DY_STR_LIT("]"));
        add_string(string, DY_STR_LIT(" ~> "));
        dy_core_expr_to_string(*expr.type_map_elim.type_map.expr, string);
        return;
    }
    case DY_CORE_EXPR_UNKNOWN: {
        char *c;
        dy_assert(asprintf(&c, "%zu", expr.unknown.id) != -1);
        add_string(string, (dy_string_t){ .ptr = c, .size = strlen(c) });
        free(c);
        return;
    }
    case DY_CORE_EXPR_TYPE_OF_TYPES:
        add_string(string, DY_STR_LIT("Type"));
        return;
    case DY_CORE_EXPR_BOTH:
        dy_core_expr_to_string(*expr.both.first, string);
        add_string(string, DY_STR_LIT(" and "));
        dy_core_expr_to_string(*expr.both.second, string);
        return;
    case DY_CORE_EXPR_ANY_OF:
        dy_core_expr_to_string(*expr.both.first, string);
        add_string(string, DY_STR_LIT(" or "));
        dy_core_expr_to_string(*expr.both.second, string);
        return;
    case DY_CORE_EXPR_ONE_OF:
        dy_core_expr_to_string(*expr.both.first, string);
        add_string(string, DY_STR_LIT(" else "));
        dy_core_expr_to_string(*expr.both.second, string);
        return;
    case DY_CORE_EXPR_STRING:
        add_string(string, DY_STR_LIT("\""));
        add_string(string, expr.string);
        add_string(string, DY_STR_LIT("\""));
        return;
    case DY_CORE_EXPR_TYPE_OF_STRINGS:
        add_string(string, DY_STR_LIT("String"));
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

    int len = _vscprintf(format, ap);
    dy_assert(len >= 0);

    char *buffer = malloc((size_t)len + 1 /* for '\0' */);

    int ret = vsprintf(buffer, format, ap);
    dy_assert(ret == len);

    va_end(ap);

    *ret = buffer;

    return ret;
}
#endif
