/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
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
    dy_array_t *successful_elims;
};

struct dy_constraint;

DY_CORE_API bool dy_check_expr(struct dy_check_ctx ctx, struct dy_core_expr expr, struct dy_core_expr *new_expr, struct dy_constraint *constraint, bool *did_generate_constraint);

DY_CORE_API bool dy_check_value_map(struct dy_check_ctx ctx, struct dy_core_value_map value_map, struct dy_core_value_map *new_value_map, struct dy_constraint *constraint, bool *did_generate_constraint);

DY_CORE_API bool dy_check_type_map(struct dy_check_ctx ctx, struct dy_core_type_map type_map, struct dy_core_expr *new_expr, struct dy_constraint *constraint, bool *did_generate_constraint);

DY_CORE_API bool dy_check_value_map_elim(struct dy_check_ctx ctx, struct dy_core_value_map_elim elim, struct dy_core_value_map_elim *new_elim, struct dy_constraint *constraint, bool *did_generate_constraint);

DY_CORE_API bool dy_check_type_map_elim(struct dy_check_ctx ctx, struct dy_core_type_map_elim elim, struct dy_core_type_map_elim *new_elim, struct dy_constraint *constraint, bool *did_generate_constraint);

DY_CORE_API bool dy_check_both(struct dy_check_ctx ctx, struct dy_core_both both, struct dy_core_both *new_both, struct dy_constraint *constraint, bool *did_generate_constraint);

DY_CORE_API bool dy_check_one_of(struct dy_check_ctx ctx, struct dy_core_one_of one_of, struct dy_core_expr *new_expr, struct dy_constraint *constraint, bool *did_generate_constraint);

#endif // DY_CHECK_H
