/*
 * Copyright 2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_CORE_CTX_H
#define DY_CORE_CTX_H

#include <duality/core/core.h>

#include <duality/support/obj_pool.h>

struct dy_bound_constraint {
    size_t id;
    struct dy_core_expr type;
    dy_array_t *binding_ids;
};

struct dy_core_ctx {
    size_t *running_id;
    dy_obj_pool_t *expr_pool;
    dy_array_t *bound_constraints;
};

#endif // DY_CORE_CTX_H
