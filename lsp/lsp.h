/*
 * Copyright 2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_LSP_H
#define DY_LSP_H

#include "../support/json.h"
#include "../support/array.h"

#include "../core/check.h"

#include "../syntax/parser.h"
#include "../syntax/ast_to_core.h"

/**
 * This file implements LSP support.
 */

typedef struct dy_lsp_ctx dy_lsp_ctx_t;

typedef void (*dy_lsp_send_fn)(dy_json_t message, void *env);

static inline dy_lsp_ctx_t *dy_lsp_create(dy_lsp_send_fn send, void *env);

static inline bool dy_lsp_handle_message(dy_lsp_ctx_t *ctx, dy_json_t message);

static inline bool dy_lsp_initialize(dy_lsp_ctx_t *ctx, dy_json_t *result, dy_json_t *error);

static inline void dy_lsp_initialized(dy_lsp_ctx_t *ctx);

static inline dy_json_t dy_lsp_shutdown(dy_lsp_ctx_t *ctx);

static inline void dy_lsp_did_open(dy_lsp_ctx_t *ctx, dy_string_t uri, dy_string_t text);

static inline void dy_lsp_did_close(dy_lsp_ctx_t *ctx, dy_string_t uri);

static inline void dy_lsp_did_change(dy_lsp_ctx_t *ctx, dy_string_t uri, struct dy_json_array content_changes);

static inline bool dy_lsp_hover(dy_lsp_ctx_t *ctx, dy_string_t uri, long line_number, long utf16_char_offset, dy_json_t *result, dy_json_t *error);

static inline void dy_lsp_exit(dy_lsp_ctx_t *ctx);

static inline int dy_lsp_exit_code(dy_lsp_ctx_t *ctx);

static inline void dy_lsp_destroy(dy_lsp_ctx_t *ctx);

/**
 * Represents an open document.
 */
struct document {
    dy_string_t uri; /** The URI is used as the identifier for a document. */
    dy_string_t text;
    size_t running_id;
    struct dy_core_expr core;
    bool core_is_present;
};

struct dy_lsp_ctx {
    dy_lsp_send_fn send;
    void *env;

    bool is_initialized;
    bool received_shutdown_request;
    int exit_code;
    dy_array_t documents;
};

static inline dy_json_t invalid_request(dy_string_t message);
static inline dy_json_t server_capabilities(void);
static inline dy_json_t server_info(void);
static inline dy_json_t text_document_sync_options(void);
static inline dy_json_t initialize_response(void);
static inline dy_json_t error_response(dy_json_t id, dy_json_t error);
static inline dy_json_t method_not_found(dy_string_t message);
static inline dy_json_t response_error(long code, dy_string_t message);
static inline dy_json_t success_response(dy_json_t id, dy_json_t result);
static inline dy_json_t hover_result(dy_string_t contents);
static inline dy_json_t make_position(long line, long character);
static inline dy_json_t make_range(dy_json_t start, dy_json_t end);
static inline dy_json_t make_notification(dy_string_t method, dy_json_t params);

static inline void process_document(struct dy_lsp_ctx *ctx, struct document *doc);
static inline struct dy_stream stream_from_string(dy_string_t s);
static inline void null_stream(dy_array_t *buffer, void *env);
static inline bool compute_byte_offset(dy_string_t text, long line_offset, long utf16_offset, size_t *byte_offset);
static inline void produce_diagnostics(struct dy_core_ctx *ctx, struct dy_core_expr expr, dy_string_t text, dy_array_t *diagnostics);
static inline dy_json_t compute_lsp_range(dy_string_t text, struct dy_range range);
static inline dy_json_t error_message(struct dy_core_ctx *ctx, struct dy_core_equality_map_elim elim);
static inline dy_json_t make_diagnostic(dy_json_t range, dy_json_t severity, dy_json_t message);
static inline dy_json_t make_diagnostics_params(dy_string_t uri, dy_json_t diagnostics);

static inline bool json_force_object(dy_json_t json, struct dy_json_object *object);
static inline bool json_force_string(dy_json_t json, dy_string_t *value);
static inline bool json_get_member(struct dy_json_object object, dy_string_t member, dy_json_t *value);
static inline bool json_force_array(dy_json_t json, struct dy_json_array *array);
static inline bool json_force_integer(dy_json_t json, long *value);

