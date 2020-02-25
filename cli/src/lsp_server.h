/*
 * Copyright 2019-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_LSP_SERVER_H
#define DY_LSP_SERVER_H

#include <duality/support/stream.h>
#include <duality/support/string.h>

int run_lsp_server(void);

struct dy_stream stream_from_string(struct dy_allocator allocator, dy_string_t s);

#endif // DY_LSP_SERVER_H
