/*
 * Copyright 2019-2021 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "array.h"
#include "overflow.h"
#include "string.h"

/**
 * Implements a buffered stream.
 *
 * The "end" of a stream is indicated by get_chars() not
 * adding any characters to 'buffer'.
 */

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

bool dy_stream_get_char(struct dy_stream *stream, char *c)
{
    if (stream->current_index == stream->buffer.num_elems) {
        stream->get_chars(&stream->buffer, stream->env);

        if (stream->current_index == stream->buffer.num_elems) {
            return false;
        }
    }

    memcpy(c, dy_array_pos(&stream->buffer, stream->current_index), 1);
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