dy_lsp_ctx_t *dy_lsp_create(dy_lsp_send_fn send, void *env)
{
    struct dy_lsp_ctx *ctx = dy_malloc(sizeof *ctx);
    *ctx = (struct dy_lsp_ctx){
        .send = send,
        .env = env,
        .is_initialized = false,
        .received_shutdown_request = false,
        .exit_code = 1, // Error by default.
        .documents = dy_array_create(sizeof(struct document), 8)
    };

    return ctx;
}

void dy_lsp_destroy(dy_lsp_ctx_t *ctx)
{
    dy_array_destroy(ctx->documents);
    dy_free(ctx);
}

bool dy_lsp_handle_message(dy_lsp_ctx_t *ctx, dy_json_t message)
{
    struct dy_json_object object;
    if (!json_force_object(message, &object)) {
        return true;
    }

    dy_json_t id;
    bool have_id = json_get_member(object, DY_STR_LIT("id"), &id);

    dy_json_t method;
    if (!json_get_member(object, DY_STR_LIT("method"), &method)) {
        if (have_id) {
            ctx->send(error_response(id, invalid_request(DY_STR_LIT("Missing 'method' field."))), ctx->env);
        }

        return true;
    }

    dy_json_t params;
    bool have_params = json_get_member(object, DY_STR_LIT("params"), &params);

    dy_string_t method_string;
    if (!json_force_string(method, &method_string)) {
        if (have_id) {
            ctx->send(error_response(id, invalid_request(DY_STR_LIT("'method' field is not a string."))), ctx->env);
        }

        return true;
    }

    if (dy_string_are_equal(method_string, DY_STR_LIT("exit"))) {
        dy_lsp_exit(ctx);
        return false;
    }

    if (ctx->received_shutdown_request) {
        if (have_id) {
            ctx->send(error_response(id, invalid_request(DY_STR_LIT("Non-exit request after shutdown."))), ctx->env);
        }

        return true;
    }

    if (dy_string_are_equal(method_string, DY_STR_LIT("initialize"))) {
        if (!have_id) {
            return true;
        }

        if (!have_params) {
            ctx->send(error_response(id, invalid_request(DY_STR_LIT("Missing 'params' field."))), ctx->env);
            return true;
        }

        dy_json_t result, error;
        if (dy_lsp_initialize(ctx, &result, &error)) {
            ctx->send(success_response(id, result), ctx->env);
        } else {
            ctx->send(error_response(id, error), ctx->env);
        }

        return true;
    }

    if (!ctx->is_initialized) {
        if (have_id) {
            ctx->send(error_response(id, response_error(-32002, DY_STR_LIT("Not yet initialized."))), ctx->env);
        }

        return true;
    }

    if (dy_string_are_equal(method_string, DY_STR_LIT("shutdown"))) {
        if (!have_id) {
            return true;
        }

        ctx->send(success_response(id, dy_lsp_shutdown(ctx)), ctx->env);

        return true;
    }

    if (dy_string_are_equal(method_string, DY_STR_LIT("initialized"))) {
        dy_lsp_initialized(ctx);
        return true;
    }

    if (dy_string_are_equal(method_string, DY_STR_LIT("textDocument/didOpen"))) {
        if (!have_params) {
            return true;
        }

        struct dy_json_object param_obj;
        if (!json_force_object(params, &param_obj)) {
            return true;
        }

        dy_json_t text_document;
        if (!json_get_member(param_obj, DY_STR_LIT("textDocument"), &text_document)) {
            return true;
        }

        struct dy_json_object text_document_obj;
        if (!json_force_object(text_document, &text_document_obj)) {
            return true;
        }

        dy_json_t uri;
        if (!json_get_member(text_document_obj, DY_STR_LIT("uri"), &uri)) {
            return true;
        }

        dy_string_t uri_string;
        if (!json_force_string(uri, &uri_string)) {
            return true;
        }

        dy_json_t text;
        if (!json_get_member(text_document_obj, DY_STR_LIT("text"), &text)) {
            return true;
        }

        dy_string_t text_string;
        if (!json_force_string(text, &text_string)) {
            return true;
        }

        dy_lsp_did_open(ctx, uri_string, text_string);

        return true;
    }

    if (dy_string_are_equal(method_string, DY_STR_LIT("textDocument/didChange"))) {
        if (!have_params) {
            return true;
        }

        struct dy_json_object params_obj;
        if (!json_force_object(params, &params_obj)) {
            return true;
        }

        dy_json_t text_document;
        if (!json_get_member(params_obj, DY_STR_LIT("textDocument"), &text_document)) {
            return true;
        }

        struct dy_json_object text_document_obj;
        if (!json_force_object(text_document, &text_document_obj)) {
            return true;
        }

        dy_json_t uri;
        if (!json_get_member(text_document_obj, DY_STR_LIT("uri"), &uri)) {
            return true;
        }

        dy_string_t uri_string;
        if (!json_force_string(uri, &uri_string)) {
            return true;
        }

        dy_json_t content_changes;
        if (!json_get_member(params_obj, DY_STR_LIT("contentChanges"), &content_changes)) {
            return true;
        }

        struct dy_json_array content_changes_array;
        if (!json_force_array(content_changes, &content_changes_array)) {
            return true;
        }

        dy_lsp_did_change(ctx, uri_string, content_changes_array);

        return true;
    }

    if (dy_string_are_equal(method_string, DY_STR_LIT("textDocument/didClose"))) {
        if (!have_params) {
            return true;
        }

        struct dy_json_object params_obj;
        if (!json_force_object(params, &params_obj)) {
            return true;
        }

        dy_json_t text_document;
        if (!json_get_member(params_obj, DY_STR_LIT("textDocument"), &text_document)) {
            return true;
        }

        struct dy_json_object text_document_obj;
        if (!json_force_object(text_document, &text_document_obj)) {
            return true;
        }

        dy_json_t uri;
        if (!json_get_member(text_document_obj, DY_STR_LIT("uri"), &uri)) {
            return true;
        }

        dy_string_t uri_string;
        if (!json_force_string(uri, &uri_string)) {
            return true;
        }

        dy_lsp_did_close(ctx, uri_string);

        return true;
    }

    if (dy_string_are_equal(method_string, DY_STR_LIT("textDocument/hover"))) {
        if (!have_id) {
            return true;
        }

        if (!have_params) {
            ctx->send(error_response(id, invalid_request(DY_STR_LIT("Missing 'params' field."))), ctx->env);
            return true;
        }

        struct dy_json_object params_obj;
        if (!json_force_object(params, &params_obj)) {
            ctx->send(error_response(id, invalid_request(DY_STR_LIT("'params' is not an object."))), ctx->env);
            return true;
        }

        dy_json_t text_document;
        if (!json_get_member(params_obj, DY_STR_LIT("textDocument"), &text_document)) {
            ctx->send(error_response(id, invalid_request(DY_STR_LIT("Missing 'textDocument' field."))), ctx->env);
            return true;
        }

        struct dy_json_object text_document_obj;
        if (!json_force_object(text_document, &text_document_obj)) {
            ctx->send(error_response(id, invalid_request(DY_STR_LIT("'textDocument' is not an object."))), ctx->env);
            return true;
        }

        dy_json_t uri;
        if (!json_get_member(text_document_obj, DY_STR_LIT("uri"), &uri)) {
            ctx->send(error_response(id, invalid_request(DY_STR_LIT("Missing 'uri' field."))), ctx->env);
            return true;
        }

        dy_string_t uri_string;
        if (!json_force_string(uri, &uri_string)) {
            ctx->send(error_response(id, invalid_request(DY_STR_LIT("'uri' is not a string."))), ctx->env);
            return true;
        }

        dy_json_t position;
        if (!json_get_member(params_obj, DY_STR_LIT("position"), &position)) {
            ctx->send(error_response(id, invalid_request(DY_STR_LIT("Missing 'position' field."))), ctx->env);
            return true;
        }

        struct dy_json_object position_obj;
        if (!json_force_object(position, &position_obj)) {
            ctx->send(error_response(id, invalid_request(DY_STR_LIT("'position' is not a string."))), ctx->env);
            return true;
        }

        dy_json_t line;
        if (!json_get_member(position_obj, DY_STR_LIT("line"), &line)) {
            ctx->send(error_response(id, invalid_request(DY_STR_LIT("Missing 'line' field."))), ctx->env);
            return true;
        }

        long line_number;
        if (!json_force_integer(line, &line_number)) {
            ctx->send(error_response(id, invalid_request(DY_STR_LIT("'line' is not a number."))), ctx->env);
            return true;
        }

        dy_json_t character;
        if (!json_get_member(position_obj, DY_STR_LIT("character"), &character)) {
            ctx->send(error_response(id, invalid_request(DY_STR_LIT("Missing 'character' field."))), ctx->env);
            return true;
        }

        long character_number;
        if (!json_force_integer(character, &character_number)) {
            ctx->send(error_response(id, invalid_request(DY_STR_LIT("'character' is not a number."))), ctx->env);
            return true;
        }

        dy_json_t result, error;
        if (dy_lsp_hover(ctx, uri_string, line_number, character_number, &result, &error)) {
            ctx->send(success_response(id, result), ctx->env);
        } else {
            ctx->send(error_response(id, error), ctx->env);
        }

        return true;
    }

    if (have_id) {
        ctx->send(error_response(id, method_not_found(DY_STR_LIT("Unknown method name."))), ctx->env);
    }

    return true;
}

