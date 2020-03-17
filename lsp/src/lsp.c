/*
 * Copyright 2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <duality/lsp/lsp.h>

#include <duality/support/array.h>

#include <duality/syntax/ast_to_core.h>

#include <duality/core/check.h>
#include <duality/core/type_of.h>

struct document {
    dy_string_t uri;
    dy_string_t text;
    size_t running_id;
    struct dy_ast_do_block ast;
    bool ast_is_valid;
    struct dy_text_range_core_map core;
    bool core_is_valid;
};

struct dy_lsp_ctx {
    struct dy_allocator allocator;
    dy_lsp_send_fn send;
    void *env;

    bool is_initialized;
    bool received_shutdown_request;
    int exit_code;
    dy_array_t *documents;
};

static dy_json_t invalid_request(dy_lsp_ctx_t *ctx, dy_string_t message);
static dy_json_t server_capabilities(dy_lsp_ctx_t *ctx);
static dy_json_t server_info(dy_lsp_ctx_t *ctx);
static dy_json_t text_document_sync_options(dy_lsp_ctx_t *ctx);
static dy_json_t initialize_response(dy_lsp_ctx_t *ctx);
static dy_json_t error_response(dy_lsp_ctx_t *ctx, dy_json_t id, dy_json_t error);
static dy_json_t method_not_found(dy_lsp_ctx_t *ctx, dy_string_t message);
static dy_json_t response_error(dy_lsp_ctx_t *ctx, long code, dy_string_t message);
static dy_json_t success_response(dy_lsp_ctx_t *ctx, dy_json_t id, dy_json_t result);
static dy_json_t hover_result(dy_lsp_ctx_t *ctx, dy_string_t contents);

static void process_document(struct document *doc);
static struct dy_stream stream_from_string(struct dy_allocator allocator, dy_string_t s);
static void null_stream(dy_array_t *buffer, void *env);
static bool compute_byte_offset(dy_string_t text, long line_offset, long utf16_offset, size_t *byte_offset);
static bool type_of_expr_at_byte_offset(size_t *running_id, struct dy_text_range_core_map map, size_t byte_offset, struct dy_core_expr *type);

static bool json_force_object(dy_json_t json, struct dy_json_object *object);
static bool json_force_string(dy_json_t json, dy_string_t *value);
static bool json_get_member(struct dy_json_object object, dy_string_t member, dy_json_t *value);
static bool json_force_array(dy_json_t json, struct dy_json_array *array);
static bool json_force_integer(dy_json_t json, long *value);

dy_lsp_ctx_t *dy_lsp_create(struct dy_allocator allocator, dy_lsp_send_fn send, void *env)
{
    struct dy_lsp_ctx *ctx = allocator.alloc(sizeof *ctx, allocator.env);
    *ctx = (struct dy_lsp_ctx){
        .allocator = allocator,
        .send = send,
        .env = env,
        .is_initialized = false,
        .received_shutdown_request = false,
        .exit_code = 1, // Error by default.
        .documents = dy_array_create(allocator, sizeof(struct document), 8)
    };

    return ctx;
}

void dy_lsp_destroy(dy_lsp_ctx_t *ctx)
{
    ctx->allocator.free(ctx, sizeof *ctx, ctx->allocator.env);
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
            ctx->send(error_response(ctx, id, invalid_request(ctx, DY_STR_LIT("Missing 'method' field."))), ctx->env);
        }

        return true;
    }

    dy_json_t params;
    bool have_params = json_get_member(object, DY_STR_LIT("params"), &params);

    dy_string_t method_string;
    if (!json_force_string(method, &method_string)) {
        if (have_id) {
            ctx->send(error_response(ctx, id, invalid_request(ctx, DY_STR_LIT("'method' field is not a string."))), ctx->env);
        }

        return true;
    }

    if (dy_string_are_equal(method_string, DY_STR_LIT("exit"))) {
        dy_lsp_exit(ctx);
        return false;
    }

    if (ctx->received_shutdown_request) {
        if (have_id) {
            ctx->send(error_response(ctx, id, invalid_request(ctx, DY_STR_LIT("Non-exit request after shutdown."))), ctx->env);
        }

        return true;
    }

    if (dy_string_are_equal(method_string, DY_STR_LIT("initialize"))) {
        if (!have_id) {
            return true;
        }

        if (!have_params) {
            ctx->send(error_response(ctx, id, invalid_request(ctx, DY_STR_LIT("Missing 'params' field."))), ctx->env);
            return true;
        }

        dy_json_t result, error;
        if (dy_lsp_initialize(ctx, &result, &error)) {
            ctx->send(success_response(ctx, id, result), ctx->env);
        } else {
            ctx->send(error_response(ctx, id, error), ctx->env);
        }

        return true;
    }

    if (!ctx->is_initialized) {
        if (have_id) {
            ctx->send(error_response(ctx, id, response_error(ctx, -32002, DY_STR_LIT("Not yet initialized."))), ctx->env);
        }

        return true;
    }

    if (dy_string_are_equal(method_string, DY_STR_LIT("shutdown"))) {
        if (!have_id) {
            return true;
        }

        ctx->send(success_response(ctx, id, dy_lsp_shutdown(ctx)), ctx->env);

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
            ctx->send(error_response(ctx, id, invalid_request(ctx, DY_STR_LIT("Missing 'params' field."))), ctx->env);
            return true;
        }

        struct dy_json_object params_obj;
        if (!json_force_object(params, &params_obj)) {
            ctx->send(error_response(ctx, id, invalid_request(ctx, DY_STR_LIT("'params' is not an object."))), ctx->env);
            return true;
        }

        dy_json_t text_document;
        if (!json_get_member(params_obj, DY_STR_LIT("textDocument"), &text_document)) {
            ctx->send(error_response(ctx, id, invalid_request(ctx, DY_STR_LIT("Missing 'textDocument' field."))), ctx->env);
            return true;
        }

        struct dy_json_object text_document_obj;
        if (!json_force_object(text_document, &text_document_obj)) {
            ctx->send(error_response(ctx, id, invalid_request(ctx, DY_STR_LIT("'textDocument' is not an object."))), ctx->env);
            return true;
        }

        dy_json_t uri;
        if (!json_get_member(text_document_obj, DY_STR_LIT("uri"), &uri)) {
            ctx->send(error_response(ctx, id, invalid_request(ctx, DY_STR_LIT("Missing 'uri' field."))), ctx->env);
            return true;
        }

        dy_string_t uri_string;
        if (!json_force_string(uri, &uri_string)) {
            ctx->send(error_response(ctx, id, invalid_request(ctx, DY_STR_LIT("'uri' is not a string."))), ctx->env);
            return true;
        }

        dy_json_t position;
        if (!json_get_member(params_obj, DY_STR_LIT("position"), &position)) {
            ctx->send(error_response(ctx, id, invalid_request(ctx, DY_STR_LIT("Missing 'position' field."))), ctx->env);
            return true;
        }

        struct dy_json_object position_obj;
        if (!json_force_object(position, &position_obj)) {
            ctx->send(error_response(ctx, id, invalid_request(ctx, DY_STR_LIT("'position' is not a string."))), ctx->env);
            return true;
        }

        dy_json_t line;
        if (!json_get_member(position_obj, DY_STR_LIT("line"), &line)) {
            ctx->send(error_response(ctx, id, invalid_request(ctx, DY_STR_LIT("Missing 'line' field."))), ctx->env);
            return true;
        }

        long line_number;
        if (!json_force_integer(line, &line_number)) {
            ctx->send(error_response(ctx, id, invalid_request(ctx, DY_STR_LIT("'line' is not a number."))), ctx->env);
            return true;
        }

        dy_json_t character;
        if (!json_get_member(position_obj, DY_STR_LIT("character"), &character)) {
            ctx->send(error_response(ctx, id, invalid_request(ctx, DY_STR_LIT("Missing 'character' field."))), ctx->env);
            return true;
        }

        long character_number;
        if (!json_force_integer(character, &character_number)) {
            ctx->send(error_response(ctx, id, invalid_request(ctx, DY_STR_LIT("'character' is not a number."))), ctx->env);
            return true;
        }

        dy_json_t result, error;
        if (dy_lsp_hover(ctx, uri_string, line_number, character_number, &result, &error)) {
            ctx->send(success_response(ctx, id, result), ctx->env);
        } else {
            ctx->send(error_response(ctx, id, error), ctx->env);
        }

        return true;
    }

    if (have_id) {
        ctx->send(error_response(ctx, id, method_not_found(ctx, DY_STR_LIT("Unknown method name."))), ctx->env);
    }

    return true;
}

bool dy_lsp_initialize(dy_lsp_ctx_t *ctx, dy_json_t *result, dy_json_t *error)
{
    if (ctx->is_initialized) {
        *error = invalid_request(ctx, DY_STR_LIT("Already initialized."));
        return false;
    }

    ctx->is_initialized = true;

    *result = initialize_response(ctx);

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
        .running_id = 0
    };

    process_document(&doc);

    dy_array_add(ctx->documents, &doc);
}

void dy_lsp_did_close(dy_lsp_ctx_t *ctx, dy_string_t uri)
{
    for (size_t i = 0, size = dy_array_size(ctx->documents); i < size; ++i) {
        struct document doc;
        dy_array_get(ctx->documents, i, &doc);

        if (dy_string_are_equal(doc.uri, uri)) {
            dy_array_remove(ctx->documents, i);
            break;
        }
    }
}

void dy_lsp_did_change(dy_lsp_ctx_t *ctx, dy_string_t uri, struct dy_json_array content_changes)
{
    for (size_t i = 0, size = dy_array_size(ctx->documents); i < size; ++i) {
        struct document *doc = dy_array_get_ptr(ctx->documents, i);

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

            process_document(doc);
        }

        break;
    }
}

bool dy_lsp_hover(dy_lsp_ctx_t *ctx, dy_string_t uri, long line_number, long utf16_char_offset, dy_json_t *result, dy_json_t *error)
{
    for (size_t i = 0, size = dy_array_size(ctx->documents); i < size; ++i) {
        struct document *doc = dy_array_get_ptr(ctx->documents, i);

        if (!dy_string_are_equal(doc->uri, uri)) {
            continue;
        }

        if (doc->core_is_valid) {
            size_t byte_offset;
            if (!compute_byte_offset(doc->text, line_number, utf16_char_offset, &byte_offset)) {
                *error = invalid_request(ctx, DY_STR_LIT("Could'nt compute byte offset."));
                return false;
            }

            struct dy_core_expr type;
            if (!type_of_expr_at_byte_offset(&doc->running_id, doc->core, byte_offset, &type)) {
                *error = invalid_request(ctx, DY_STR_LIT("'position' is not a string."));
                return false;
            }

            dy_array_t *type_string_storage = dy_array_create(dy_allocator_stdlib(), sizeof(char), 64);
            dy_core_expr_to_string(type, type_string_storage);

            dy_string_t type_string = {
                .ptr = dy_array_buffer(type_string_storage),
                .size = dy_array_size(type_string_storage)
            };

            *result = hover_result(ctx, type_string);
            return true;
        }

        break;
    }

    *error = invalid_request(ctx, DY_STR_LIT("Could not find the document."));
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

dy_json_t initialize_response(dy_lsp_ctx_t *ctx)
{
    struct dy_json_member capabilities = {
        .string = DY_STR_LIT("capabilities"),
        .value = server_capabilities(ctx)
    };

    struct dy_json_member m_server_info = {
        .string = DY_STR_LIT("serverInfo"),
        .value = server_info(ctx)
    };

    dy_array_t *members = dy_array_create(ctx->allocator, sizeof(struct dy_json_member), 2);
    dy_array_add(members, &capabilities);
    dy_array_add(members, &m_server_info);

    return (dy_json_t){
        .tag = DY_JSON_VALUE_OBJECT,
        .object = {
            .members = dy_array_buffer(members),
            .num_members = dy_array_size(members),
        }
    };
}

dy_json_t invalid_request(dy_lsp_ctx_t *ctx, dy_string_t message)
{
    return response_error(ctx, -32600, message);
}

dy_json_t method_not_found(dy_lsp_ctx_t *ctx, dy_string_t message)
{
    return response_error(ctx, -32601, message);
}

dy_json_t response_error(dy_lsp_ctx_t *ctx, long code, dy_string_t message)
{
    struct dy_json_member m_code = {
        .string = DY_STR_LIT("code"),
        .value = dy_json_integer(code)
    };

    struct dy_json_member m_message = {
        .string = DY_STR_LIT("message"),
        .value = dy_json_string(message)
    };

    dy_array_t *members = dy_array_create(ctx->allocator, sizeof(struct dy_json_member), 2);
    dy_array_add(members, &m_code);
    dy_array_add(members, &m_message);

    return (dy_json_t){
        .tag = DY_JSON_VALUE_OBJECT,
        .object = {
            .members = dy_array_buffer(members),
            .num_members = dy_array_size(members),
        }
    };
}

dy_json_t server_capabilities(dy_lsp_ctx_t *ctx)
{
    struct dy_json_member text_document_sync = {
        .string = DY_STR_LIT("textDocumentSync"),
        .value = text_document_sync_options(ctx)
    };

    struct dy_json_member hover_provider = {
        .string = DY_STR_LIT("hoverProvider"),
        .value = dy_json_true()
    };

    dy_array_t *members = dy_array_create(dy_allocator_stdlib(), sizeof(struct dy_json_member), 2);
    dy_array_add(members, &text_document_sync);
    dy_array_add(members, &hover_provider);

    return (dy_json_t){
        .tag = DY_JSON_VALUE_OBJECT,
        .object = {
            .members = dy_array_buffer(members),
            .num_members = dy_array_size(members),
        }
    };
}

dy_json_t server_info(dy_lsp_ctx_t *ctx)
{
    struct dy_json_member name = {
        .string = DY_STR_LIT("name"),
        .value = dy_json_string(DY_STR_LIT("Duality"))
    };

    struct dy_json_member version = {
        .string = DY_STR_LIT("version"),
        .value = dy_json_string(DY_STR_LIT("0.0.1"))
    };

    dy_array_t *members = dy_array_create(ctx->allocator, sizeof(struct dy_json_member), 2);
    dy_array_add(members, &name);
    dy_array_add(members, &version);

    return (dy_json_t){
        .tag = DY_JSON_VALUE_OBJECT,
        .object = {
            .members = dy_array_buffer(members),
            .num_members = dy_array_size(members),
        }
    };
}

dy_json_t text_document_sync_options(dy_lsp_ctx_t *ctx)
{
    struct dy_json_member open_close = {
        .string = DY_STR_LIT("openClose"),
        .value = dy_json_true()
    };

    struct dy_json_member change = {
        .string = DY_STR_LIT("change"),
        .value = dy_json_integer(1)
    };

    dy_array_t *members = dy_array_create(ctx->allocator, sizeof(struct dy_json_member), 2);
    dy_array_add(members, &open_close);
    dy_array_add(members, &change);

    return (dy_json_t){
        .tag = DY_JSON_VALUE_OBJECT,
        .object = {
            .members = dy_array_buffer(members),
            .num_members = dy_array_size(members),
        }
    };
}

dy_json_t error_response(dy_lsp_ctx_t *ctx, dy_json_t id, dy_json_t error)
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

    dy_array_t *members = dy_array_create(ctx->allocator, sizeof(struct dy_json_member), 3);
    dy_array_add(members, &jsonrpc);
    dy_array_add(members, &m_id);
    dy_array_add(members, &m_error);

    return (dy_json_t){
        .tag = DY_JSON_VALUE_OBJECT,
        .object = {
            .members = dy_array_buffer(members),
            .num_members = dy_array_size(members),
        }
    };
}

dy_json_t success_response(dy_lsp_ctx_t *ctx, dy_json_t id, dy_json_t result)
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

    dy_array_t *members = dy_array_create(ctx->allocator, sizeof(struct dy_json_member), 3);
    dy_array_add(members, &jsonrpc);
    dy_array_add(members, &m_id);
    dy_array_add(members, &m_result);

    return (dy_json_t){
        .tag = DY_JSON_VALUE_OBJECT,
        .object = {
            .members = dy_array_buffer(members),
            .num_members = dy_array_size(members),
        }
    };
}

dy_json_t hover_result(dy_lsp_ctx_t *ctx, dy_string_t contents)
{
    struct dy_json_member m_contents = {
        .string = DY_STR_LIT("contents"),
        .value = dy_json_string(contents)
    };

    dy_array_t *members = dy_array_create(dy_allocator_stdlib(), sizeof(struct dy_json_member), 1);
    dy_array_add(members, &m_contents);

    return (dy_json_t){
        .tag = DY_JSON_VALUE_OBJECT,
        .object = {
            .members = dy_array_buffer(members),
            .num_members = dy_array_size(members),
        }
    };
}

void process_document(struct document *doc)
{
    struct dy_parser_ctx parser_ctx = {
        .stream = stream_from_string(dy_allocator_stdlib(), doc->text),
        .allocator = dy_allocator_stdlib(),
        .arrays = dy_array_create(dy_allocator_stdlib(), sizeof(dy_array_t *), 32)
    };

    struct dy_ast_do_block ast;
    bool parsing_succeeded = dy_parse_file(&parser_ctx, &ast);
    dy_array_destroy(parser_ctx.stream.buffer);
    if (!parsing_succeeded) {
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
