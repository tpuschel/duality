/*
 * Copyright 2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_LSP_H
#define DY_LSP_H

#include <duality/lsp/api.h>

#include <duality/support/allocator.h>
#include <duality/support/json.h>

typedef struct dy_lsp_ctx dy_lsp_ctx_t;

typedef void (*dy_lsp_send_fn)(dy_json_t message, void *env);

DY_LSP_API dy_lsp_ctx_t *dy_lsp_create(struct dy_allocator allocator, dy_lsp_send_fn send, void *env);

DY_LSP_API bool dy_lsp_handle_message(dy_lsp_ctx_t *ctx, dy_json_t message);

DY_LSP_API bool dy_lsp_initialize(dy_lsp_ctx_t *ctx, dy_json_t *result, dy_json_t *error);

DY_LSP_API void dy_lsp_initialized(dy_lsp_ctx_t *ctx);

DY_LSP_API dy_json_t dy_lsp_shutdown(dy_lsp_ctx_t *ctx);

DY_LSP_API void dy_lsp_did_open(dy_lsp_ctx_t *ctx, dy_string_t uri, dy_string_t text);

DY_LSP_API void dy_lsp_did_close(dy_lsp_ctx_t *ctx, dy_string_t uri);

DY_LSP_API void dy_lsp_did_change(dy_lsp_ctx_t *ctx, dy_string_t uri, struct dy_json_array content_changes);

DY_LSP_API bool dy_lsp_hover(dy_lsp_ctx_t *ctx, dy_string_t uri, long line_number, long utf16_char_offset, dy_json_t *result, dy_json_t *error);

DY_LSP_API void dy_lsp_exit(dy_lsp_ctx_t *ctx);

DY_LSP_API int dy_lsp_exit_code(dy_lsp_ctx_t *ctx);

DY_LSP_API void dy_lsp_destroy(dy_lsp_ctx_t *ctx);

#endif // DY_LSP_H