bool dy_lsp_initialize(dy_lsp_ctx_t *ctx, dy_json_t *result, dy_json_t *error)
{
    if (ctx->is_initialized) {
        *error = invalid_request(DY_STR_LIT("Already initialized."));
        return false;
    }

    ctx->is_initialized = true;

    *result = initialize_response();

    return true;
}

void dy_lsp_initialized(dy_lsp_ctx_t *ctx)
{
    (void)ctx;
    return;
}

dy_json_t dy_lsp_shutdown(dy_lsp_ctx_t *ctx)
{
    ctx->received_shutdown_request = true;
    return dy_json_null();
}

void dy_lsp_did_open(dy_lsp_ctx_t *ctx, dy_string_t uri, dy_string_t text)
{
    struct document doc = {
        .uri = uri,
        .text = text,
        .running_id = 0,
        .core_is_present = false
    };

    process_document(ctx, &doc);

    dy_array_add(&ctx->documents, &doc);
}

void dy_lsp_did_close(dy_lsp_ctx_t *ctx, dy_string_t uri)
{
    for (size_t i = 0, size = ctx->documents.num_elems; i < size; ++i) {
        struct document doc;
        dy_array_get(ctx->documents, i, &doc);

        if (dy_string_are_equal(doc.uri, uri)) {
            dy_array_remove(&ctx->documents, i);
            break;
        }
    }
}

