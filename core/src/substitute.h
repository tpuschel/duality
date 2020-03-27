/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_SUBSTITUTE_H
#define DY_SUBSTITUTE_H

#include <duality/core/core.h>

struct dy_core_expr substitute(size_t id, struct dy_core_expr sub, struct dy_core_expr expr);

#endif // DY_SUBSTITUTE_H
