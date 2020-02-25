/*
 * Copyright 2019-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_JSON_H
#define DY_JSON_H

#include <duality/support/string.h>

struct dy_json_element;

struct dy_json_object {
    const struct dy_json_member *members;
    size_t num_members;
};

struct dy_json_array {
    const struct dy_json_value *values;
    size_t num_values;
};

enum dy_json_number_tag {
    DY_JSON_NUMBER_INTEGER,
    DY_JSON_NUMBER_FLOATING_POINT
};

struct dy_json_number {
    union {
        long integer;
        double floating_point;
    };

    enum dy_json_number_tag tag;
};

enum dy_json_value_tag {
    DY_JSON_VALUE_OBJECT,
    DY_JSON_VALUE_ARRAY,
    DY_JSON_VALUE_STRING,
    DY_JSON_VALUE_NUMBER,
    DY_JSON_VALUE_TRUE,
    DY_JSON_VALUE_FALSE,
    DY_JSON_VALUE_NULL
};

struct dy_json_value {
    union {
        struct dy_json_object object;
        struct dy_json_array array;
        dy_string_t string;
        struct dy_json_number number;
    };

    enum dy_json_value_tag tag;
};

struct dy_json_member {
    dy_string_t string;
    struct dy_json_value value;
};

typedef struct dy_json_value dy_json_t;

#endif // DY_JSON_H