void dy_lsp_did_change(dy_lsp_ctx_t *ctx, dy_string_t uri, struct dy_json_array content_changes)
{
    for (size_t i = 0, size = ctx->documents.num_elems; i < size; ++i) {
        struct document *doc = dy_array_pos(ctx->documents, i);

        if (!dy_string_are_equal(doc->uri, uri)) {
            continue;
        }

        for (size_t k = 0; k < content_changes.num_values; ++k) {
            dy_json_t change = content_changes.values[k];

            struct dy_json_object change_obj;
            if (!json_force_object(change, &change_obj)) {
                return;
            }

            dy_json_t text;
            if (!json_get_member(change_obj, DY_STR_LIT("text"), &text)) {
                return;
            }

            dy_string_t text_string;
            if (!json_force_string(text, &text_string)) {
                return;
            }

            doc->text = text_string;

            process_document(ctx, doc);
        }

        break;
    }
}

bool dy_lsp_hover(dy_lsp_ctx_t *ctx, dy_string_t uri, long line_number, long utf16_char_offset, dy_json_t *result, dy_json_t *error)
{
    for (size_t i = 0, size = ctx->documents.num_elems; i < size; ++i) {
        struct document *doc = dy_array_pos(ctx->documents, i);

        if (!dy_string_are_equal(doc->uri, uri)) {
            continue;
        }

        *result = dy_json_null();
        return true;
    }

    *error = invalid_request(DY_STR_LIT("Could not find the document."));
    return false;
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

dy_json_t initialize_response()
{
    struct dy_json_member capabilities = {
        .string = DY_STR_LIT("capabilities"),
        .value = server_capabilities()
    };

    struct dy_json_member m_server_info = {
        .string = DY_STR_LIT("serverInfo"),
        .value = server_info()
    };

    dy_array_t members = dy_array_create(sizeof(struct dy_json_member), 2);
    dy_array_add(&members, &capabilities);
    dy_array_add(&members, &m_server_info);

    return (dy_json_t){
        .tag = DY_JSON_VALUE_OBJECT,
        .object = {
            .members = members.buffer,
            .num_members = members.num_elems,
        }
    };
}

dy_json_t invalid_request(dy_string_t message)
{
    return response_error(-32600, message);
}

dy_json_t method_not_found(dy_string_t message)
{
    return response_error(-32601, message);
}

dy_json_t response_error(long code, dy_string_t message)
{
    struct dy_json_member m_code = {
        .string = DY_STR_LIT("code"),
        .value = dy_json_integer(code)
    };

    struct dy_json_member m_message = {
        .string = DY_STR_LIT("message"),
        .value = dy_json_string(message)
    };

    dy_array_t members = dy_array_create(sizeof(struct dy_json_member), 2);
    dy_array_add(&members, &m_code);
    dy_array_add(&members, &m_message);

    return (dy_json_t){
        .tag = DY_JSON_VALUE_OBJECT,
        .object = {
            .members = members.buffer,
            .num_members = members.num_elems,
        }
    };
}

dy_json_t server_capabilities()
{
    struct dy_json_member text_document_sync = {
        .string = DY_STR_LIT("textDocumentSync"),
        .value = text_document_sync_options()
    };

    struct dy_json_member hover_provider = {
        .string = DY_STR_LIT("hoverProvider"),
        .value = dy_json_true()
    };

    dy_array_t members = dy_array_create(sizeof(struct dy_json_member), 2);
    dy_array_add(&members, &text_document_sync);
    dy_array_add(&members, &hover_provider);

    return (dy_json_t){
        .tag = DY_JSON_VALUE_OBJECT,
        .object = {
            .members = members.buffer,
            .num_members = members.num_elems,
        }
    };
}

dy_json_t server_info(void)
{
    struct dy_json_member name = {
        .string = DY_STR_LIT("name"),
        .value = dy_json_string(DY_STR_LIT("Duality"))
    };

    struct dy_json_member version = {
        .string = DY_STR_LIT("version"),
        .value = dy_json_string(DY_STR_LIT("0.0.1"))
    };

    dy_array_t members = dy_array_create(sizeof(struct dy_json_member), 2);
    dy_array_add(&members, &name);
    dy_array_add(&members, &version);

    return (dy_json_t){
        .tag = DY_JSON_VALUE_OBJECT,
        .object = {
            .members = members.buffer,
            .num_members = members.num_elems,
        }
    };
}

dy_json_t text_document_sync_options(void)
{
    struct dy_json_member open_close = {
        .string = DY_STR_LIT("openClose"),
        .value = dy_json_true()
    };

    struct dy_json_member change = {
        .string = DY_STR_LIT("change"),
        .value = dy_json_integer(1)
    };

    dy_array_t members = dy_array_create(sizeof(struct dy_json_member), 2);
    dy_array_add(&members, &open_close);
    dy_array_add(&members, &change);

    return (dy_json_t){
        .tag = DY_JSON_VALUE_OBJECT,
        .object = {
            .members = members.buffer,
            .num_members = members.num_elems,
        }
    };
}

dy_json_t error_response(dy_json_t id, dy_json_t error)
{
    struct dy_json_member jsonrpc = {
        .string = DY_STR_LIT("jsonrpc"),
        .value = dy_json_string(DY_STR_LIT("2.0"))
    };

    struct dy_json_member m_id = {
        .string = DY_STR_LIT("id"),
        .value = id
    };

    struct dy_json_member m_error = {
        .string = DY_STR_LIT("error"),
        .value = error
    };

    dy_array_t members = dy_array_create(sizeof(struct dy_json_member), 3);
    dy_array_add(&members, &jsonrpc);
    dy_array_add(&members, &m_id);
    dy_array_add(&members, &m_error);

    return (dy_json_t){
        .tag = DY_JSON_VALUE_OBJECT,
        .object = {
            .members = members.buffer,
            .num_members = members.num_elems,
        }
    };
}

dy_json_t success_response(dy_json_t id, dy_json_t result)
{
    struct dy_json_member jsonrpc = {
        .string = DY_STR_LIT("jsonrpc"),
        .value = dy_json_string(DY_STR_LIT("2.0"))
    };

    struct dy_json_member m_id = {
        .string = DY_STR_LIT("id"),
        .value = id
    };

    struct dy_json_member m_result = {
        .string = DY_STR_LIT("result"),
        .value = result
    };

    dy_array_t members = dy_array_create(sizeof(struct dy_json_member), 3);
    dy_array_add(&members, &jsonrpc);
    dy_array_add(&members, &m_id);
    dy_array_add(&members, &m_result);

    return (dy_json_t){
        .tag = DY_JSON_VALUE_OBJECT,
        .object = {
            .members = members.buffer,
            .num_members = members.num_elems,
        }
    };
}

dy_json_t hover_result(dy_string_t contents)
{
    struct dy_json_member m_contents = {
        .string = DY_STR_LIT("contents"),
        .value = dy_json_string(contents)
    };

    dy_array_t members = dy_array_create(sizeof(struct dy_json_member), 1);
    dy_array_add(&members, &m_contents);

    return (dy_json_t){
        .tag = DY_JSON_VALUE_OBJECT,
        .object = {
            .members = members.buffer,
            .num_members = members.num_elems,
        }
    };
}

dy_json_t make_notification(dy_string_t method, dy_json_t params)
{
    struct dy_json_member jsonrpc_member = {
        .string = DY_STR_LIT("jsonrpc"),
        .value = dy_json_string(DY_STR_LIT("2.0"))
    };

    struct dy_json_member method_member = {
        .string = DY_STR_LIT("method"),
        .value = dy_json_string(method)
    };

    struct dy_json_member params_member = {
        .string = DY_STR_LIT("params"),
        .value = params
    };

    dy_array_t members = dy_array_create(sizeof(struct dy_json_member), 3);
    dy_array_add(&members, &jsonrpc_member);
    dy_array_add(&members, &method_member);
    dy_array_add(&members, &params_member);

    return (dy_json_t){
        .tag = DY_JSON_VALUE_OBJECT,
        .object = {
            .members = members.buffer,
            .num_members = members.num_elems,
        }
    };
}

void process_document(struct dy_lsp_ctx *ctx, struct document *doc)
{
    if (doc->core_is_present) {
        dy_core_expr_release(doc->core);
        doc->core_is_present = false;
    }

    struct dy_parser_ctx parser_ctx = {
        .stream = stream_from_string(doc->text),
        .string_arrays = dy_array_create(sizeof(dy_array_t *), 32)
    };

    struct dy_ast_do_block ast;
    bool parsing_succeeded = dy_parse_file(&parser_ctx, &ast);

    dy_array_destroy(parser_ctx.stream.buffer);

    if (!parsing_succeeded) {
        return;
    }

    struct dy_ast_to_core_ctx ast_to_core_ctx = {
        .running_id = 0,
        .bound_vars = dy_array_create(sizeof(struct dy_ast_to_core_bound_var), 64)
    };

    struct dy_core_expr core = dy_ast_do_block_to_core(&ast_to_core_ctx, ast);

    dy_ast_do_block_release(ast.body);

    struct dy_core_ctx core_ctx = {
        .running_id = ast_to_core_ctx.running_id,
        .bound_constraints = dy_array_create(sizeof(struct dy_bound_constraint), 1)
    };

    struct dy_constraint c;
    bool have_c = false;
    struct dy_core_expr new_core = dy_check_expr(&core_ctx, core, &c, &have_c);

    dy_core_expr_release(core);

    doc->core_is_present = true;
    doc->core = new_core;

    dy_array_t diagnostics = dy_array_create(sizeof(dy_json_t), 8);
    produce_diagnostics(&core_ctx, new_core, doc->text, &diagnostics);

    dy_json_t diag_json = {
        .tag = DY_JSON_VALUE_ARRAY,
        .array = {
            .values = diagnostics.buffer,
            .num_values = diagnostics.num_elems,
        }
    };

    dy_json_t params = make_diagnostics_params(doc->uri, diag_json);

    ctx->send(make_notification(DY_STR_LIT("textDocument/publishDiagnostics"), params), ctx->env);
}

struct dy_stream stream_from_string(dy_string_t s)
{
    dy_array_t buffer = dy_array_create(sizeof(char), s.size);
    for (size_t i = 0; i < s.size; ++i) {
        dy_array_add(&buffer, s.ptr + i);
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

void produce_diagnostics(struct dy_core_ctx *ctx, struct dy_core_expr expr, dy_string_t text, dy_array_t *diagnostics)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_EQUALITY_MAP:
        produce_diagnostics(ctx, *expr.equality_map.e1, text, diagnostics);
        produce_diagnostics(ctx, *expr.equality_map.e2, text, diagnostics);
        return;
    case DY_CORE_EXPR_TYPE_MAP:
        produce_diagnostics(ctx, *expr.type_map.binding.type, text, diagnostics);
        produce_diagnostics(ctx, *expr.type_map.expr, text, diagnostics);
        return;
    case DY_CORE_EXPR_EQUALITY_MAP_ELIM:
        produce_diagnostics(ctx, *expr.equality_map_elim.expr, text, diagnostics);
        produce_diagnostics(ctx, *expr.equality_map_elim.map.e1, text, diagnostics);
        produce_diagnostics(ctx, *expr.equality_map_elim.map.e2, text, diagnostics);

        if (expr.equality_map_elim.check_result != DY_YES && expr.equality_map_elim.has_text_range) {
            dy_json_t range = compute_lsp_range(text, expr.equality_map_elim.text_range);

            dy_json_t msg = error_message(ctx, expr.equality_map_elim);

            long severity = expr.equality_map_elim.check_result == DY_NO ? 1 : 2;

            dy_json_t diagnostic = make_diagnostic(range, dy_json_integer(severity), msg);

            dy_array_add(diagnostics, &diagnostic);
        }

        return;
    case DY_CORE_EXPR_TYPE_MAP_ELIM:
        produce_diagnostics(ctx, *expr.type_map_elim.expr, text, diagnostics);
        produce_diagnostics(ctx, *expr.type_map_elim.map.binding.type, text, diagnostics);
        produce_diagnostics(ctx, *expr.type_map_elim.map.expr, text, diagnostics);

        if (expr.type_map_elim.check_result != DY_YES && expr.type_map_elim.has_text_range) {
            /*dy_json_t range = compute_lsp_range(text, expr.type_map_elim.text_range);

            dy_json_t msg = error_message(ctx, expr.type_map_elim);

            long severity = expr.equality_map_elim.check_result == DY_NO ? 1 : 2;

            dy_json_t diagnostic = make_diagnostic(range, dy_json_integer(severity), msg);

            dy_array_add(diagnostics, &diagnostic);*/
        }

        return;
    case DY_CORE_EXPR_BOTH:
        produce_diagnostics(ctx, *expr.both.e1, text, diagnostics);
        produce_diagnostics(ctx, *expr.both.e2, text, diagnostics);
        return;
    case DY_CORE_EXPR_ONE_OF:
        produce_diagnostics(ctx, *expr.one_of.first, text, diagnostics);
        produce_diagnostics(ctx, *expr.one_of.second, text, diagnostics);
        return;
    case DY_CORE_EXPR_VARIABLE:
        return;
    case DY_CORE_EXPR_INFERENCE_TYPE_MAP:
        dy_bail("should never be reached");
    case DY_CORE_EXPR_RECURSION:
        produce_diagnostics(ctx, *expr.recursion.map.binding.type, text, diagnostics);
        produce_diagnostics(ctx, *expr.recursion.map.expr, text, diagnostics);
        return;
    case DY_CORE_EXPR_END:
        return;
    case DY_CORE_EXPR_CUSTOM:
        // TODO: Add diagnostics callback to custom exprs.
        return;
    case DY_CORE_EXPR_INFERENCE_VARIABLE:
        return;
    case DY_CORE_EXPR_SYMBOL:
        return;
    }

    dy_bail("Impossible core type.");
}

dy_json_t compute_lsp_range(dy_string_t text, struct dy_range range)
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

    return make_range(make_position(line_start, character_start), make_position(line_end, character_end));
}

