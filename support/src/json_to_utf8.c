/*
 * Copyright 2019-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <duality/support/json_to_utf8.h>

#include <duality/support/assert.h>

#include <stdlib.h>

static void utf8_literal(dy_string_t s, dy_array_t *utf8);
static void array_to_utf8(struct dy_json_array array, dy_array_t *utf8);
static void object_to_utf8(struct dy_json_object object, dy_array_t *utf8);
static void number_to_utf8(struct dy_json_number number, dy_array_t *utf8);
static void string_to_utf8(dy_string_t string, dy_array_t *utf8);

void dy_json_to_utf8(dy_json_t json, dy_array_t *utf8)
{
    switch (json.tag) {
    case DY_JSON_VALUE_TRUE:
        utf8_literal(DY_STR_LIT("true"), utf8);
        return;
    case DY_JSON_VALUE_FALSE:
        utf8_literal(DY_STR_LIT("false"), utf8);
        return;
    case DY_JSON_VALUE_NULL:
        utf8_literal(DY_STR_LIT("null"), utf8);
        return;
    case DY_JSON_VALUE_ARRAY:
        array_to_utf8(json.array, utf8);
        return;
    case DY_JSON_VALUE_OBJECT:
        object_to_utf8(json.object, utf8);
        return;
    case DY_JSON_VALUE_NUMBER:
        number_to_utf8(json.number, utf8);
        return;
    case DY_JSON_VALUE_STRING:
        string_to_utf8(json.string, utf8);
        return;
    }

    DY_IMPOSSIBLE_ENUM();
}

void utf8_literal(dy_string_t s, dy_array_t *utf8)
{
    for (size_t i = 0; i < s.size; ++i) {
        if (s.ptr[i] == '\"') {
            dy_array_add(utf8, &(char){ '\\' });
            dy_array_add(utf8, &(char){ '\"' });
            continue;
        }

        dy_array_add(utf8, s.ptr + i);
    }
}

void array_to_utf8(struct dy_json_array array, dy_array_t *utf8)
{
    dy_array_add(utf8, &(char){ '[' });

    if (array.num_values >= 1) {
        dy_json_to_utf8(array.values[0], utf8);
    }

    for (size_t i = 1; i < array.num_values; ++i) {
        dy_array_add(utf8, &(char){ ',' });
        dy_json_to_utf8(array.values[i], utf8);
    }

    dy_array_add(utf8, &(char){ ']' });
}

void object_to_utf8(struct dy_json_object object, dy_array_t *utf8)
{
    dy_array_add(utf8, &(char){ '{' });

    if (object.num_members >= 1) {
        string_to_utf8(object.members[0].string, utf8);
        dy_array_add(utf8, &(char){ ':' });
        dy_json_to_utf8(object.members[0].value, utf8);
    }

    for (size_t i = 1; i < object.num_members; ++i) {
        dy_array_add(utf8, &(char){ ',' });
        string_to_utf8(object.members[i].string, utf8);
        dy_array_add(utf8, &(char){ ':' });
        dy_json_to_utf8(object.members[i].value, utf8);
    }

    dy_array_add(utf8, &(char){ '}' });
}

void number_to_utf8(struct dy_json_number number, dy_array_t *utf8)
{
    if (number.tag != DY_JSON_NUMBER_INTEGER) {
        return;
    }

    if (number.integer < 0) {
        dy_array_add(utf8, &(char){ '-' });
    }

    long absolute = labs(number.integer);

    dy_array_t *local = dy_array_create(dy_allocator_stdlib(), sizeof(char), 4);
    for (;;) {
        char digit = (absolute % 10) + '0';

        dy_array_add(local, &digit);

        absolute = absolute / 10;

        if (absolute == 0) {
            break;
        }
    }

    for (size_t i = dy_array_size(local); i-- > 0;) {
        dy_array_add(utf8, dy_array_get_ptr(local, i));
    }

    dy_array_destroy(local);
}

void string_to_utf8(dy_string_t string, dy_array_t *utf8)
{
    dy_array_add(utf8, &(char){ '\"' });
    utf8_literal(string, utf8);
    dy_array_add(utf8, &(char){ '\"' });
}
