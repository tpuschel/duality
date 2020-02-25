/*
 * Copyright 2019-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_STREAM_H
#define DY_STREAM_H

#include <duality/support/array.h>
#include <duality/support/string.h>

struct dy_stream {
    void (*get_chars)(dy_array_t *buffer, void *env);
    dy_array_t *buffer;
    void *env;

    size_t current_index;
};

DY_SUPPORT_API bool dy_stream_get_char(struct dy_stream *stream, char *c);

DY_SUPPORT_API void dy_stream_put_last_char_back(struct dy_stream *stream);

DY_SUPPORT_API bool dy_stream_parse_literal(struct dy_stream *stream, dy_string_t literal);

DY_SUPPORT_API bool dy_stream_parse_size_t_decimal(struct dy_stream *stream, size_t *number);

#endif // DY_STREAM_H
