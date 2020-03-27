/*
 * Copyright 2019-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <duality/support/utf8_to_json.h>

#include <duality/support/overflow.h>

#include <stdio.h>

static void parse_whitespace(struct dy_stream *stream);
static bool parse_value(struct dy_stream *stream, struct dy_json_value *value);
static bool parse_object(struct dy_stream *stream, struct dy_json_object *object);
static bool parse_empty_object(struct dy_stream *stream, struct dy_json_object *object);
static bool parse_nonempty_object(struct dy_stream *stream, struct dy_json_object *object);
static bool parse_object_members(struct dy_stream *stream, struct dy_json_object *object);
static bool parse_object_members_storage(struct dy_stream *stream, dy_array_t *member_storage);
static bool parse_object_member(struct dy_stream *stream, struct dy_json_member *member);
static bool parse_string(struct dy_stream *stream, dy_string_t *string);
static bool parse_characters(struct dy_stream *stream, dy_string_t *characters);
static bool parse_array(struct dy_stream *stream, struct dy_json_array *array);
static bool parse_empty_array(struct dy_stream *stream, struct dy_json_array *array);
static bool parse_nonempty_array(struct dy_stream *stream, struct dy_json_array *array);
static bool parse_array_elements_storage(struct dy_stream *stream, dy_array_t *element_storage);
static bool parse_array_elements(struct dy_stream *stream, struct dy_json_array *array);
static bool parse_number(struct dy_stream *stream, struct dy_json_number *number);
static bool parse_integer(struct dy_stream *stream, long *integer);

static bool hex_char(char c, uint8_t *hex);

bool dy_utf8_to_json(struct dy_stream *stream, dy_json_t *json)
{
    size_t start_index = stream->current_index;

    parse_whitespace(stream);

    struct dy_json_value value;
    if (!parse_value(stream, &value)) {
        stream->current_index = start_index;
        return false;
    }

    parse_whitespace(stream);

    *json = value;

    return true;
}

void parse_whitespace(struct dy_stream *stream)
{
    for (;;) {
        char c;
        if (!dy_stream_get_char(stream, &c)) {
            return;
        }

        if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
            dy_stream_put_last_char_back(stream);
            return;
        }
    }
}

bool parse_value(struct dy_stream *stream, struct dy_json_value *value)
{
    if (dy_stream_parse_literal(stream, DY_STR_LIT("true"))) {
        *value = (struct dy_json_value){
            .tag = DY_JSON_VALUE_TRUE
        };

        return true;
    }

    if (dy_stream_parse_literal(stream, DY_STR_LIT("false"))) {
        *value = (struct dy_json_value){
            .tag = DY_JSON_VALUE_FALSE
        };

        return true;
    }

    if (dy_stream_parse_literal(stream, DY_STR_LIT("null"))) {
        *value = (struct dy_json_value){
            .tag = DY_JSON_VALUE_NULL
        };

        return true;
    }

    struct dy_json_object object;
    if (parse_object(stream, &object)) {
        *value = (struct dy_json_value){
            .tag = DY_JSON_VALUE_OBJECT,
            .object = object
        };

        return true;
    }

    dy_string_t string;
    if (parse_string(stream, &string)) {
        *value = (struct dy_json_value){
            .tag = DY_JSON_VALUE_STRING,
            .string = string
        };

        return true;
    }

    struct dy_json_array array;
    if (parse_array(stream, &array)) {
        *value = (struct dy_json_value){
            .tag = DY_JSON_VALUE_ARRAY,
            .array = array
        };

        return true;
    }

    struct dy_json_number number;
    if (parse_number(stream, &number)) {
        *value = (struct dy_json_value){
            .tag = DY_JSON_VALUE_NUMBER,
            .number = number
        };

        return true;
    }

    return false;
}

bool parse_object(struct dy_stream *stream, struct dy_json_object *object)
{
    if (parse_empty_object(stream, object)) {
        return true;
    }

    if (parse_nonempty_object(stream, object)) {
        return true;
    }

    return false;
}

bool parse_empty_object(struct dy_stream *stream, struct dy_json_object *object)
{
    size_t start_index = stream->current_index;

    if (!dy_stream_parse_literal(stream, DY_STR_LIT("{"))) {
        stream->current_index = start_index;
        return false;
    }

    parse_whitespace(stream);

    if (!dy_stream_parse_literal(stream, DY_STR_LIT("}"))) {
        stream->current_index = start_index;
        return false;
    }

    *object = (struct dy_json_object){
        .members = NULL,
        .num_members = 0
    };

    return true;
}

bool parse_nonempty_object(struct dy_stream *stream, struct dy_json_object *object)
{
    size_t start_index = stream->current_index;

    if (!dy_stream_parse_literal(stream, DY_STR_LIT("{"))) {
        return false;
    }

    struct dy_json_object obj;
    if (!parse_object_members(stream, &obj)) {
        fprintf(stderr, "Failed to parse object members.\n");
        stream->current_index = start_index;
        return false;
    }

    if (!dy_stream_parse_literal(stream, DY_STR_LIT("}"))) {
        fprintf(stderr, "Missing closing brace.\n");
        stream->current_index = start_index;
        return false;
    }

    *object = obj;

    return true;
}

bool parse_object_members(struct dy_stream *stream, struct dy_json_object *object)
{
    dy_array_t *member_storage = dy_array_create(sizeof(struct dy_json_member), 4);

    if (!parse_object_members_storage(stream, member_storage)) {
        return false;
    }

    *object = (struct dy_json_object){
        .members = dy_array_buffer(member_storage),
        .num_members = dy_array_size(member_storage)
    };

    return true;
}

bool parse_object_members_storage(struct dy_stream *stream, dy_array_t *member_storage)
{
    size_t start_index = stream->current_index;

    struct dy_json_member member;
    if (!parse_object_member(stream, &member)) {
        fprintf(stderr, "Failed to parse object member\n");
        stream->current_index = start_index;
        return false;
    }

    dy_array_add(member_storage, &member);

    if (!dy_stream_parse_literal(stream, DY_STR_LIT(","))) {
        return true;
    }

    if (!parse_object_members_storage(stream, member_storage)) {
        fprintf(stderr, "Failed to parse further.\n");
        stream->current_index = start_index;
        return false;
    }

    return true;
}

bool parse_object_member(struct dy_stream *stream, struct dy_json_member *member)
{
    size_t start_index = stream->current_index;

    parse_whitespace(stream);

    dy_string_t string;
    if (!parse_string(stream, &string)) {
        fprintf(stderr, "Not string\n");
        stream->current_index = start_index;
        return false;
    }

    parse_whitespace(stream);

    if (!dy_stream_parse_literal(stream, DY_STR_LIT(":"))) {
        fprintf(stderr, "Missing colon\n");
        stream->current_index = start_index;
        return false;
    }

    struct dy_json_value value;
    if (!dy_utf8_to_json(stream, &value)) {
        fprintf(stderr, "Failed json member parse.\n");
        stream->current_index = start_index;
        return false;
    }

    *member = (struct dy_json_member){
        .string = string,
        .value = value
    };

    return true;
}

bool parse_string(struct dy_stream *stream, dy_string_t *string)
{
    size_t start_index = stream->current_index;

    if (!dy_stream_parse_literal(stream, DY_STR_LIT("\""))) {
        stream->current_index = start_index;
        return false;
    }

    dy_string_t characters;
    if (!parse_characters(stream, &characters)) {
        fprintf(stderr, "Not characters\n");
        stream->current_index = start_index;
        return false;
    }

    if (!dy_stream_parse_literal(stream, DY_STR_LIT("\""))) {
        fprintf(stderr, "Missing closing quote\n");
        stream->current_index = start_index;
        return false;
    }

    *string = characters;

    return true;
}

bool parse_characters(struct dy_stream *stream, dy_string_t *characters)
{
    size_t start_index = stream->current_index;

    dy_array_t *characters_storage = dy_array_create(sizeof(char), 8);

    for (;;) {
        char c;
        if (!dy_stream_get_char(stream, &c)) {
            *characters = (dy_string_t){
                .ptr = dy_array_buffer(characters_storage),
                .size = dy_array_size(characters_storage)
            };

            return true;
        }

        if (c == '\"') {
            dy_stream_put_last_char_back(stream);

            *characters = (dy_string_t){
                .ptr = dy_array_buffer(characters_storage),
                .size = dy_array_size(characters_storage)
            };

            return true;
        }

        if (c == '\\') {
            char c2;
            if (!dy_stream_get_char(stream, &c2)) {
                stream->current_index = start_index;
                return false;
            }

            if (c2 == '\"') {
                dy_array_add(characters_storage, &(char){ '\"' });
                continue;
            }

            if (c2 == '\\') {
                dy_array_add(characters_storage, &(char){ '\\' });
                continue;
            }

            if (c2 == '/') {
                dy_array_add(characters_storage, &(char){ '/' });
                continue;
            }

            if (c2 == 'b') {
                dy_array_add(characters_storage, &(char){ '\b' });
                continue;
            }

            if (c2 == 'f') {
                dy_array_add(characters_storage, &(char){ '\f' });
                continue;
            }

            if (c2 == 'n') {
                dy_array_add(characters_storage, &(char){ '\n' });
                continue;
            }

            if (c2 == 'r') {
                dy_array_add(characters_storage, &(char){ '\r' });
                continue;
            }

            if (c2 == 't') {
                dy_array_add(characters_storage, &(char){ '\t' });
                continue;
            }

            if (c2 == 'u') {
                char c3;
                uint8_t hex1;
                if (!dy_stream_get_char(stream, &c3) || !hex_char(c3, &hex1)) {
                    stream->current_index = start_index;
                    return false;
                }

                char c4;
                uint8_t hex2;
                if (!dy_stream_get_char(stream, &c4) || !hex_char(c4, &hex2)) {
                    stream->current_index = start_index;
                    return false;
                }

                char c5;
                uint8_t hex3;
                if (!dy_stream_get_char(stream, &c5) || !hex_char(c5, &hex3)) {
                    stream->current_index = start_index;
                    return false;
                }

                char c6;
                uint8_t hex4;
                if (!dy_stream_get_char(stream, &c6) || !hex_char(c6, &hex4)) {
                    stream->current_index = start_index;
                    return false;
                }

                uint16_t hex = (uint16_t)((hex1 << 12) | (hex2 << 8) | (hex3 << 4) | hex4);

                if (!(hex & 0xff80)) {
                    dy_array_add(characters_storage, &(char){ (char)hex });
                    continue;
                }

                if (!(hex & 0xf800)) {
                    dy_array_add(characters_storage, &(char){ (char)(0xc | (hex & 0x07c0) >> 6) });
                    dy_array_add(characters_storage, &(char){ (char)(0x8 | (hex & 0x003f)) });
                    continue;
                }

                dy_array_add(characters_storage, &(char){ (char)(0xe | (hex & 0xf000) >> (6 + 6)) });
                dy_array_add(characters_storage, &(char){ (char)(0x8 | (hex & 0x0fc0) >> 6) });
                dy_array_add(characters_storage, &(char){ (char)(0x8 | (hex & 0x003f)) });

                continue;
            }

            stream->current_index = start_index;

            return false;
        }

        dy_array_add(characters_storage, &c);
    }
}

bool parse_array(struct dy_stream *stream, struct dy_json_array *array)
{
    if (parse_empty_array(stream, array)) {
        return true;
    }

    if (parse_nonempty_array(stream, array)) {
        return true;
    }

    return false;
}

bool parse_empty_array(struct dy_stream *stream, struct dy_json_array *array)
{
    size_t start_index = stream->current_index;

    if (!dy_stream_parse_literal(stream, DY_STR_LIT("["))) {
        stream->current_index = start_index;
        return false;
    }

    parse_whitespace(stream);

    if (!dy_stream_parse_literal(stream, DY_STR_LIT("]"))) {
        stream->current_index = start_index;
        return false;
    }

    *array = (struct dy_json_array){
        .values = NULL,
        .num_values = 0
    };

    return true;
}

bool parse_nonempty_array(struct dy_stream *stream, struct dy_json_array *array)
{
    size_t start_index = stream->current_index;

    if (!dy_stream_parse_literal(stream, DY_STR_LIT("["))) {
        stream->current_index = start_index;
        return false;
    }

    struct dy_json_array arr;
    if (!parse_array_elements(stream, &arr)) {
        fprintf(stderr, "Failed array element parse.\n");
        stream->current_index = start_index;
        return false;
    }

    if (!dy_stream_parse_literal(stream, DY_STR_LIT("]"))) {
        fprintf(stderr, "Missing closing bracket.\n");
        stream->current_index = start_index;
        return false;
    }

    *array = arr;

    return true;
}

bool parse_array_elements(struct dy_stream *stream, struct dy_json_array *array)
{
    dy_array_t *element_storage = dy_array_create(sizeof(struct dy_json_value), 4);

    if (!parse_array_elements_storage(stream, element_storage)) {
        return false;
    }

    *array = (struct dy_json_array){
        .values = dy_array_buffer(element_storage),
        .num_values = dy_array_size(element_storage)
    };

    return true;
}

bool parse_array_elements_storage(struct dy_stream *stream, dy_array_t *element_storage)
{
    size_t start_index = stream->current_index;

    struct dy_json_value value;
    if (!dy_utf8_to_json(stream, &value)) {
        stream->current_index = start_index;
        return false;
    }

    dy_array_add(element_storage, &value);

    if (!dy_stream_parse_literal(stream, DY_STR_LIT(","))) {
        return true;
    }

    if (!parse_array_elements_storage(stream, element_storage)) {
        stream->current_index = start_index;
        return false;
    }

    return true;
}

bool parse_number(struct dy_stream *stream, struct dy_json_number *number)
{
    long integer;
    if (parse_integer(stream, &integer)) {
        *number = (struct dy_json_number){
            .tag = DY_JSON_NUMBER_INTEGER,
            .integer = integer
        };

        return true;
    }

    return false;
}

bool parse_integer(struct dy_stream *stream, long *integer)
{
    size_t start_index = stream->current_index;

    bool is_negative = dy_stream_parse_literal(stream, DY_STR_LIT("-"));

    long x = 0;
    bool first_round = true;

    for (;;) {
        char c;
        if (!dy_stream_get_char(stream, &c)) {
            if (first_round) {
                stream->current_index = start_index;
                return false;
            } else {
                break;
            }
        }

        if (c < '0' || '9' < c) {
            if (first_round) {
                stream->current_index = start_index;
                return false;
            } else {
                dy_stream_put_last_char_back(stream);
                break;
            }
        }

        if (dy_smull_overflow(x, 10, &x)) {
            stream->current_index = start_index;
            return false;
        }

        if (dy_saddl_overflow(x, c - '0', &x)) {
            stream->current_index = start_index;
            return false;
        }

        first_round = false;
    }

    if (is_negative) {
        if (dy_smull_overflow(x, -1, &x)) {
            stream->current_index = start_index;
            return false;
        }
    }

    *integer = x;

    return true;
}

bool hex_char(char c, uint8_t *hex)
{
    if ('0' <= c && c <= '9') {
        *hex = (uint8_t)(c - '0');
        return true;
    }

    if ('a' <= c && c <= 'f') {
        *hex = (uint8_t)(c - 'a');
        return true;
    }

    if ('A' <= c && c <= 'F') {
        *hex = (uint8_t)(c - 'A');
        return true;
    }

    return false;
}
