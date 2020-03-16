/*
 * Copyright 2019-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_UTF8_TO_JSON_H
#define DY_UTF8_TO_JSON_H

#include <duality/support/json.h>

#include <duality/support/stream.h>

DY_SUPPORT_API bool dy_utf8_to_json(struct dy_stream *stream, dy_json_t *json);

#endif // DY_UTF8_TO_JSON_H
