/*
 * Copyright 2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "../support/json.h"
#include "../support/array.h"

#include "../core/check.h"

#include "../syntax/utf8_to_ast.h"
#include "../syntax/ast_to_core.h"

/**
 * This file implements LSP support.
 */

typedef struct dy_lsp_ctx dy_lsp_ctx_t;

typedef void (*dy_lsp_send_fn)(const uint8_t *message, void *env);

static inline dy_lsp_ctx_t *dy_lsp_create(dy_lsp_send_fn send, void *env);

static inline bool dy_lsp_handle_message(dy_lsp_ctx_t *ctx, const uint8_t *message);

static inline void dy_lsp_initialize(const uint8_t *id, dy_lsp_ctx_t *ctx, dy_array_t *json);

static inline void dy_lsp_initialized(dy_lsp_ctx_t *ctx);

static inline void dy_lsp_shutdown(dy_lsp_ctx_t *ctx);

static inline void dy_lsp_did_open(dy_lsp_ctx_t *ctx, dy_string_t uri, dy_string_t text);

static inline void dy_lsp_did_close(dy_lsp_ctx_t *ctx, dy_string_t uri);

static inline void dy_lsp_did_change(dy_lsp_ctx_t *ctx, dy_string_t uri, const uint8_t *content_changes);

static inline void dy_lsp_hover(dy_lsp_ctx_t *ctx, const uint8_t *id, dy_string_t uri, long line_number, long utf16_char_offset, dy_array_t *json);

static inline void dy_lsp_exit(dy_lsp_ctx_t *ctx);

static inline int dy_lsp_exit_code(dy_lsp_ctx_t *ctx);

static inline void dy_lsp_destroy(dy_lsp_ctx_t *ctx);

/**
 * Represents an open document.
 */
struct document {
    dy_array_t uri; /** The URI is used as the identifier for a document. */
    dy_array_t text;
    struct dy_core_ctx core_ctx;
    struct dy_core_expr core;
    bool core_is_present;
};

struct dy_lsp_ctx {
    dy_array_t output_buffer;

    dy_lsp_send_fn send;
    void *env;

    bool is_initialized;
    bool received_shutdown_request;
    int exit_code;
    dy_array_t documents;
};

static inline void dy_lsp_send(dy_lsp_ctx_t *ctx);

static inline void invalid_request(const uint8_t *id, dy_string_t message, dy_array_t *json);
static inline void server_capabilities(dy_array_t *json);
static inline void server_info(dy_array_t *json);
static inline void text_document_sync_options(dy_array_t *json);
static inline void initialize_response(const uint8_t *id, dy_array_t *json);
static inline void method_not_found(const uint8_t *id, dy_string_t message, dy_array_t *json);
static inline void response_error(long code, dy_string_t message, dy_array_t *json);
static inline void null_success_response(const uint8_t *id, dy_array_t *json);
static inline void hover_result(dy_string_t contents, dy_array_t *json);
static inline void make_position(long line, long character, dy_array_t *json);
static inline void make_range(long start_line, long start_character, long end_line, long end_character, dy_array_t *json);
static inline void publish_diagnostics(struct dy_core_ctx *ctx, dy_string_t uri, const dy_array_t *text_sources, struct dy_core_expr expr, dy_string_t text, dy_array_t *json);
static inline void compute_lsp_range(dy_string_t text, struct dy_range range, dy_array_t *json);

static inline void process_document(struct dy_lsp_ctx *ctx, struct document *doc);
static inline void null_stream(dy_array_t *buffer, void *env);
static inline bool compute_byte_offset(dy_string_t text, long line_offset, long utf16_offset, size_t *byte_offset);
static inline bool produce_diagnostics(struct dy_core_ctx *ctx, const dy_array_t *text_sources, struct dy_core_expr expr, dy_string_t text, dy_array_t *json);
static inline bool produce_solution_diagnostics(struct dy_core_ctx *ctx, const dy_array_t *text_sources, struct dy_core_solution solution, dy_string_t text, dy_array_t *json);
static inline void compute_lsp_range(dy_string_t text, struct dy_range range, dy_array_t *json);
static inline void make_diagnostic(dy_string_t text, struct dy_range range, long severity, dy_string_t message, dy_array_t *json);
static inline void diagnostics_params(struct dy_core_ctx *ctx, dy_string_t uri, const dy_array_t *text_sources, struct dy_core_expr expr, dy_string_t text, dy_array_t *json);
static inline const struct dy_range *get_text_range(const dy_array_t *text_sources, size_t id);

