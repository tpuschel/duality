/*
 * Copyright 2019-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "lsp_server.h"

#include <duality/support/array.h>
#include <duality/support/string.h>
#include <duality/support/overflow.h>

#include <stdio.h>

#include "utf8_to_json.h"
#include "json_to_utf8.h"

#include <duality/syntax/parser.h>
#include <duality/syntax/ast_to_core.h>

#include <duality/core/type_of.h>

#include <duality/support/assert.h>

#ifdef _WIN32
#    include <io.h>
#    include <fcntl.h>
#endif

struct dy_stream_state {
    bool is_bounded;
    size_t bound_size_in_bytes;
};

struct document {
    dy_string_t uri;
    dy_string_t text;
    size_t running_id;
    struct dy_ast_do_block ast;
    bool ast_is_valid;
    struct dy_text_range_core_map core;
    bool core_is_valid;
};

struct lsp_state {
    bool is_initialized;
    bool received_exit_notification;
    bool received_shutdown_request;
    dy_json_t init_params;
    dy_array_t *open_documents;
};

struct mime_type {
    dy_string_t type;
    dy_string_t subtype;
    dy_string_t parameter_name;
    dy_string_t parameter_value;
    bool have_parameter;
};

static bool parse_content_length(struct dy_stream *stream, size_t *content_length_in_bytes);
static bool parse_content_type(struct dy_stream *stream, struct mime_type *mime_type);
static bool parse_mime_type(struct dy_stream *stream, struct mime_type *mime_type);
static bool parse_mime_type_name(struct dy_stream *stream, dy_string_t *name);

static void stdin_get_chars(dy_array_t *buffer, void *env);

static bool force_json_object(dy_json_t json, struct dy_json_object *object);
static bool force_json_string(dy_json_t json, dy_string_t *string);
static bool force_json_array(dy_json_t json, struct dy_json_array *array);
static bool force_json_integer(dy_json_t json, long *integer);
static bool get_json_member(struct dy_json_object obj, dy_string_t member_name, dy_json_t *value);

static void handle_msg(struct lsp_state *state, dy_json_t message, dy_array_t *response_buffer);

static dy_json_t invalid_request(void);
static dy_json_t init_result(dy_json_t id);
static dy_json_t server_capabilities(void);
static dy_json_t create_response(dy_json_t id, dy_json_t result);
static dy_json_t create_error_response(dy_json_t id, long code, dy_string_t message);

static dy_json_t json_true(void);
static dy_json_t json_false(void);
static dy_json_t json_null(void);
static dy_json_t json_integer(long x);
static dy_json_t json_string(dy_string_t s);

static void write_size_t(FILE *stream, size_t x);

static void write_json_with_header(FILE *stream, dy_json_t json, dy_array_t *response_buffer);

static void process_text(struct document *doc);

static bool compute_byte_offset(dy_string_t text, long line_offset, long utf16_offset, size_t *byte_offset);

static bool type_of_expr_at_byte_offset(size_t *running_id, struct dy_text_range_core_map map, size_t byte_offset, struct dy_core_expr *type);

static dy_json_t create_hover_response(dy_json_t id, dy_string_t type);

static void null_stream(dy_array_t *buffer, void *env);
struct dy_stream stream_from_string(struct dy_allocator allocator, dy_string_t s);

static void set_io_to_binary(void);

int run_lsp_server(void)
{
    set_io_to_binary();

    struct dy_stream_state stream_state = {
        .is_bounded = false
    };

    struct lsp_state lsp_state = {
        .is_initialized = false,
        .received_exit_notification = false,
        .received_shutdown_request = false,
        .open_documents = dy_array_create(dy_allocator_stdlib(), sizeof(struct document), 8)
    };

    struct dy_stream stream = {
        .get_chars = stdin_get_chars,
        .env = &stream_state,
        .buffer = dy_array_create(dy_allocator_stdlib(), sizeof(char), 1024),
        .current_index = 0
    };

    dy_array_t *response_buffer = dy_array_create(dy_allocator_stdlib(), sizeof(char), 1024);

    for (;;) {
        fprintf(stderr, "Waiting for message...\n");

        size_t content_length_in_bytes;
        if (!parse_content_length(&stream, &content_length_in_bytes)) {
            dy_stream_dump(&stream, stderr);
            return -1;
        }

        struct mime_type mime_type;
        if (parse_content_type(&stream, &mime_type)) {
            fprintf(stderr, "Received a mime type.\n");
        }

        if (!dy_stream_parse_literal(&stream, DY_STR_LIT("\r\n"))) {
            fprintf(stderr, "No header CRLF.\n");
            dy_stream_dump(&stream, stderr);
            return -1;
        }

        stream_state.is_bounded = true;
        stream_state.bound_size_in_bytes = content_length_in_bytes;

        dy_json_t message;
        if (!utf8_to_json(&stream, &message)) {
            fprintf(stderr, "Parsing fail\n");
            dy_stream_dump(&stream, stderr);
            return -1;
        }

        stream_state.is_bounded = false;

        handle_msg(&lsp_state, message, response_buffer);

        if (lsp_state.received_exit_notification) {
            if (lsp_state.received_shutdown_request) {
                return 0;
            } else {
                return -1;
            }
        }
    }
}

void write_size_t(FILE *stream, size_t x)
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
        fprintf(stream, "%c", *(char *)dy_array_get_ptr(buffer, i));
    }

    dy_array_destroy(buffer);
}

void write_json_with_header(FILE *stream, dy_json_t json, dy_array_t *response_buffer)
{
    size_t size = dy_array_size(response_buffer);

    json_to_utf8(json, response_buffer);

    fprintf(stream, "Content-Length:");
    write_size_t(stream, dy_array_size(response_buffer) - size);
    fprintf(stream, "\r\n\r\n");

    fwrite((char *)dy_array_buffer(response_buffer) + size, sizeof(char), dy_array_size(response_buffer) - size, stream);

    fflush(stream);

    dy_array_set_size(response_buffer, size);
}

void handle_msg(struct lsp_state *state, dy_json_t message, dy_array_t *response_buffer)
{
    struct dy_json_object obj;
    if (!force_json_object(message, &obj)) {
        write_json_with_header(stdout, invalid_request(), response_buffer);
        return;
    }

    dy_json_t id;
    bool have_id = get_json_member(obj, DY_STR_LIT("id"), &id);

    dy_json_t method;
    if (!get_json_member(obj, DY_STR_LIT("method"), &method)) {
        write_json_with_header(stdout, invalid_request(), response_buffer);
        return;
    }

    dy_string_t method_str;
    if (!force_json_string(method, &method_str)) {
        write_json_with_header(stdout, invalid_request(), response_buffer);
        return;
    }

    if (dy_string_are_equal(method_str, DY_STR_LIT("initialize"))) {
        fprintf(stderr, "--> initialize\n");

        if (!have_id) {
            write_json_with_header(stdout, invalid_request(), response_buffer);
            return;
        }

        if (state->is_initialized) {
            write_json_with_header(stdout, invalid_request(), response_buffer);
            return;
        }

        if (!get_json_member(obj, DY_STR_LIT("params"), &state->init_params)) {
            write_json_with_header(stdout, invalid_request(), response_buffer);
            return;
        }

        state->is_initialized = true;

        write_json_with_header(stdout, init_result(id), response_buffer);

        return;
    }

    if (dy_string_are_equal(method_str, DY_STR_LIT("exit"))) {
        state->received_exit_notification = true;
        fprintf(stderr, "--> exit\n");
        return;
    }

    if (dy_string_are_equal(method_str, DY_STR_LIT("initialized"))) {
        fprintf(stderr, "--> initialized\n");
        return;
    }

    if (dy_string_are_equal(method_str, DY_STR_LIT("shutdown"))) {
        fprintf(stderr, "--> shutdown\n");

        if (!have_id) {
            write_json_with_header(stdout, invalid_request(), response_buffer);
            return;
        }

        state->received_shutdown_request = true;

        write_json_with_header(stdout, create_response(id, json_null()), response_buffer);

        return;
    }

    if (dy_string_are_equal(method_str, DY_STR_LIT("textDocument/didOpen"))) {
        fprintf(stderr, "--> textDocument/didOpen\n");

        dy_json_t params;
        if (!get_json_member(obj, DY_STR_LIT("params"), &params)) {
            return;
        }

        struct dy_json_object param_obj;
        if (!force_json_object(params, &param_obj)) {
            return;
        }

        dy_json_t text_document;
        if (!get_json_member(param_obj, DY_STR_LIT("textDocument"), &text_document)) {
            return;
        }

        struct dy_json_object text_document_obj;
        if (!force_json_object(text_document, &text_document_obj)) {
            return;
        }

        dy_json_t uri;
        if (!get_json_member(text_document_obj, DY_STR_LIT("uri"), &uri)) {
            return;
        }

        dy_string_t uri_string;
        if (!force_json_string(uri, &uri_string)) {
            return;
        }

        dy_json_t text;
        if (!get_json_member(text_document_obj, DY_STR_LIT("text"), &text)) {
            return;
        }

        dy_string_t text_string;
        if (!force_json_string(text, &text_string)) {
            return;
        }

        struct document doc = {
            .uri = uri_string,
            .running_id = 0,
            .text = text_string
        };

        process_text(&doc);

        dy_array_add(state->open_documents, &doc);

        return;
    }

    if (dy_string_are_equal(method_str, DY_STR_LIT("textDocument/didClose"))) {
        fprintf(stderr, "--> textDocument/didClose\n");

        dy_json_t params;
        if (!get_json_member(obj, DY_STR_LIT("params"), &params)) {
            return;
        }

        struct dy_json_object params_obj;
        if (!force_json_object(params, &params_obj)) {
            return;
        }

        dy_json_t text_document;
        if (!get_json_member(params_obj, DY_STR_LIT("textDocument"), &text_document)) {
            return;
        }

        struct dy_json_object text_document_obj;
        if (!force_json_object(text_document, &text_document_obj)) {
            return;
        }

        dy_json_t uri;
        if (!get_json_member(text_document_obj, DY_STR_LIT("uri"), &uri)) {
            return;
        }

        dy_string_t uri_string;
        if (!force_json_string(uri, &uri_string)) {
            return;
        }

        for (size_t i = 0, size = dy_array_size(state->open_documents); i < size; ++i) {
            struct document doc;
            dy_array_get(state->open_documents, i, &doc);

            if (dy_string_are_equal(doc.uri, uri_string)) {
                dy_array_remove(state->open_documents, i);
                break;
            }
        }

        return;
    }

    if (dy_string_are_equal(method_str, DY_STR_LIT("textDocument/didChange"))) {
        fprintf(stderr, "--> textDocument/didChange\n");

        dy_json_t params;
        if (!get_json_member(obj, DY_STR_LIT("params"), &params)) {
            return;
        }

        struct dy_json_object params_obj;
        if (!force_json_object(params, &params_obj)) {
            return;
        }

        dy_json_t text_document;
        if (!get_json_member(params_obj, DY_STR_LIT("textDocument"), &text_document)) {
            return;
        }

        struct dy_json_object text_document_obj;
        if (!force_json_object(text_document, &text_document_obj)) {
            return;
        }

        dy_json_t uri;
        if (!get_json_member(text_document_obj, DY_STR_LIT("uri"), &uri)) {
            return;
        }

        dy_string_t uri_string;
        if (!force_json_string(uri, &uri_string)) {
            return;
        }

        dy_json_t content_changes;
        if (!get_json_member(params_obj, DY_STR_LIT("contentChanges"), &content_changes)) {
            return;
        }

        struct dy_json_array content_changes_array;
        if (!force_json_array(content_changes, &content_changes_array)) {
            return;
        }

        for (size_t i = 0, size = dy_array_size(state->open_documents); i < size; ++i) {
            struct document *doc = dy_array_get_ptr(state->open_documents, i);

            if (!dy_string_are_equal(doc->uri, uri_string)) {
                continue;
            }

            for (size_t k = 0; k < content_changes_array.num_values; ++k) {
                dy_json_t change = content_changes_array.values[k];

                struct dy_json_object change_obj;
                if (!force_json_object(change, &change_obj)) {
                    return;
                }

                dy_json_t text;
                if (!get_json_member(change_obj, DY_STR_LIT("text"), &text)) {
                    return;
                }

                dy_string_t text_string;
                if (!force_json_string(text, &text_string)) {
                    return;
                }

                doc->text = text_string;

                process_text(doc);
            }

            break;
        }

        return;
    }

    if (dy_string_are_equal(method_str, DY_STR_LIT("textDocument/hover"))) {
        fprintf(stderr, "--> textDocument/hover\n");

        if (!have_id) {
            write_json_with_header(stdout, invalid_request(), response_buffer);
            return;
        }

        dy_json_t params;
        if (!get_json_member(obj, DY_STR_LIT("params"), &params)) {
            return;
        }

        struct dy_json_object params_obj;
        if (!force_json_object(params, &params_obj)) {
            return;
        }

        dy_json_t text_document;
        if (!get_json_member(params_obj, DY_STR_LIT("textDocument"), &text_document)) {
            return;
        }

        struct dy_json_object text_document_obj;
        if (!force_json_object(text_document, &text_document_obj)) {
            return;
        }

        dy_json_t uri;
        if (!get_json_member(text_document_obj, DY_STR_LIT("uri"), &uri)) {
            return;
        }

        dy_string_t uri_string;
        if (!force_json_string(uri, &uri_string)) {
            return;
        }

        dy_json_t position;
        if (!get_json_member(params_obj, DY_STR_LIT("position"), &position)) {
            return;
        }

        struct dy_json_object position_obj;
        if (!force_json_object(position, &position_obj)) {
            return;
        }

        dy_json_t line;
        if (!get_json_member(position_obj, DY_STR_LIT("line"), &line)) {
            return;
        }

        long line_number;
        if (!force_json_integer(line, &line_number)) {
            return;
        }

        dy_json_t character;
        if (!get_json_member(position_obj, DY_STR_LIT("character"), &character)) {
            return;
        }

        long character_number;
        if (!force_json_integer(character, &character_number)) {
            return;
        }

        for (size_t i = 0, size = dy_array_size(state->open_documents); i < size; ++i) {
            struct document *doc = dy_array_get_ptr(state->open_documents, i);

            if (!dy_string_are_equal(doc->uri, uri_string)) {
                continue;
            }

            if (doc->core_is_valid) {
                size_t byte_offset;
                if (!compute_byte_offset(doc->text, line_number, character_number, &byte_offset)) {
                    fprintf(stderr, "Invalid offsets\n");
                    write_json_with_header(stdout, create_response(id, json_null()), response_buffer);
                    return;
                }

                struct dy_core_expr type;
                if (!type_of_expr_at_byte_offset(&doc->running_id, doc->core, byte_offset, &type)) {
                    fprintf(stderr, "Failed to compute type\n");
                    write_json_with_header(stdout, create_response(id, json_null()), response_buffer);
                    return;
                }

                dy_array_t *type_string_storage = dy_array_create(dy_allocator_stdlib(), sizeof(char), 64);
                dy_core_expr_to_string(type, type_string_storage);

                dy_string_t type_string = {
                    .ptr = dy_array_buffer(type_string_storage),
                    .size = dy_array_size(type_string_storage)
                };

                write_json_with_header(stdout, create_hover_response(id, type_string), response_buffer);

                return;
            }

            break;
        }

        write_json_with_header(stdout, create_response(id, json_null()), response_buffer);

        return;
    }

    fprintf(stderr, "Unhandled method '");
    for (size_t i = 0; i < method_str.size; ++i) {
        fprintf(stderr, "%c", method_str.ptr[i]);
    }
    fprintf(stderr, "'\n");

    if (have_id) {
        dy_json_t response = create_error_response(id, -32601, DY_STR_LIT("Can't handle that method :/"));

        write_json_with_header(stdout, response, response_buffer);

        return;
    }

    return;
}

dy_json_t invalid_request()
{
    return create_error_response(json_null(), -32600, DY_STR_LIT("Invalid request, sorry"));
}

dy_json_t create_error_response(dy_json_t id, long code, dy_string_t message)
{
    struct dy_json_value json_code = {
        .tag = DY_JSON_VALUE_NUMBER,
        .number = {
            .tag = DY_JSON_NUMBER_INTEGER,
            .integer = code,
        }
    };

    struct dy_json_value json_message = {
        .tag = DY_JSON_VALUE_STRING,
        .string = message
    };

    dy_array_t *error_members = dy_array_create(dy_allocator_stdlib(), sizeof(struct dy_json_member), 2);
    dy_array_add(error_members, &(struct dy_json_member){ .string = DY_STR_LIT("code"), .value = json_code });
    dy_array_add(error_members, &(struct dy_json_member){ .string = DY_STR_LIT("message"), .value = json_message });

    struct dy_json_value error = {
        .tag = DY_JSON_VALUE_OBJECT,
        .object = {
            .members = dy_array_buffer(error_members),
            .num_members = dy_array_size(error_members),
        }
    };

    dy_array_t *members = dy_array_create(dy_allocator_stdlib(), sizeof(struct dy_json_member), 2);
    dy_array_add(members, &(struct dy_json_member){ .string = DY_STR_LIT("id"), .value = id });
    dy_array_add(members, &(struct dy_json_member){ .string = DY_STR_LIT("error"), .value = error });

    return (dy_json_t){
        .tag = DY_JSON_VALUE_OBJECT,
        .object = {
            .members = dy_array_buffer(members),
            .num_members = dy_array_size(members),
        }
    };
}

dy_json_t init_result(dy_json_t id)
{
    dy_array_t *init_res = dy_array_create(dy_allocator_stdlib(), sizeof(struct dy_json_member), 1);
    dy_array_add(init_res, &(struct dy_json_member){ .string = DY_STR_LIT("capabilities"), .value = server_capabilities() });

    dy_json_t result = {
        .tag = DY_JSON_VALUE_OBJECT,
        .object = {
            .members = dy_array_buffer(init_res),
            .num_members = dy_array_size(init_res),
        }
    };

    return create_response(id, result);
}

dy_json_t server_capabilities(void)
{
    dy_array_t *text_doc_sync_options = dy_array_create(dy_allocator_stdlib(), sizeof(struct dy_json_member), 5);
    dy_array_add(text_doc_sync_options, &(struct dy_json_member){ .string = DY_STR_LIT("openClose"), .value = json_true() });
    dy_array_add(text_doc_sync_options, &(struct dy_json_member){ .string = DY_STR_LIT("change"), .value = json_integer(1) });

    dy_json_t text_doc_sync_options_obj = {
        .tag = DY_JSON_VALUE_OBJECT,
        .object = {
            .members = dy_array_buffer(text_doc_sync_options),
            .num_members = dy_array_size(text_doc_sync_options),
        }
    };

    dy_array_t *caps = dy_array_create(dy_allocator_stdlib(), sizeof(struct dy_json_member), 2);
    dy_array_add(caps, &(struct dy_json_member){ .string = DY_STR_LIT("textDocumentSync"), .value = text_doc_sync_options_obj });
    dy_array_add(caps, &(struct dy_json_member){ .string = DY_STR_LIT("hoverProvider"), .value = json_true() });

    return (dy_json_t){
        .tag = DY_JSON_VALUE_OBJECT,
        .object = {
            .members = dy_array_buffer(caps),
            .num_members = dy_array_size(caps),
        }
    };
}

dy_json_t create_response(dy_json_t id, dy_json_t result)
{
    dy_array_t *response = dy_array_create(dy_allocator_stdlib(), sizeof(struct dy_json_member), 3);
    dy_array_add(response, &(struct dy_json_member){ .string = DY_STR_LIT("jsonrpc"), .value = json_string(DY_STR_LIT("2.0")) });
    dy_array_add(response, &(struct dy_json_member){ .string = DY_STR_LIT("id"), .value = id });
    dy_array_add(response, &(struct dy_json_member){ .string = DY_STR_LIT("result"), .value = result });

    return (dy_json_t){
        .tag = DY_JSON_VALUE_OBJECT,
        .object = {
            .members = dy_array_buffer(response),
            .num_members = dy_array_size(response),
        }
    };
}

dy_json_t create_hover_response(dy_json_t id, dy_string_t type)
{
    dy_array_t *hover_res = dy_array_create(dy_allocator_stdlib(), sizeof(struct dy_json_member), 1);
    dy_array_add(hover_res, &(struct dy_json_member){ .string = DY_STR_LIT("contents"), .value = json_string(type) });

    dy_json_t result = {
        .tag = DY_JSON_VALUE_OBJECT,
        .object = {
            .members = dy_array_buffer(hover_res),
            .num_members = dy_array_size(hover_res),
        }
    };

    return create_response(id, result);
}

dy_json_t json_true(void)
{
    return (dy_json_t){
        .tag = DY_JSON_VALUE_TRUE
    };
}

dy_json_t json_false(void)
{
    return (dy_json_t){
        .tag = DY_JSON_VALUE_FALSE
    };
}

dy_json_t json_null(void)
{
    return (dy_json_t){
        .tag = DY_JSON_VALUE_NULL
    };
}

dy_json_t json_integer(long x)
{
    return (dy_json_t){
        .tag = DY_JSON_VALUE_NUMBER,
        .number = {
            .tag = DY_JSON_NUMBER_INTEGER,
            .integer = x,
        }
    };
}

dy_json_t json_string(dy_string_t s)
{
    return (dy_json_t){
        .tag = DY_JSON_VALUE_STRING,
        .string = s
    };
}

bool force_json_object(dy_json_t json, struct dy_json_object *object)
{
    if (json.tag != DY_JSON_VALUE_OBJECT) {
        fprintf(stderr, "Not an object %d\n", json.tag);
        return false;
    }

    *object = json.object;
    return true;
}

bool force_json_string(dy_json_t json, dy_string_t *string)
{
    if (json.tag != DY_JSON_VALUE_STRING) {
        fprintf(stderr, "Not a string %d\n", json.tag);
        return false;
    }

    *string = json.string;
    return true;
}

bool force_json_array(dy_json_t json, struct dy_json_array *array)
{
    if (json.tag != DY_JSON_VALUE_ARRAY) {
        fprintf(stderr, "Not an array %d\n", json.tag);
        return false;
    }

    *array = json.array;
    return true;
}

bool force_json_integer(dy_json_t json, long *integer)
{
    if (json.tag != DY_JSON_VALUE_NUMBER) {
        fprintf(stderr, "Not a number %d\n", json.tag);
        return false;
    }

    if (json.number.tag != DY_JSON_NUMBER_INTEGER) {
        fprintf(stderr, "Not an integer %d\n", json.tag);
        return false;
    }

    *integer = json.number.integer;
    return true;
}

bool get_json_member(struct dy_json_object obj, dy_string_t member_name, dy_json_t *value)
{
    for (size_t i = 0; i < obj.num_members; ++i) {
        if (dy_string_are_equal(obj.members[i].string, member_name)) {
            *value = obj.members[i].value;
            return true;
        }
    }

    return false;
}

void stdin_get_chars(dy_array_t *buffer, void *env)
{
    struct dy_stream_state *state = env;

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

void process_text(struct document *doc)
{
    struct dy_parser_ctx parser_ctx = {
        .stream = stream_from_string(dy_allocator_stdlib(), doc->text),
        .allocator = dy_allocator_stdlib(),
        .arrays = dy_array_create(dy_allocator_stdlib(), sizeof(dy_array_t *), 32)
    };

    struct dy_ast_do_block ast;
    if (!dy_parse_file(&parser_ctx, &ast)) {
        doc->ast_is_valid = false;
        doc->core_is_valid = false;

        return;
    }

    dy_array_t *unbound_vars = dy_array_create(dy_allocator_stdlib(), sizeof(dy_string_t), 2);
    size_t running_id = 0;
    struct dy_ast_to_core_ctx ast_to_core_ctx = {
        .running_id = &running_id,
        .bound_vars = dy_array_create(dy_allocator_stdlib(), sizeof(struct dy_ast_to_core_bound_var), 64),
        .allocator = dy_allocator_stdlib(),
        .unbound_vars = unbound_vars
    };

    struct dy_core_expr core;
    dy_array_t *sub_maps = dy_array_create(dy_allocator_stdlib(), sizeof(struct dy_text_range_core_map), 2);
    if (!dy_ast_do_block_to_core(&ast_to_core_ctx, ast, &core, sub_maps)) {
        doc->ast_is_valid = true;
        doc->ast = ast;
        doc->core_is_valid = false;

        return;
    }

    doc->ast_is_valid = true;
    doc->ast = ast;
    doc->core_is_valid = true;
    doc->core = (struct dy_text_range_core_map){
        .text_range = {
            .start = 0,
            .end = doc->text.size,
        },
        .expr = core,
        .sub_maps = sub_maps,
    };

    return;
}

bool compute_byte_offset(dy_string_t text, long line_offset, long utf16_offset, size_t *byte_offset)
{
    if (line_offset < 0 || utf16_offset < 0) {
        return false;
    }

    size_t cnt = 0;
    for (long i = 0; i < line_offset; ++i) {
        if (cnt >= text.size) {
            return false;
        }

        for (;;) {
            if (text.ptr[cnt] == '\n') {
                ++cnt;
                break;
            }

            ++cnt;
        }
    }

    for (long i = 0; i < utf16_offset; ++i) {
        if (cnt >= text.size) {
            return false;
        }

        if (text.ptr[cnt] == '\n') {
            --cnt;
            break;
        }

        // TODO: Breaks if the text is anything but 1-byte UTF-8 codepoints. Sadly we only get UTF-16 offsets :/
        ++cnt;
    }

    *byte_offset = cnt;

    return true;
}

bool type_of_expr_at_byte_offset(size_t *running_id, struct dy_text_range_core_map map, size_t byte_offset, struct dy_core_expr *type)
{
    if (map.text_range.start > byte_offset || map.text_range.end < byte_offset) {
        return false;
    }

    for (size_t i = 0, size = dy_array_size(map.sub_maps); i < size; ++i) {
        struct dy_text_range_core_map sub_map;
        dy_array_get(map.sub_maps, i, &sub_map);

        if (type_of_expr_at_byte_offset(running_id, sub_map, byte_offset, type)) {
            return true;
        }
    }

    struct dy_check_ctx check_ctx = {
        .running_id = running_id,
        .allocator = dy_allocator_stdlib(),
        .successful_elims = dy_array_create(dy_allocator_stdlib(), sizeof(size_t), 32)
    };

    *type = dy_type_of(check_ctx, map.expr);

    return true;
}

bool parse_content_length(struct dy_stream *stream, size_t *content_length_in_bytes)
{
    struct dy_stream saved_stream = *stream;

    if (!dy_stream_parse_literal(&saved_stream, DY_STR_LIT("Content-Length: "))) {
        fprintf(stderr, "Failed parsing 'Content-Length: '\n");
        return false;
    }

    size_t number;
    if (!dy_stream_parse_size_t_decimal(&saved_stream, &number)) {
        fprintf(stderr, "Failed to parse size decimal\n");
        return false;
    }

    if (!dy_stream_parse_literal(&saved_stream, DY_STR_LIT("\r\n"))) {
        fprintf(stderr, "Failed to parse closing CRLF\n");
        return false;
    }

    *stream = saved_stream;
    *content_length_in_bytes = number;

    return true;
}

bool parse_content_type(struct dy_stream *stream, struct mime_type *mime_type)
{
    struct dy_stream saved_stream = *stream;

    if (!dy_stream_parse_literal(&saved_stream, DY_STR_LIT("Content-Type: "))) {
        return false;
    }

    struct mime_type m;
    if (!parse_mime_type(&saved_stream, &m)) {
        return false;
    }

    if (!dy_stream_parse_literal(&saved_stream, DY_STR_LIT("\r\n"))) {
        return false;
    }

    *mime_type = m;
    *stream = saved_stream;
    return true;
}

bool parse_mime_type(struct dy_stream *stream, struct mime_type *mime_type)
{
    struct dy_stream saved_stream = *stream;

    dy_string_t type;
    if (!parse_mime_type_name(&saved_stream, &type)) {
        return false;
    }

    if (!dy_stream_parse_literal(&saved_stream, DY_STR_LIT("/"))) {
        return false;
    }

    dy_string_t subtype;
    if (!parse_mime_type_name(&saved_stream, &subtype)) {
        return false;
    }

    if (!dy_stream_parse_literal(&saved_stream, DY_STR_LIT(";"))) {
        *mime_type = (struct mime_type){
            .type = type,
            .subtype = subtype,
            .have_parameter = false
        };

        return true;
    }

    dy_string_t parameter_name;
    if (!parse_mime_type_name(&saved_stream, &parameter_name)) {
        return false;
    }

    if (!dy_stream_parse_literal(&saved_stream, DY_STR_LIT("="))) {
        return false;
    }

    dy_string_t parameter_value;
    if (!parse_mime_type_name(&saved_stream, &parameter_value)) {
        return false;
    }

    *mime_type = (struct mime_type){
        .type = type,
        .subtype = subtype,
        .have_parameter = true,
        .parameter_name = parameter_name,
        .parameter_value = parameter_value
    };

    return true;
}

bool parse_mime_type_name(struct dy_stream *stream, dy_string_t *name)
{
    struct dy_stream saved_stream = *stream;

    dy_array_t *string = dy_array_create(dy_allocator_stdlib(), sizeof(char), 8);

    char c;
    if (!dy_stream_get_char(&saved_stream, &c)) {
        return false;
    }

    if ((c < 'a' || 'z' < c) && (c < '0' || '9' < c) && (c < 'A' || 'Z' < c)) {
        return false;
    }

    dy_array_add(string, &c);

    for (;;) {
        if (!dy_stream_get_char(&saved_stream, &c)) {
            dy_stream_put_last_char_back(&saved_stream);
            break;
        }

        if ((c < 'a' || 'z' < c) && (c < '0' || '9' < c) && (c < 'A' || 'Z' < c) && c != '!' && c != '#' && c != '$' && c != '&' && c != '-' && c != '^' && c != '_' && c != '.' && c != '+') {
            dy_stream_put_last_char_back(&saved_stream);
            break;
        }
    }

    *stream = saved_stream;
    *name = (dy_string_t){
        .ptr = dy_array_buffer(string),
        .size = dy_array_size(string)
    };

    return true;
}

struct dy_stream stream_from_string(struct dy_allocator allocator, dy_string_t s)
{
    dy_array_t *buffer = dy_array_create(allocator, sizeof(char), s.size);
    for (size_t i = 0; i < s.size; ++i) {
        dy_array_add(buffer, s.ptr + i);
    }

    return (struct dy_stream){
        .get_chars = null_stream,
        .buffer = buffer,
        .env = NULL,
        .current_index = 0
    };
}

void null_stream(dy_array_t *buffer, void *env)
{
    (void)buffer;
    (void)env;
    return;
}

void set_io_to_binary(void)
{
#ifdef _WIN32
    dy_assert(_setmode(_fileno(stdin), _O_BINARY) != -1);
    dy_assert(_setmode(_fileno(stdout), _O_BINARY) != -1);
#endif
}
