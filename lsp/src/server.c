/*
 * Copyright 2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <duality/lsp/server.h>

#include <duality/support/utf8_to_json.h>
#include <duality/support/json_to_utf8.h>
#include <duality/support/assert.h>

#ifdef _WIN32
#    include <io.h>
#    include <fcntl.h>
#endif

struct send_env {
    dy_array_t *buffer;
    FILE *file;
};

static void send_callback(dy_json_t message, void *env);
static void stream_callback(dy_array_t *buffer, void *env);
static void write_size_t(FILE *file, size_t x);
static void set_file_to_binary(FILE *file);

int dy_lsp_run_server(FILE *in, FILE *out)
{
    set_file_to_binary(in);
    set_file_to_binary(out);

    struct dy_lsp_stream_env recv_env = {
        .file = in,
        .is_bounded = false
    };

    struct send_env send_env = {
        .buffer = dy_array_create(dy_allocator_stdlib(), sizeof(char), 1024),
        .file = out
    };

    struct dy_stream stream = {
        .get_chars = stream_callback,
        .env = &recv_env,
        .buffer = dy_array_create(dy_allocator_stdlib(), sizeof(char), 1024),
        .current_index = 0
    };

    dy_lsp_ctx_t *ctx = dy_lsp_create(dy_allocator_stdlib(), send_callback, &send_env);

    for (;;) {
        if (!dy_lsp_process_message(ctx, &stream)) {
            int ret = dy_lsp_exit_code(ctx);
            dy_lsp_destroy(ctx);
            return ret;
        }
    }
}

bool dy_lsp_process_message(dy_lsp_ctx_t *ctx, struct dy_stream *stream)
{
    struct dy_lsp_stream_env *env = stream->env;

    env->is_bounded = false;

    size_t content_length_in_bytes;
    if (!dy_lsp_read_content_length(stream, &content_length_in_bytes)) {
        return false;
    }

    if (!dy_stream_parse_literal(stream, DY_STR_LIT("\r\n"))) {
        return false;
    }

    env->is_bounded = true;
    env->bound_size_in_bytes = content_length_in_bytes;

    dy_json_t message;
    if (!dy_utf8_to_json(stream, &message)) {
        return false;
    }

    dy_stream_reset(stream);

    return dy_lsp_handle_message(ctx, message);
}

bool dy_lsp_read_content_length(struct dy_stream *stream, size_t *content_length_in_bytes)
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

void send_callback(dy_json_t message, void *env)
{
    struct send_env *send_env = env;

    size_t size = dy_array_size(send_env->buffer);

    dy_json_to_utf8(message, send_env->buffer);

    fprintf(send_env->file, "Content-Length:");
    write_size_t(send_env->file, dy_array_size(send_env->buffer) - size);
    fprintf(send_env->file, "\r\n\r\n");

    fwrite((char *)dy_array_buffer(send_env->buffer) + size, sizeof(char), dy_array_size(send_env->buffer) - size, send_env->file);

    fflush(send_env->file);

    dy_array_set_size(send_env->buffer, size);
}

void stream_callback(dy_array_t *buffer, void *env)
{
    struct dy_lsp_stream_env *state = env;

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
    dy_array_t *buffer = dy_array_create(dy_allocator_stdlib(), sizeof(char), 4);

    for (;;) {
        char digit = (x % 10) + '0';

        dy_array_add(buffer, &digit);

        x = x / 10;

        if (x == 0) {
            break;
        }
    }

    for (size_t i = dy_array_size(buffer); i-- > 0;) {
        fprintf(file, "%c", *(char *)dy_array_get_ptr(buffer, i));
    }

    dy_array_destroy(buffer);
}

void set_file_to_binary(FILE *file)
{
#ifdef _WIN32
    dy_assert(_setmode(_fileno(file), _O_BINARY) != -1);
#else
    (void)file;
#endif
}