static inline dy_string_t convert_json_string(const uint8_t *p);
static inline void put_string_literal(dy_string_t s, dy_array_t *json);
static inline const uint8_t *copy_json_value(const uint8_t *src, dy_array_t *dst);
static inline const uint8_t *copy_json_string(const uint8_t *src, dy_array_t *dst);
static inline const uint8_t *skip_json_value(const uint8_t *json);
static inline void put_number(long x, dy_array_t *json);
static inline const uint8_t *skip_json_string(const uint8_t *p);
static inline const uint8_t *json_get_member(const uint8_t *p, dy_string_t member);

static inline dy_string_t array_view(const dy_array_t *x);
static inline dy_array_t view_to_array(dy_string_t s);
static inline void replace_storage_with_view(dy_array_t *x, dy_string_t s);

dy_lsp_ctx_t *dy_lsp_create(dy_lsp_send_fn send, void *env)
{
    struct dy_lsp_ctx *ctx = dy_rc_alloc(sizeof *ctx, DY_ALIGNOF(*ctx));
    *ctx = (struct dy_lsp_ctx){
        .output_buffer = dy_array_create(1, 1, 128),
        .send = send,
        .env = env,
        .is_initialized = false,
        .received_shutdown_request = false,
        .exit_code = 1, // Error by default.
        .documents = dy_array_create(sizeof(struct document), DY_ALIGNOF(struct document), 8)
    };

    return ctx;
}

void dy_lsp_destroy(dy_lsp_ctx_t *ctx)
{
    dy_array_release(&ctx->documents);
    dy_array_release(&ctx->output_buffer);
    dy_rc_release(ctx, DY_ALIGNOF(*ctx));
}

