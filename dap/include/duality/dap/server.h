/*
 * Copyright 2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_DAP_SERVER_H
#define DY_DAP_SERVER_H

#include <duality/dap/dap.h>

#include <duality/support/stream.h>

#include <stdio.h>

struct dy_dap_stream_env {
    FILE *file;
    bool is_bounded;
    size_t bound_size_in_bytes;
};

DY_DAP_API int dy_dap_run_server(FILE *in, FILE *out);

DY_DAP_API bool dy_dap_process_message(struct dy_stream *stream);

DY_DAP_API bool dy_dap_read_content_length(struct dy_stream *stream, size_t *content_length_in_bytes);

#endif // DY_LSP_SERVER_H
