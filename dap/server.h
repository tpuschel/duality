/*
 * Copyright 2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "dap.h"

#include "../support/stream.h"
#include "../support/json.h"
#include "../support/json_to_utf8.h"
#include "../support/utf8_to_json.h"

#include <stdio.h>
#include <assert.h>

#ifdef _WIN32
#    include <io.h>
#    include <fcntl.h>
#endif

/**
 * This file implements the JSON message infrastructure for the DAP.
 */

struct dy_dap_stream_env {
    FILE *file;
    bool is_bounded;
    size_t bound_size_in_bytes;
};

struct send_env {
    dy_array_t buffer;
    FILE *file;
};

static inline int dy_dap_run_server(FILE *in, FILE *out);

static inline bool dy_dap_process_message(struct dy_stream *stream, dy_array_t *json);

static inline bool dy_dap_read_content_length(struct dy_stream *stream, size_t *content_length_in_bytes);

static inline void send_callback(const uint8_t *message, void *env);
static inline void stream_callback(dy_array_t *buffer, void *env);
static inline void write_size_t(FILE *file, size_t x);
static inline void set_file_to_binary(FILE *file);

int dy_dap_run_server(FILE *in, FILE *out)
{
    set_file_to_binary(in);
    set_file_to_binary(out);

    struct dy_dap_stream_env recv_env = {
        .file = in,
        .is_bounded = false
    };

    struct send_env send_env = {
        .buffer = dy_array_create(sizeof(char), DY_ALIGNOF(char), 1024),
        .file = out
    };

    struct dy_stream stream = {
        .get_chars = stream_callback,
        .env = &recv_env,
        .buffer = dy_array_create(sizeof(char), DY_ALIGNOF(char), 1024),
        .current_index = 0
    };

    dy_array_t json = dy_array_create(1, 1, 128);

    for (;;) {
        if (!dy_dap_process_message(&stream, &json)) {
            dy_array_release(&json);
            return -1;
        }
    }
}

bool dy_dap_process_message(struct dy_stream *stream, dy_array_t *json)
{
    struct dy_dap_stream_env *env = stream->env;

    env->is_bounded = false;

    size_t content_length_in_bytes;
    if (!dy_dap_read_content_length(stream, &content_length_in_bytes)) {
        return false;
    }

    if (!dy_stream_parse_literal(stream, DY_STR_LIT("\r\n"))) {
        return false;
    }

    env->is_bounded = true;
    env->bound_size_in_bytes = content_length_in_bytes;

    if (!dy_utf8_to_json(stream, json)) {
        return false;
    }

    dy_stream_reset(stream);

    return true;
}

bool dy_dap_read_content_length(struct dy_stream *stream, size_t *content_length_in_bytes)
{
    size_t index = stream->current_index;

    if (!dy_stream_parse_literal(stream, DY_STR_LIT("Content-Length: "))) {
        stream->current_index = index;
        return false;
    }

    size_t number;
    if (!dy_stream_parse_size_t_decimal(stream, &number)) {
        stream->current_index = index;
        return false;
    }

    if (!dy_stream_parse_literal(stream, DY_STR_LIT("\r\n"))) {
        stream->current_index = index;
        return false;
    }

    *content_length_in_bytes = number;

    return true;
}

void send_callback(const uint8_t *message, void *env)
{
    struct send_env *send_env = env;

    size_t size = send_env->buffer.num_elems;

    dy_json_to_utf8(message, &send_env->buffer);

    fprintf(send_env->file, "Content-Length:");
    write_size_t(send_env->file, send_env->buffer.num_elems - size);
    fprintf(send_env->file, "\r\n\r\n");

    fwrite((char *)send_env->buffer.buffer + size, sizeof(char), send_env->buffer.num_elems - size, send_env->file);

    fflush(send_env->file);

    send_env->buffer.num_elems = size;
}

void stream_callback(dy_array_t *buffer, void *env)
{
    struct dy_dap_stream_env *state = env;

    if (feof(stdin) || ferror(stdin)) {
        return;
    }

    if (state->is_bounded) {
        if (state->bound_size_in_bytes == 0) {
            return;
        }

        dy_array_set_excess_capacity(buffer, state->bound_size_in_bytes);

        size_t num_bytes_read = fread(dy_array_excess_buffer(buffer), sizeof(char), state->bound_size_in_bytes, stdin);

        dy_array_add_to_size(buffer, num_bytes_read);

        state->bound_size_in_bytes -= num_bytes_read;
    } else {
        dy_array_set_excess_capacity(buffer, 1);

        size_t num_bytes_read = fread(dy_array_excess_buffer(buffer), sizeof(char), 1, stdin);

        dy_array_add_to_size(buffer, num_bytes_read);
    }
}

void write_size_t(FILE *file, size_t x)
{
    dy_array_t buffer = dy_array_create(sizeof(char), DY_ALIGNOF(char), 4);

    for (;;) {
        char digit = (x % 10) + '0';

        dy_array_add(&buffer, &digit);

        x = x / 10;

        if (x == 0) {
            break;
        }
    }

    for (size_t i = buffer.num_elems; i-- > 0;) {
        fprintf(file, "%c", *(char *)dy_array_pos(&buffer, i));
    }

    dy_array_release(&buffer);
}

void set_file_to_binary(FILE *file)
{
#ifdef _WIN32
    assert(_setmode(_fileno(file), _O_BINARY) != -1);
#else
    (void)file;
#endif
}
