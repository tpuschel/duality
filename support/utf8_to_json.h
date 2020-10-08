/*
 * Copyright 2019-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "json.h"
#include "stream.h"

/**
 * Implements parsing JSON from a UTF-8 character stream.
 *
 * Floating-point values are not currently supported.
 */

static inline bool dy_utf8_to_json(struct dy_stream *stream, dy_array_t *json);

static inline void parse_whitespace(struct dy_stream *stream);
static inline bool parse_value(struct dy_stream *stream, dy_array_t *json);
static inline bool parse_object(struct dy_stream *stream, dy_array_t *json);
static inline bool parse_empty_object(struct dy_stream *stream, dy_array_t *json);
static inline bool parse_nonempty_object(struct dy_stream *stream, dy_array_t *json);
static inline bool dy_json_parse_string(struct dy_stream *stream, dy_array_t *json);
static inline bool parse_array(struct dy_stream *stream, dy_array_t *json);
static inline bool parse_empty_array(struct dy_stream *stream, dy_array_t *json);
static inline bool parse_nonempty_array(struct dy_stream *stream, dy_array_t *json);
static inline bool parse_integer(struct dy_stream *stream, dy_array_t *json);

static bool hex_char(char c, uint8_t *hex);

bool dy_utf8_to_json(struct dy_stream *stream, dy_array_t *json)
{
    size_t start_index = stream->current_index;

    parse_whitespace(stream);

    if (!parse_value(stream, json)) {
        stream->current_index = start_index;
        return false;
    }

    parse_whitespace(stream);

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

bool parse_value(struct dy_stream *stream, dy_array_t *json)
{
    if (dy_stream_parse_literal(stream, DY_STR_LIT("true"))) {
        dy_array_add(json, &(uint8_t){ DY_JSON_TRUE });
        return true;
    }

    if (dy_stream_parse_literal(stream, DY_STR_LIT("false"))) {
        dy_array_add(json, &(uint8_t){ DY_JSON_FALSE });
        return true;
    }

    if (dy_stream_parse_literal(stream, DY_STR_LIT("null"))) {
        dy_array_add(json, &(uint8_t){ DY_JSON_NULL });
        return true;
    }

    if (parse_object(stream, json)) {
        return true;
    }

    if (dy_json_parse_string(stream, json)) {
        return true;
    }

    if (parse_array(stream, json)) {
        return true;
    }

    if (parse_integer(stream, json)) {
        return true;
    }

    return false;
}

bool parse_object(struct dy_stream *stream, dy_array_t *json)
{
    if (parse_empty_object(stream, json)) {
        return true;
    }

    if (parse_nonempty_object(stream, json)) {
        return true;
    }

    return false;
}

bool parse_empty_object(struct dy_stream *stream, dy_array_t *json)
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

    dy_array_add(json, &(uint8_t){ DY_JSON_OBJECT });
    dy_array_add(json, &(uint8_t){ DY_JSON_END });

    return true;
}

bool parse_nonempty_object(struct dy_stream *stream, dy_array_t *json)
{
    size_t start_index = stream->current_index;
    size_t write_index = json->num_elems;

    if (!dy_stream_parse_literal(stream, DY_STR_LIT("{"))) {
        return false;
    }

    dy_array_add(json, &(uint8_t){ DY_JSON_OBJECT });

    for (;;) {
        parse_whitespace(stream);

        if (!dy_json_parse_string(stream, json)) {
            stream->current_index = start_index;
            return false;
        }

        parse_whitespace(stream);

        if (!dy_stream_parse_literal(stream, DY_STR_LIT(":"))) {
            stream->current_index = start_index;
            json->num_elems = write_index;
            return false;
        }

        if (!dy_utf8_to_json(stream, json)) {
            stream->current_index = start_index;
            json->num_elems = write_index;
            return false;
        }

        if (!dy_stream_parse_literal(stream, DY_STR_LIT(","))) {
            break;
        }
    }

    if (!dy_stream_parse_literal(stream, DY_STR_LIT("}"))) {
        stream->current_index = start_index;
        json->num_elems = write_index;
        return false;
    }

    dy_array_add(json, &(uint8_t){ DY_JSON_END });

    return true;
}