dy_json_t error_message(struct dy_core_ctx *ctx, struct dy_core_equality_map_elim elim)
{
    if (elim.check_result == DY_NO) {
        return dy_json_string(DY_STR_LIT("Error message placeholder."));
    } else {
        return dy_json_string(DY_STR_LIT("Warning message placeholder."));
    }
}

dy_json_t make_diagnostic(dy_json_t range, dy_json_t severity, dy_json_t message)
{
    struct dy_json_member range_member = {
        .string = DY_STR_LIT("range"),
        .value = range
    };

    struct dy_json_member severity_member = {
        .string = DY_STR_LIT("severity"),
        .value = severity
    };

    struct dy_json_member message_member = {
        .string = DY_STR_LIT("message"),
        .value = message
    };

    dy_array_t members = dy_array_create(sizeof(struct dy_json_member), 3);
    dy_array_add(&members, &range_member);
    dy_array_add(&members, &severity_member);
    dy_array_add(&members, &message_member);

    return (dy_json_t){
        .tag = DY_JSON_VALUE_OBJECT,
        .object = {
            .members = members.buffer,
            .num_members = members.num_elems,
        }
    };
}

dy_json_t make_diagnostics_params(dy_string_t uri, dy_json_t diagnostics)
{
    struct dy_json_member uri_member = {
        .string = DY_STR_LIT("uri"),
        .value = dy_json_string(uri)
    };

    struct dy_json_member diagnostics_member = {
        .string = DY_STR_LIT("diagnostics"),
        .value = diagnostics
    };

    dy_array_t members = dy_array_create(sizeof(struct dy_json_member), 3);
    dy_array_add(&members, &uri_member);
    dy_array_add(&members, &diagnostics_member);

    return (dy_json_t){
        .tag = DY_JSON_VALUE_OBJECT,
        .object = {
            .members = members.buffer,
            .num_members = members.num_elems,
        }
    };
}

