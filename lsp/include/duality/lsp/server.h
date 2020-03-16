/*
 * Copyright 2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_LSP_SERVER_H
#define DY_LSP_SERVER_H

#include <duality/lsp/lsp.h>

#include <duality/support/stream.h>

#include <stdio.h>

struct dy_lsp_stream_env {
    FILE *file;
    bool is_bounded;
    size_t bound_size_in_bytes;
};

DY_LSP_API int dy_lsp_run_server(FILE *in, FILE *out);

DY_LSP_API bool dy_lsp_process_message(dy_lsp_ctx_t *ctx, struct dy_stream *stream);

DY_LSP_API bool dy_lsp_read_content_length(struct dy_stream *stream, size_t *content_length_in_bytes);

#endif // DY_LSP_SERVER_H