bool dy_lsp_handle_message(dy_lsp_ctx_t *ctx, const uint8_t *message)
{
    if (*message != DY_JSON_OBJECT) {
        return true;
    }

    message++;

    const uint8_t *id = json_get_member(message, DY_STR_LIT("id"));

    const uint8_t *method = json_get_member(message, DY_STR_LIT("method"));
    if (method == NULL) {
        if (id != NULL) {
            invalid_request(id, DY_STR_LIT("Missing 'method' field."), &ctx->output_buffer);
            dy_lsp_send(ctx);
        }

        return true;
    }

    const uint8_t *params = json_get_member(message, DY_STR_LIT("params"));

    if (*method != DY_JSON_STRING) {
        if (id != NULL) {
            invalid_request(id, DY_STR_LIT("'method' field is not a string."), &ctx->output_buffer);
            dy_lsp_send(ctx);
        }

        return true;
    }

    method++;

    dy_string_t method_string = convert_json_string(method);

    if (dy_string_are_equal(method_string, DY_STR_LIT("exit"))) {
        dy_lsp_exit(ctx);
        return false;
    }

    if (ctx->received_shutdown_request) {
        if (id != NULL) {
            invalid_request(id, DY_STR_LIT("Non-exit request after shutdown."), &ctx->output_buffer);
            dy_lsp_send(ctx);
        }

        return true;
    }

    if (dy_string_are_equal(method_string, DY_STR_LIT("initialize"))) {
        if (id == NULL) {
            return true;
        }

        if (params == NULL) {
            invalid_request(id, DY_STR_LIT("Missing 'params' field."), &ctx->output_buffer);
            dy_lsp_send(ctx);
            return true;
        }

        dy_lsp_initialize(id, ctx, &ctx->output_buffer);

        dy_lsp_send(ctx);

        return true;
    }

    if (!ctx->is_initialized) {
        if (id != NULL) {
            dy_array_add(&ctx->output_buffer, &(uint8_t){ DY_JSON_OBJECT });

            put_string_literal(DY_STR_LIT("jsonrpc"), &ctx->output_buffer);
            put_string_literal(DY_STR_LIT("2.0"), &ctx->output_buffer);

            put_string_literal(DY_STR_LIT("id"), &ctx->output_buffer);
            copy_json_value(id, &ctx->output_buffer);

            put_string_literal(DY_STR_LIT("error"), &ctx->output_buffer);
            response_error(-32002, DY_STR_LIT("Not yet initialized."), &ctx->output_buffer);

            dy_array_add(&ctx->output_buffer, &(uint8_t){ DY_JSON_END });

            dy_lsp_send(ctx);
        }

        return true;
    }

    if (dy_string_are_equal(method_string, DY_STR_LIT("shutdown"))) {
        if (id == NULL) {
            return true;
        }

        dy_lsp_shutdown(ctx);

        null_success_response(id, &ctx->output_buffer);
        dy_lsp_send(ctx);

        return true;
    }

    if (dy_string_are_equal(method_string, DY_STR_LIT("initialized"))) {
        dy_lsp_initialized(ctx);
        return true;
    }

    if (dy_string_are_equal(method_string, DY_STR_LIT("textDocument/didOpen"))) {
        if (params == NULL) {
            return true;
        }
        if (*params != DY_JSON_OBJECT) {
            return true;
        }
        ++params;

        const uint8_t *text_document = json_get_member(params, DY_STR_LIT("textDocument"));
        if (text_document == NULL) {
            return true;
        }
        if (*text_document != DY_JSON_OBJECT) {
            return true;
        }
        ++text_document;

        const uint8_t *uri = json_get_member(text_document, DY_STR_LIT("uri"));
        if (uri == NULL) {
            return true;
        }
        if (*uri != DY_JSON_STRING) {
            return true;
        }
        ++uri;
        dy_string_t uri_string = convert_json_string(uri);

        const uint8_t *text = json_get_member(text_document, DY_STR_LIT("text"));
        if (text == NULL) {
            return true;
        }
        if (*text != DY_JSON_STRING) {
            return true;
        }
        ++text;
        dy_string_t text_string = convert_json_string(text);

        dy_lsp_did_open(ctx, uri_string, text_string);

        return true;
    }

    if (dy_string_are_equal(method_string, DY_STR_LIT("textDocument/didChange"))) {
        if (params == NULL) {
            return true;
        }
        if (*params != DY_JSON_OBJECT) {
            return true;
        }
        ++params;

        const uint8_t *text_document = json_get_member(params, DY_STR_LIT("textDocument"));
        if (text_document == NULL) {
            return true;
        }
        if (*text_document != DY_JSON_OBJECT) {
            return true;
        }
        ++text_document;

        const uint8_t *uri = json_get_member(text_document, DY_STR_LIT("uri"));
        if (uri == NULL) {
            return true;
        }
        if (*uri != DY_JSON_STRING) {
            return true;
        }
        ++uri;
        dy_string_t uri_string = convert_json_string(uri);

        const uint8_t *content_changes = json_get_member(params, DY_STR_LIT("contentChanges"));
        if (content_changes == NULL) {
            return true;
        }
        if (*content_changes != DY_JSON_ARRAY) {
            return true;
        }
        ++content_changes;

        dy_lsp_did_change(ctx, uri_string, content_changes);

        return true;
    }

    if (dy_string_are_equal(method_string, DY_STR_LIT("textDocument/didClose"))) {
        if (params == NULL) {
            return true;
        }
        if (*params != DY_JSON_OBJECT) {
            return true;
        }
        ++params;

        const uint8_t *text_document = json_get_member(params, DY_STR_LIT("textDocument"));
        if (text_document == NULL) {
            return true;
        }
        if (*text_document != DY_JSON_OBJECT) {
            return true;
        }
        ++text_document;

        const uint8_t *uri = json_get_member(text_document, DY_STR_LIT("uri"));
        if (uri == NULL)  {
            return true;
        }
        if (*uri != DY_JSON_STRING) {
            return true;
        }
        ++uri;

        dy_string_t uri_string = convert_json_string(uri);

        dy_lsp_did_close(ctx, uri_string);

        return true;
    }

    if (dy_string_are_equal(method_string, DY_STR_LIT("textDocument/hover"))) {
        if (id == NULL) {
            return true;
        }

        if (params == NULL) {
            invalid_request(id, DY_STR_LIT("Missing 'params' field."), &ctx->output_buffer);
            dy_lsp_send(ctx);
            return true;
        }
        if (*params != DY_JSON_OBJECT) {
            invalid_request(id, DY_STR_LIT("'params' is not an object."), &ctx->output_buffer);
            dy_lsp_send(ctx);
            return true;
        }
        ++params;

        const uint8_t *text_document = json_get_member(params, DY_STR_LIT("textDocument"));
        if (text_document == NULL) {
            invalid_request(id, DY_STR_LIT("Missing 'textDocument' field."), &ctx->output_buffer);
            dy_lsp_send(ctx);
            return true;
        }

        if (*text_document != DY_JSON_OBJECT) {
            invalid_request(id, DY_STR_LIT("'textDocument' is not an object."), &ctx->output_buffer);
            dy_lsp_send(ctx);
            return true;
        }
        ++text_document;

        const uint8_t *uri = json_get_member(text_document, DY_STR_LIT("uri"));
        if (uri == NULL) {
            invalid_request(id, DY_STR_LIT("Missing 'uri' field."), &ctx->output_buffer);
            dy_lsp_send(ctx);
            return true;
        }
        if (*uri != DY_JSON_STRING) {
            invalid_request(id, DY_STR_LIT("'uri' is not a string."), &ctx->output_buffer);
            dy_lsp_send(ctx);
            return true;
        }
        ++uri;
        dy_string_t uri_string = convert_json_string(uri);

        const uint8_t *position = json_get_member(params, DY_STR_LIT("position"));
        if (position == NULL) {
            invalid_request(id, DY_STR_LIT("Missing 'position' field."), &ctx->output_buffer);
            dy_lsp_send(ctx);
            return true;
        }
        if (*position != DY_JSON_OBJECT) {
            invalid_request(id, DY_STR_LIT("'position' is not an object."), &ctx->output_buffer);
            dy_lsp_send(ctx);
            return true;
        }
        ++position;

        const uint8_t *line = json_get_member(position, DY_STR_LIT("line"));
        if (line == NULL) {
            invalid_request(id, DY_STR_LIT("Missing 'line' field."), &ctx->output_buffer);
            dy_lsp_send(ctx);
            return true;
        }
        if (*line != DY_JSON_NUMBER) {
            invalid_request(id, DY_STR_LIT("'line' is not a number."), &ctx->output_buffer);
            dy_lsp_send(ctx);
            return true;
        }
        ++line;

        long line_number;
        memcpy(&line_number, line, sizeof(line_number));

        const uint8_t *character = json_get_member(position, DY_STR_LIT("character"));
        if (character == NULL) {
            invalid_request(id, DY_STR_LIT("Missing 'character' field."), &ctx->output_buffer);
            dy_lsp_send(ctx);
            return true;
        }
        if (*character != DY_JSON_NUMBER) {
            invalid_request(id, DY_STR_LIT("'character' is not a number."), &ctx->output_buffer);
            dy_lsp_send(ctx);
            return true;
        }
        ++character;
        long character_number;
        memcpy(&character_number, character, sizeof(character_number));

        dy_lsp_hover(ctx, id, uri_string, line_number, character_number, &ctx->output_buffer);

        dy_lsp_send(ctx);

        return true;
    }

    if (id != NULL) {
        method_not_found(id, DY_STR_LIT("Unknown method name."), &ctx->output_buffer);
        dy_lsp_send(ctx);
    }

    return true;
}

