/*
 * Copyright 2017-2019 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_IS_SUBTYPE_H
#define DY_IS_SUBTYPE_H

#include <duality/core/check.h>

DY_CORE_API dy_ternary_t dy_is_subtype(struct dy_check_ctx ctx, struct dy_core_expr subtype, struct dy_core_expr supertype);

#endif // DY_IS_SUBTYPE_H
