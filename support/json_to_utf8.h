/*
 * Copyright 2019-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "json.h"
#include "array.h"
#include "string.h"

#include "../support/bail.h"

/**
 * Implements functions that append
 * a UTF-8 representation of the
 * given JSON to a dynamic array.
 */

static inline const uint8_t *dy_json_to_utf8(const uint8_t *json, dy_array_t *utf8);

static inline void dy_utf8_literal(dy_string_t s, dy_array_t *utf8);
static inline const uint8_t *dy_array_to_utf8(const uint8_t *json, dy_array_t *utf8);
static inline const uint8_t *dy_object_to_utf8(const uint8_t *json, dy_array_t *utf8);
static inline void dy_number_to_utf8(const uint8_t *json, dy_array_t *utf8);
static inline const uint8_t *dy_string_to_utf8(const uint8_t *json, dy_array_t *utf8);

const uint8_t *dy_json_to_utf8(const uint8_t *json, dy_array_t *utf8)
{
    switch (*json) {
    case DY_JSON_TRUE:
        dy_utf8_literal(DY_STR_LIT("true"), utf8);
        return json + 1;
    case DY_JSON_FALSE:
        dy_utf8_literal(DY_STR_LIT("false"), utf8);
        return json + 1;
    case DY_JSON_NULL:
        dy_utf8_literal(DY_STR_LIT("null"), utf8);
        return json + 1;
    case DY_JSON_ARRAY:
        return dy_array_to_utf8(json + 1, utf8);
    case DY_JSON_OBJECT:
        return dy_object_to_utf8(json + 1, utf8);
    case DY_JSON_NUMBER:
        dy_number_to_utf8(json + 1, utf8);
        return json + 1 + sizeof(long);
    case DY_JSON_STRING:
        return dy_string_to_utf8(json + 1, utf8);
    }

    dy_bail("Invalid enum tag.");
}

void dy_utf8_literal(dy_string_t s, dy_array_t *utf8)
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

const uint8_t *dy_array_to_utf8(const uint8_t *json, dy_array_t *utf8)
{
    dy_array_add(utf8, &(char){ '[' });

    if (*json != DY_JSON_END) {
        for (;;) {
            json = dy_json_to_utf8(json, utf8);

            if (*json == DY_JSON_END) {
                break;
            } else {
                dy_array_add(utf8, &(char){ ',' });
            }
        }
    }

    dy_array_add(utf8, &(char){ ']' });

    return json + 1;
}

const uint8_t *dy_object_to_utf8(const uint8_t *json, dy_array_t *utf8)
{
    dy_array_add(utf8, &(char){ '{' });

    if (*json != DY_JSON_END) {
        for (;;) {
            json = dy_string_to_utf8(json + 1, utf8);
            dy_array_add(utf8, &(char){ ':' });
            json = dy_json_to_utf8(json, utf8);

            if (*json == DY_JSON_END) {
                break;
            } else {
                dy_array_add(utf8, &(char){ ',' });
            }
        }
    }

    dy_array_add(utf8, &(char){ '}' });

    return json + 1;
}

void dy_number_to_utf8(const uint8_t *json, dy_array_t *utf8)
{
    long number;
    memcpy(&number, json, sizeof(number));

    if (number < 0) {
        dy_array_add(utf8, &(char){ '-' });
    }

    long absolute = labs(number);

    dy_array_t local = dy_array_create(sizeof(char), DY_ALIGNOF(char), 4);
    for (;;) {
        char digit = (absolute % 10) + '0';

        dy_array_add(&local, &digit);

        absolute = absolute / 10;

        if (absolute == 0) {
            break;
        }
    }

    for (size_t i = local.num_elems; i-- > 0;) {
        dy_array_add(utf8, dy_array_pos(local, i));
    }

    dy_array_destroy(local);
}

const uint8_t *dy_string_to_utf8(const uint8_t *json, dy_array_t *utf8)
{
    dy_array_add(utf8, &(char){ '\"' });

    while (*json != DY_JSON_END) {
        if (*json == '\"') {
            dy_array_add(utf8, &(char){ '\\' });
            dy_array_add(utf8, &(char){ '\"' });
        } else {
            dy_array_add(utf8, json);
        }

        ++json;
    }

    dy_array_add(utf8, &(char){ '\"' });

    return json + 1;
}