void dy_lsp_initialize(const uint8_t *id, dy_lsp_ctx_t *ctx, dy_array_t *json)
{
    if (ctx->is_initialized) {
        invalid_request(id, DY_STR_LIT("Already initialized."), json);
    } else {
        ctx->is_initialized = true;
        initialize_response(id, json);
    }
}

void dy_lsp_initialized(dy_lsp_ctx_t *ctx)
{
    (void)ctx;
    return;
}

void dy_lsp_shutdown(dy_lsp_ctx_t *ctx)
{
    ctx->received_shutdown_request = true;
}

void dy_lsp_did_open(dy_lsp_ctx_t *ctx, dy_string_t uri, dy_string_t text)
{
    struct document doc = {
        .uri = view_to_array(uri),
        .text = view_to_array(text),
        .core_ctx = {
            .running_id = 0,
            .captured_inference_vars = dy_array_create(sizeof(struct dy_captured_inference_var), DY_ALIGNOF(struct dy_captured_inference_var), 1),
            .recovered_negative_inference_ids = dy_array_create(sizeof(size_t), DY_ALIGNOF(size_t), 8),
            .past_subtype_checks = dy_array_create(sizeof(struct dy_core_past_subtype_check), DY_ALIGNOF(struct dy_core_past_subtype_check), 64),
            .free_variables = dy_array_create(sizeof(struct dy_free_var), DY_ALIGNOF(struct dy_free_var), 64),
            .equal_variables = dy_array_create(sizeof(struct dy_equal_variables), DY_ALIGNOF(struct dy_equal_variables), 64),
            .free_ids_arrays = dy_array_create(sizeof(dy_array_t), DY_ALIGNOF(dy_array_t), 8),
            .constraints = dy_array_create(sizeof(struct dy_constraint), DY_ALIGNOF(struct dy_constraint), 64),
            .custom_shared = dy_array_create(sizeof(struct dy_core_custom_shared), DY_ALIGNOF(struct dy_core_custom_shared), 3)
        },
        .core_is_present = false
    };

    dy_uv_register(&doc.core_ctx.custom_shared);
    dy_def_register(&doc.core_ctx.custom_shared);
    dy_string_register(&doc.core_ctx.custom_shared);
    dy_string_type_register(&doc.core_ctx.custom_shared);

    process_document(ctx, &doc);

    dy_array_add(&ctx->documents, &doc);
}

void dy_lsp_did_close(dy_lsp_ctx_t *ctx, dy_string_t uri)
{
    for (size_t i = 0, size = ctx->documents.num_elems; i < size; ++i) {
        const struct document *doc = dy_array_pos(&ctx->documents, i);

        if (dy_string_are_equal(array_view(&doc->uri), uri)) {
            dy_array_remove(&ctx->documents, i);
            break;
        }
    }
}