bool dy_json_parse_string(struct dy_stream *stream, dy_array_t *json)
{
    size_t start_index = stream->current_index;
    size_t write_index = json->num_elems;

    if (!dy_stream_parse_literal(stream, DY_STR_LIT("\""))) {
        stream->current_index = start_index;
        return false;
    }

    dy_array_add(json, &(uint8_t){ DY_JSON_STRING });

    for (;;) {
        char c;
        if (!dy_stream_get_char(stream, &c)) {
            stream->current_index = start_index;
            json->num_elems = write_index;
            return false;
        }

        if (c == '\"') {
            dy_array_add(json, &(uint8_t){ DY_JSON_END });
            break;
        }

        if (c == '\\') {
            char c2;
            if (!dy_stream_get_char(stream, &c2)) {
                stream->current_index = start_index;
                json->num_elems = write_index;
                return false;
            }

            if (c2 == '\"') {
                dy_array_add(json, &(char){ '\"' });
                continue;
            }

            if (c2 == '\\') {
                dy_array_add(json, &(char){ '\\' });
                continue;
            }

            if (c2 == '/') {
                dy_array_add(json, &(char){ '/' });
                continue;
            }

            if (c2 == 'b') {
                dy_array_add(json, &(char){ '\b' });
                continue;
            }

            if (c2 == 'f') {
                dy_array_add(json, &(char){ '\f' });
                continue;
            }

            if (c2 == 'n') {
                dy_array_add(json, &(char){ '\n' });
                continue;
            }

            if (c2 == 'r') {
                dy_array_add(json, &(char){ '\r' });
                continue;
            }

            if (c2 == 't') {
                dy_array_add(json, &(char){ '\t' });
                continue;
            }

            if (c2 == 'u') {
                char c3;
                uint8_t hex1;
                if (!dy_stream_get_char(stream, &c3) || !hex_char(c3, &hex1)) {
                    stream->current_index = start_index;
                    json->num_elems = write_index;
                    return false;
                }

                char c4;
                uint8_t hex2;
                if (!dy_stream_get_char(stream, &c4) || !hex_char(c4, &hex2)) {
                    stream->current_index = start_index;
                    json->num_elems = write_index;
                    return false;
                }

                char c5;
                uint8_t hex3;
                if (!dy_stream_get_char(stream, &c5) || !hex_char(c5, &hex3)) {
                    stream->current_index = start_index;
                    json->num_elems = write_index;
                    return false;
                }

                char c6;
                uint8_t hex4;
                if (!dy_stream_get_char(stream, &c6) || !hex_char(c6, &hex4)) {
                    stream->current_index = start_index;
                    json->num_elems = write_index;
                    return false;
                }

                // Gnarly utf8 bit shifting stuff.

                uint16_t hex = (uint16_t)((hex1 << 12) | (hex2 << 8) | (hex3 << 4) | hex4);

                if (!(hex & 0xff80)) {
                    dy_array_add(json, &(char){ (char)hex });
                    continue;
                }

                if (!(hex & 0xf800)) {
                    dy_array_add(json, &(char){ (char)(0xc | (hex & 0x07c0) >> 6) });
                    dy_array_add(json, &(char){ (char)(0x8 | (hex & 0x003f)) });
                    continue;
                }

                dy_array_add(json, &(char){ (char)(0xe | (hex & 0xf000) >> (6 + 6)) });
                dy_array_add(json, &(char){ (char)(0x8 | (hex & 0x0fc0) >> 6) });
                dy_array_add(json, &(char){ (char)(0x8 | (hex & 0x003f)) });

                continue;
            }

            stream->current_index = start_index;
            json->num_elems = write_index;
            return false;
        }

        dy_array_add(json, &c);
    }

    return true;
}

bool parse_array(struct dy_stream *stream, dy_array_t *json)
{
    if (parse_empty_array(stream, json)) {
        return true;
    }

    if (parse_nonempty_array(stream, json)) {
        return true;
    }

    return false;
}

bool parse_empty_array(struct dy_stream *stream, dy_array_t *json)
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

    dy_array_add(json, &(uint8_t){ DY_JSON_ARRAY });
    dy_array_add(json, &(uint8_t){ DY_JSON_END });

    return true;
}

bool parse_nonempty_array(struct dy_stream *stream, dy_array_t *json)
{
    size_t start_index = stream->current_index;
    size_t write_index = json->num_elems;

    if (!dy_stream_parse_literal(stream, DY_STR_LIT("["))) {
        stream->current_index = start_index;
        return false;
    }

    dy_array_add(json, &(uint8_t){ DY_JSON_ARRAY });

    for (;;) {
        if (!dy_utf8_to_json(stream, json)) {
            stream->current_index = start_index;
            json->num_elems = write_index;
            return false;
        }

        if (!dy_stream_parse_literal(stream, DY_STR_LIT(","))) {
            break;
        }
    }

    if (!dy_stream_parse_literal(stream, DY_STR_LIT("]"))) {
        stream->current_index = start_index;
        json->num_elems = write_index;
        return false;
    }

    dy_array_add(json, &(uint8_t){ DY_JSON_END });

    return true;
}

bool parse_integer(struct dy_stream *stream, dy_array_t *json)
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

    dy_array_add(json, &(uint8_t){ DY_JSON_NUMBER });

    for (size_t i = 0; i < sizeof(long); ++i) {
        dy_array_add(json, (uint8_t *)&x + i);
    }

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
