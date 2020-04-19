/*
 * Copyright 2019-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_STREAM_H
#define DY_STREAM_H

#include "array.h"
#include "overflow.h"
#include "string.h"

#include <stdio.h>

struct dy_stream {
    void (*get_chars)(dy_array_t *buffer, void *env);
    dy_array_t buffer;
    void *env;

    size_t current_index;
};

static inline bool dy_stream_get_char(struct dy_stream *stream, char *c);

static inline void dy_stream_put_last_char_back(struct dy_stream *stream);

static inline void dy_stream_reset(struct dy_stream *stream);

static inline bool dy_stream_parse_literal(struct dy_stream *stream, dy_string_t literal);

static inline bool dy_stream_parse_size_t_decimal(struct dy_stream *stream, size_t *number);

static inline void dy_stream_dump(struct dy_stream *stream, FILE *file);

bool dy_stream_get_char(struct dy_stream *stream, char *c)
{
    if (stream->current_index == stream->buffer.num_elems) {
        stream->get_chars(&stream->buffer, stream->env);

        if (stream->current_index == stream->buffer.num_elems) {
            return false;
        }
    }

    dy_array_get(stream->buffer, stream->current_index, c);
    ++stream->current_index;
    return true;
}

void dy_stream_put_last_char_back(struct dy_stream *stream)
{
    --stream->current_index;
}

void dy_stream_reset(struct dy_stream *stream)
{
    stream->buffer.num_elems = 0;
    stream->current_index = 0;
}

bool dy_stream_parse_literal(struct dy_stream *stream, dy_string_t literal)
{
    size_t start_index = stream->current_index;

    for (size_t i = 0; i < literal.size; ++i) {
        char c;
        if (!dy_stream_get_char(stream, &c)) {
            stream->current_index = start_index;
            return false;
        }

        if (c != literal.ptr[i]) {
            stream->current_index = start_index;
            return false;
        }
    }

    return true;
}

bool dy_stream_parse_size_t_decimal(struct dy_stream *stream, size_t *number)
{
    size_t start_index = stream->current_index;

    size_t n = 0;
    bool first_run = true;
    for (;;) {
        char c;
        if (!dy_stream_get_char(stream, &c)) {
            if (first_run) {
                stream->current_index = start_index;
                return false;
            } else {
                break;
            }
        }

        if (c < '0' || '9' < c) {
            if (first_run) {
                stream->current_index = start_index;
                return false;
            } else {
                dy_stream_put_last_char_back(stream);
                break;
            }
        }

        first_run = false;

        if (dy_size_t_mul_overflow(n, 10, &n)) {
            dy_stream_put_last_char_back(stream);
            break;
        }

        if (dy_size_t_add_overflow(n, (size_t)(c - '0'), &n)) {
            dy_stream_put_last_char_back(stream);
            break;
        }
    }

    *number = n;

    return true;
}

void dy_stream_dump(struct dy_stream *stream, FILE *file)
{
    fprintf(file, "[Stream dump - begin]\n");

    for (size_t i = stream->current_index; i < stream->buffer.num_elems; ++i) {
        char c = *(char *)dy_array_pos(stream->buffer, i);
        if (c == '\n') {
            fprintf(file, "\\n");
        } else if (c == '\t') {
            fprintf(file, "\\t");
        } else if (c == '\r') {
            fprintf(file, "\\r");
        } else {
            fprintf(file, "%c", c);
        }
    }

    fprintf(file, "\n[Stream dump - end]\n");
}

#endif // DY_STREAM_H