void dy_lsp_did_change(dy_lsp_ctx_t *ctx, dy_string_t uri, const uint8_t *content_changes)
{
    for (size_t i = 0, size = ctx->documents.num_elems; i < size; ++i) {
        struct document *doc = dy_array_pos(&ctx->documents, i);

        if (!dy_string_are_equal(array_view(&doc->uri), uri)) {
            continue;
        }

        while (*content_changes != DY_JSON_END) {
            const uint8_t *change = content_changes;

            if (*change != DY_JSON_OBJECT) {
                return;
            }
            change++;

            const uint8_t *text = json_get_member(change, DY_STR_LIT("text"));
            if (text == NULL) {
                return;
            }
            if (*text != DY_JSON_STRING) {
                return;
            }
            ++text;

            dy_string_t text_string = convert_json_string(text);

            replace_storage_with_view(&doc->text, text_string);

            process_document(ctx, doc);

            content_changes = skip_json_value(content_changes);
        }

        break;
    }
}

void dy_lsp_hover(dy_lsp_ctx_t *ctx, const uint8_t *id, dy_string_t uri, long line_number, long utf16_char_offset, dy_array_t *json)
{
    for (size_t i = 0, size = ctx->documents.num_elems; i < size; ++i) {
        struct document *doc = dy_array_pos(&ctx->documents, i);

        if (!dy_string_are_equal(array_view(&doc->uri), uri)) {
            continue;
        }

        null_success_response(id, json);

        return;
    }

    invalid_request(id, DY_STR_LIT("Could not find the document."), json);
}

void dy_lsp_exit(dy_lsp_ctx_t *ctx)
{
    if (ctx->received_shutdown_request) {
        ctx->exit_code = 0;
    }
}

int dy_lsp_exit_code(dy_lsp_ctx_t *ctx)
{
    return ctx->exit_code;
}

void initialize_response(const uint8_t *id, dy_array_t *json)
{
    dy_array_add(json, &(uint8_t){ DY_JSON_OBJECT });

    put_string_literal(DY_STR_LIT("jsonrpc"), json);
    put_string_literal(DY_STR_LIT("2.0"), json);

    put_string_literal(DY_STR_LIT("id"), json);
    copy_json_value(id, json);

    put_string_literal(DY_STR_LIT("result"), json);

    dy_array_add(json, &(uint8_t){ DY_JSON_OBJECT });

    put_string_literal(DY_STR_LIT("capabilities"), json);
    server_capabilities(json);

    put_string_literal(DY_STR_LIT("serverInfo"), json);
    server_info(json);

    dy_array_add(json, &(uint8_t){ DY_JSON_END });

    dy_array_add(json, &(uint8_t){ DY_JSON_END });
}

void invalid_request(const uint8_t *id, dy_string_t message, dy_array_t *json)
{
    dy_array_add(json, &(uint8_t){ DY_JSON_OBJECT });

    put_string_literal(DY_STR_LIT("jsonrpc"), json);
    put_string_literal(DY_STR_LIT("2.0"), json);

    put_string_literal(DY_STR_LIT("id"), json);
    copy_json_value(id, json);

    put_string_literal(DY_STR_LIT("error"), json);
    response_error(-32600, message, json);

    dy_array_add(json, &(uint8_t){ DY_JSON_END });
}

void method_not_found(const uint8_t *id, dy_string_t message, dy_array_t *json)
{
    dy_array_add(json, &(uint8_t){ DY_JSON_OBJECT });

    put_string_literal(DY_STR_LIT("jsonrpc"), json);
    put_string_literal(DY_STR_LIT("2.0"), json);

    put_string_literal(DY_STR_LIT("id"), json);
    copy_json_value(id, json);

    put_string_literal(DY_STR_LIT("error"), json);
    response_error(-32601, message, json);

    dy_array_add(json, &(uint8_t){ DY_JSON_END });
}

void response_error(long code, dy_string_t message, dy_array_t *json)
{
    dy_array_add(json, &(uint8_t){ DY_JSON_OBJECT });

    put_string_literal(DY_STR_LIT("code"), json);
    put_number(code, json);

    put_string_literal(DY_STR_LIT("message"), json);
    put_string_literal(message, json);

    dy_array_add(json, &(uint8_t){ DY_JSON_END });
}

void server_capabilities(dy_array_t *json)
{
    dy_array_add(json, &(uint8_t){ DY_JSON_OBJECT });

    put_string_literal(DY_STR_LIT("textDocumentSync"), json);
    text_document_sync_options(json);

    put_string_literal(DY_STR_LIT("hoverProvider"), json);
    dy_array_add(json, &(uint8_t){ DY_JSON_TRUE });

    dy_array_add(json, &(uint8_t){ DY_JSON_END });
}

