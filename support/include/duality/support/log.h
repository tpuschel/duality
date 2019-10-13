/*
 * Copyright 2017-2019 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_LOG_H
#define DY_LOG_H

#include <duality/support/string.h>

struct dy_logger {
    void (*log)(dy_string_t msg, void *env);
    void *env;
};

#endif // DY_LOG_H
