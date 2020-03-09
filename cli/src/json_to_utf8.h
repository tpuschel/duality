/*
 * Copyright 2019-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_JSON_TO_UTF8_H
#define DY_JSON_TO_UTF8_H

#include "json.h"

#include <duality/support/array.h>

void json_to_utf8(dy_json_t json, dy_array_t *utf8);

#endif // DY_JSON_TO_UTF8_H