void server_info(dy_array_t *json)
{
    dy_array_add(json, &(uint8_t){ DY_JSON_OBJECT });

    put_string_literal(DY_STR_LIT("name"), json);
    put_string_literal(DY_STR_LIT("Duality"), json);

    put_string_literal(DY_STR_LIT("version"), json);
    put_string_literal(DY_STR_LIT("0.0.1"), json);

    dy_array_add(json, &(uint8_t){ DY_JSON_END });
}

void text_document_sync_options(dy_array_t *json)
{
    dy_array_add(json, &(uint8_t){ DY_JSON_OBJECT });

    put_string_literal(DY_STR_LIT("openClose"), json);
    dy_array_add(json, &(uint8_t){ DY_JSON_TRUE });

    put_string_literal(DY_STR_LIT("change"), json);
    put_number(1, json);

    dy_array_add(json, &(uint8_t){ DY_JSON_END });
}

void null_success_response(const uint8_t *id, dy_array_t *json)
{
    dy_array_add(json, &(uint8_t){ DY_JSON_OBJECT });

    put_string_literal(DY_STR_LIT("jsonrpc"), json);
    put_string_literal(DY_STR_LIT("2.0"), json);

    put_string_literal(DY_STR_LIT("id"), json);
    copy_json_value(id, json);

    put_string_literal(DY_STR_LIT("result"), json);
    dy_array_add(json, &(uint8_t){ DY_JSON_NULL });

    dy_array_add(json, &(uint8_t){ DY_JSON_END });
}

void hover_result(dy_string_t contents, dy_array_t *json)
{
    dy_array_add(json, &(uint8_t){ DY_JSON_OBJECT });

    put_string_literal(DY_STR_LIT("contents"), json);
    put_string_literal(contents, json);

    dy_array_add(json, &(uint8_t){ DY_JSON_END });
}

void publish_diagnostics(struct dy_core_ctx *ctx, dy_string_t uri, const dy_array_t *text_sources, struct dy_core_expr expr, dy_string_t text, dy_array_t *json)
{
    dy_array_add(json, &(uint8_t){ DY_JSON_OBJECT });

    put_string_literal(DY_STR_LIT("jsonrpc"), json);
    put_string_literal(DY_STR_LIT("2.0"), json);

    put_string_literal(DY_STR_LIT("method"), json);
    put_string_literal(DY_STR_LIT("textDocument/publishDiagnostics"), json);

    put_string_literal(DY_STR_LIT("params"), json);
    diagnostics_params(ctx, uri, text_sources, expr, text, json);

    dy_array_add(json, &(uint8_t){ DY_JSON_END });
}

void process_document(struct dy_lsp_ctx *ctx, struct document *doc)
{
    if (doc->core_is_present) {
        dy_core_expr_release(&doc->core_ctx, doc->core);
        doc->core_is_present = false;
    }

    struct dy_utf8_to_ast_ctx utf8_to_ast_ctx = {
        .stream = {
            .get_chars = null_stream,
            .buffer = doc->text,
            .env = NULL,
            .current_index = 0
        }
    };

    struct dy_ast_do_block ast;
    if (!dy_utf8_to_ast_file(&utf8_to_ast_ctx, &ast)) {
        return;
    }

    struct dy_ast_to_core_ctx ast_to_core_ctx = {
        .running_id = doc->core_ctx.running_id,
        .variable_replacements = dy_array_create(sizeof(struct dy_variable_replacement), DY_ALIGNOF(struct dy_variable_replacement), 128)
    };

    struct dy_core_expr core = dy_ast_do_block_to_core(&ast_to_core_ctx, ast);

    doc->core_ctx.running_id = ast_to_core_ctx.running_id;

    struct dy_core_expr checked_core;
    if (dy_check_expr(&doc->core_ctx, core, &checked_core)) {
        dy_core_expr_release(&doc->core_ctx, core);
        core = checked_core;
    }

    doc->core_is_present = true;
    doc->core = core;

    //publish_diagnostics(&doc->core_ctx, array_view(&doc->uri), &parser_ctx.text_sources, doc->core, array_view(&doc->text), &ctx->output_buffer);

    //dy_lsp_send(ctx);
}

