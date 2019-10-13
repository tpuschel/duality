/*
 * Copyright 2017-2019 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_CHECK_H
#define DY_CHECK_H

#include <duality/core/api.h>
#include <duality/core/core.h>
#include <duality/core/ternary.h>

#include <duality/support/allocator.h>
#include <duality/support/array.h>

struct dy_check_ctx {
    size_t *running_id;
    struct dy_allocator allocator;
};

DY_CORE_API dy_ternary_t dy_check_expr(struct dy_check_ctx ctx, struct dy_core_expr expr, struct dy_core_expr *new_expr);

DY_CORE_API dy_ternary_t dy_check_value_map(struct dy_check_ctx ctx, struct dy_core_value_map value_map, struct dy_core_value_map *new_value_map);

DY_CORE_API dy_ternary_t dy_check_type_map(struct dy_check_ctx ctx, struct dy_core_type_map type_map, struct dy_core_type_map *new_type_map);

DY_CORE_API dy_ternary_t dy_check_value_map_elim(struct dy_check_ctx ctx, struct dy_core_value_map_elim elim, struct dy_core_value_map_elim *new_elim);

DY_CORE_API dy_ternary_t dy_check_type_map_elim(struct dy_check_ctx ctx, struct dy_core_type_map_elim elim, struct dy_core_type_map_elim *new_elim);

DY_CORE_API dy_ternary_t dy_check_both(struct dy_check_ctx ctx, struct dy_core_pair both, struct dy_core_pair *new_both);

DY_CORE_API dy_ternary_t dy_check_one_of(struct dy_check_ctx ctx, struct dy_core_pair one_of, struct dy_core_expr *new_expr);

DY_CORE_API dy_ternary_t dy_check_any_of(struct dy_check_ctx ctx, struct dy_core_pair any_of, struct dy_core_pair *new_any_of);

#endif // DY_CHECK_H
