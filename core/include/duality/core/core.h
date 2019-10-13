/*
 * Copyright 2017-2019 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_CORE_H
#define DY_CORE_H

#include <duality/support/string.h>
#include <duality/support/array.h>

#include <duality/core/api.h>

struct dy_core_expr;

enum dy_core_polarity {
    DY_CORE_POLARITY_POSITIVE,
    DY_CORE_POLARITY_NEGATIVE
};

struct dy_core_unknown {
    size_t id;
    const struct dy_core_expr *type;
};

struct dy_core_value_map {
    const struct dy_core_expr *e1;
    const struct dy_core_expr *e2;
    enum dy_core_polarity polarity;
};

struct dy_core_type_map {
    struct dy_core_unknown arg;
    const struct dy_core_expr *expr;
    enum dy_core_polarity polarity;
};

struct dy_core_value_map_elim {
    const struct dy_core_expr *expr;
    struct dy_core_value_map value_map;
};

struct dy_core_type_map_elim {
    const struct dy_core_expr *expr;
    struct dy_core_type_map type_map;
};

struct dy_core_pair {
    const struct dy_core_expr *first;
    const struct dy_core_expr *second;
};

enum dy_core_expr_tag {
    DY_CORE_EXPR_VALUE_MAP,
    DY_CORE_EXPR_TYPE_MAP,
    DY_CORE_EXPR_VALUE_MAP_ELIM,
    DY_CORE_EXPR_TYPE_MAP_ELIM,
    DY_CORE_EXPR_BOTH,
    DY_CORE_EXPR_ONE_OF,
    DY_CORE_EXPR_ANY_OF,
    DY_CORE_EXPR_UNKNOWN,
    DY_CORE_EXPR_TYPE_OF_TYPES,
    DY_CORE_EXPR_STRING,
    DY_CORE_EXPR_TYPE_OF_STRINGS
};

struct dy_core_expr {
    union {
        struct dy_core_value_map value_map;
        struct dy_core_type_map type_map;
        struct dy_core_value_map_elim value_map_elim;
        struct dy_core_type_map_elim type_map_elim;
        struct dy_core_pair both;
        struct dy_core_pair one_of;
        struct dy_core_pair any_of;
        struct dy_core_unknown unknown;
        dy_string_t string;
    };

    enum dy_core_expr_tag tag;
};

DY_CORE_API void dy_core_expr_to_string(struct dy_core_expr expr, dy_array_t *string);

#endif // DY_CORE_H