void null_stream(dy_array_t *buffer, void *env)
{
    (void)buffer;
    (void)env;
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

bool produce_diagnostics(struct dy_core_ctx *ctx, const dy_array_t *text_sources, struct dy_core_expr expr, dy_string_t text, dy_array_t *json)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_PROBLEM:
        switch (expr.problem.tag) {
        case DY_CORE_FUNCTION: {
            bool b1 = produce_diagnostics(ctx, text_sources, *expr.problem.function.type, text, json);
            bool b2 = produce_diagnostics(ctx, text_sources, *expr.problem.function.expr, text, json);
            return b1 && b2;
        }
        case DY_CORE_PAIR: {
            bool b1 = produce_diagnostics(ctx, text_sources, *expr.problem.pair.left, text, json);
            bool b2 = produce_diagnostics(ctx, text_sources, *expr.problem.pair.right, text, json);
            return b1 && b2;
        }
        case DY_CORE_RECURSION:
            return produce_diagnostics(ctx, text_sources, *expr.problem.recursion.expr, text, json);
        }

        dy_bail("impossible");
    case DY_CORE_EXPR_SOLUTION:
        return produce_solution_diagnostics(ctx, text_sources, expr.solution, text, json);
    case DY_CORE_EXPR_APPLICATION: {
            bool b1 = produce_diagnostics(ctx, text_sources, *expr.application.expr, text, json);
            bool b2 = produce_solution_diagnostics(ctx, text_sources, expr.application.solution, text, json);
            return b1 && b2;
        }
    case DY_CORE_EXPR_VARIABLE:
    case DY_CORE_EXPR_ANY:
    case DY_CORE_EXPR_VOID:
    case DY_CORE_EXPR_INFERENCE_VAR:
        return true;
    case DY_CORE_EXPR_CUSTOM:
        // TODO: Add diagnostics callback to custom exprs.
        return true;
    case DY_CORE_EXPR_INFERENCE_CTX:
        dy_bail("impossible");
    }

    dy_bail("impossible");
}

bool produce_solution_diagnostics(struct dy_core_ctx *ctx, const dy_array_t *text_sources, struct dy_core_solution solution, dy_string_t text, dy_array_t *json)
{
    if (solution.tag == DY_CORE_FUNCTION) {
        bool b1 = produce_diagnostics(ctx, text_sources, *solution.expr, text, json);
        bool b2 = produce_diagnostics(ctx, text_sources, *solution.out, text, json);
        return b1 && b2;
    } else {
        return produce_diagnostics(ctx, text_sources, *solution.out, text, json);
    }
}

const struct dy_range *get_text_range(const dy_array_t *text_sources, size_t id)
{
    for (size_t i = 0, size = text_sources->num_elems; i < size; ++i) {
        /*const struct dy_text_source *p = dy_array_pos(text_sources, i);
        if (p->id == id) {
            return &p->text_range;
        }*/
    }

    return NULL;
}

void compute_lsp_range(dy_string_t text, struct dy_range range, dy_array_t *json)
{
    long line_start = 0;
    long character_start = 0;

    for (size_t i = 0; i < range.start; ++i) {
        if (text.ptr[i] == '\n') {
            ++line_start;
            character_start = 0;
        } else {
            ++character_start;
        }
    }

    long line_end = line_start;
    long character_end = character_start;

    for (size_t i = range.start; i < range.end; ++i) {
        if (text.ptr[i] == '\n') {
            ++line_end;
            character_end = 0;
        } else {
            ++character_end;
        }
    }

    make_range(line_start, character_start, line_end, character_end, json);
}

void make_diagnostic(dy_string_t text, struct dy_range range, long severity, dy_string_t message, dy_array_t *json)
{
    dy_array_add(json, &(uint8_t){ DY_JSON_OBJECT });

    put_string_literal(DY_STR_LIT("range"), json);
    compute_lsp_range(text, range, json);

    put_string_literal(DY_STR_LIT("severity"), json);
    put_number(severity, json);

    put_string_literal(DY_STR_LIT("message"), json);
    put_string_literal(message, json);

    dy_array_add(json, &(uint8_t){ DY_JSON_END });
}

void diagnostics_params(struct dy_core_ctx *ctx, dy_string_t uri, const dy_array_t *text_sources, struct dy_core_expr expr, dy_string_t text, dy_array_t *json)
{
    dy_array_add(json, &(uint8_t){ DY_JSON_OBJECT });

    put_string_literal(DY_STR_LIT("uri"), json);
    put_string_literal(uri, json);

    put_string_literal(DY_STR_LIT("diagnostics"), json);
    dy_array_add(json, &(uint8_t){ DY_JSON_ARRAY });
    produce_diagnostics(ctx, text_sources, expr, text, json);
    dy_array_add(json, &(uint8_t){ DY_JSON_END });

    dy_array_add(json, &(uint8_t){ DY_JSON_END });
}

void make_position(long line, long character, dy_array_t *json)
{
    dy_array_add(json, &(uint8_t){ DY_JSON_OBJECT });

    put_string_literal(DY_STR_LIT("line"), json);
    put_number(line, json);

    put_string_literal(DY_STR_LIT("character"), json);
    put_number(character, json);

    dy_array_add(json, &(uint8_t){ DY_JSON_END });
}