dy_json_t make_position(long line, long character)
{
    struct dy_json_member line_member = {
        .string = DY_STR_LIT("line"),
        .value = dy_json_integer(line)
    };

    struct dy_json_member character_member = {
        .string = DY_STR_LIT("character"),
        .value = dy_json_integer(character)
    };

    dy_array_t members = dy_array_create(sizeof(struct dy_json_member), 2);
    dy_array_add(&members, &line_member);
    dy_array_add(&members, &character_member);

    return (dy_json_t){
        .tag = DY_JSON_VALUE_OBJECT,
        .object = {
            .members = members.buffer,
            .num_members = members.num_elems,
        }
    };
}

dy_json_t make_range(dy_json_t start, dy_json_t end)
{
    struct dy_json_member start_member = {
        .string = DY_STR_LIT("start"),
        .value = start
    };

    struct dy_json_member end_member = {
        .string = DY_STR_LIT("end"),
        .value = end
    };

    dy_array_t members = dy_array_create(sizeof(struct dy_json_member), 2);
    dy_array_add(&members, &start_member);
    dy_array_add(&members, &end_member);

    return (dy_json_t){
        .tag = DY_JSON_VALUE_OBJECT,
        .object = {
            .members = members.buffer,
            .num_members = members.num_elems,
        }
    };
}

bool json_force_object(dy_json_t json, struct dy_json_object *object)
{
    if (json.tag != DY_JSON_VALUE_OBJECT) {
        return false;
    }

    *object = json.object;
    return true;
}

bool json_get_member(struct dy_json_object object, dy_string_t member, dy_json_t *value)
{
    for (size_t i = 0; i < object.num_members; ++i) {
        if (dy_string_are_equal(object.members[i].string, member)) {
            *value = object.members[i].value;
            return true;
        }
    }

    return false;
}

bool json_force_string(dy_json_t json, dy_string_t *value)
{
    if (json.tag != DY_JSON_VALUE_STRING) {
        return false;
    }

    *value = json.string;
    return true;
}

bool json_force_array(dy_json_t json, struct dy_json_array *array)
{
    if (json.tag != DY_JSON_VALUE_ARRAY) {
        return false;
    }

    *array = json.array;
    return true;
}

bool json_force_integer(dy_json_t json, long *value)
{
    if (json.tag != DY_JSON_VALUE_NUMBER || json.number.tag != DY_JSON_NUMBER_INTEGER) {
        return false;
    }

    *value = json.number.integer;
    return true;
}

#endif // DY_LSP_H