void make_range(long start_line, long start_character, long end_line, long end_character, dy_array_t *json)
{
    dy_array_add(json, &(uint8_t){ DY_JSON_OBJECT });

    put_string_literal(DY_STR_LIT("start"), json);
    make_position(start_line, start_character, json);

    put_string_literal(DY_STR_LIT("end"), json);
    make_position(end_line, end_character, json);

    dy_array_add(json, &(uint8_t){ DY_JSON_END });
}

const uint8_t *json_get_member(const uint8_t *p, dy_string_t member)
{
    while (*p != DY_JSON_END) {
        dy_string_t s = convert_json_string(p + 1);

        const uint8_t *value = skip_json_string(p);

        if (dy_string_are_equal(s, member)) {
            return value;
        }

        p = skip_json_value(value);
    }

    return NULL;
}

dy_string_t convert_json_string(const uint8_t *p)
{
    size_t i = 0;

    while (p[i] != DY_JSON_END) {
        ++i;
    }

    return (dy_string_t){
        .ptr = (const char *)p,
        .size = i
    };
}

void put_string_literal(dy_string_t s, dy_array_t *json)
{
    dy_array_add(json, &(uint8_t){ DY_JSON_STRING });

    for (size_t i = 0; i < s.size; ++i) {
        dy_array_add(json, s.ptr + i);
    }

    dy_array_add(json, &(uint8_t){ DY_JSON_END });
}

const uint8_t *copy_json_value(const uint8_t *src, dy_array_t *dst)
{
    dy_array_add(dst, src);

    switch (*src) {
    case DY_JSON_OBJECT:
        ++src;

        while (*src != DY_JSON_END) {
            src = copy_json_string(src, dst);
            src = copy_json_value(src, dst);
        }

        dy_array_add(dst, src);

        return src + 1;
    case DY_JSON_ARRAY:
        ++src;

        while (*src != DY_JSON_END) {
            src = copy_json_value(src, dst);
        }

        dy_array_add(dst, src);

        return src + 1;
    case DY_JSON_STRING:
        return copy_json_string(src, dst);
    case DY_JSON_NUMBER:
        ++src;

        for (size_t i = 0; i < sizeof(long); ++i) {
            dy_array_add(dst, src + i);
        }

        return src + sizeof(long);
    case DY_JSON_END:
    case DY_JSON_NULL:
    case DY_JSON_TRUE:
    case DY_JSON_FALSE:
        return src + 1;
    }

    dy_bail("Invalid JSON type");
}

const uint8_t *copy_json_string(const uint8_t *src, dy_array_t *dst)
{
    ++src;

    while (*src != DY_JSON_END) {
        dy_array_add(dst, src);
        ++src;
    }

    dy_array_add(dst, src);

    return src + 1;
}

const uint8_t *skip_json_value(const uint8_t *p)
{
    switch (*p) {
    case DY_JSON_OBJECT:
        ++p;

        while (*p != DY_JSON_END) {
            p = skip_json_string(p);
            p = skip_json_value(p);
        }

        return p + 1;
    case DY_JSON_ARRAY:
        ++p;

        while (*p != DY_JSON_END) {
            p = skip_json_value(p);
        }

        return p + 1;
    case DY_JSON_STRING:
        return skip_json_string(p);
    case DY_JSON_NUMBER:
        return p + 1 + sizeof(long);
    case DY_JSON_END:
    case DY_JSON_NULL:
    case DY_JSON_TRUE:
    case DY_JSON_FALSE:
        return p + 1;
    }

    dy_bail("Invalid JSON type");
}

const uint8_t *skip_json_string(const uint8_t *p)
{
    ++p;

    while (*p != DY_JSON_END) {
        ++p;
    }

    return p + 1;
}

void put_number(long x, dy_array_t *json)
{
    dy_array_add(json, &(uint8_t){ DY_JSON_NUMBER });
    for (size_t i = 0; i < sizeof(long); ++i) {
        dy_array_add(json, (uint8_t *)&x + i);
    }
}

dy_string_t array_view(const dy_array_t *x)
{
    return (dy_string_t){
        .ptr = x->buffer,
        .size = x->num_elems
    };
}

dy_array_t view_to_array(dy_string_t s)
{
    dy_array_t x = dy_array_create(1, 1, s.size);
    replace_storage_with_view(&x, s);
    return x;
}

void replace_storage_with_view(dy_array_t *x, dy_string_t s)
{
    x->num_elems = 0;
    for (size_t i = 0; i < s.size; ++i) {
        dy_array_add(x, s.ptr + i);
    }
}

void dy_lsp_send(dy_lsp_ctx_t *ctx)
{
    ctx->send(ctx->output_buffer.buffer, ctx->env);
    ctx->output_buffer.num_elems = 0;
}
