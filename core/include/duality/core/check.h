/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_CHECK_H
#define DY_CHECK_H

#include <duality/core/api.h>
#include <duality/core/ternary.h>
#include <duality/core/constraint.h>
#include <duality/core/ctx.h>

#include <duality/support/array.h>

DY_CORE_API bool dy_check_expr(struct dy_core_ctx ctx, struct dy_core_expr expr, struct dy_core_expr *new_expr, struct dy_constraint *constraint, bool *did_generate_constraint);

DY_CORE_API bool dy_check_expr_map(struct dy_core_ctx ctx, struct dy_core_expr_map expr_map, struct dy_core_expr_map *new_expr_map, struct dy_constraint *constraint, bool *did_generate_constraint);

DY_CORE_API bool dy_check_type_map(struct dy_core_ctx ctx, struct dy_core_type_map type_map, struct dy_core_type_map *new_type_map, struct dy_constraint *constraint, bool *did_generate_constraint);

DY_CORE_API bool dy_check_expr_map_elim(struct dy_core_ctx ctx, struct dy_core_expr_map_elim elim, struct dy_core_expr_map_elim *new_elim, struct dy_constraint *constraint, bool *did_generate_constraint);

DY_CORE_API bool dy_check_type_map_elim(struct dy_core_ctx ctx, struct dy_core_type_map_elim elim, struct dy_core_type_map_elim *new_elim, struct dy_constraint *constraint, bool *did_generate_constraint);

DY_CORE_API bool dy_check_both(struct dy_core_ctx ctx, struct dy_core_both both, struct dy_core_both *new_both, struct dy_constraint *constraint, bool *did_generate_constraint);

DY_CORE_API bool dy_check_one_of(struct dy_core_ctx ctx, struct dy_core_one_of one_of, struct dy_core_expr *new_expr, struct dy_constraint *constraint, bool *did_generate_constraint);

DY_CORE_API bool dy_check_inference_ctx(struct dy_core_ctx ctx, struct dy_core_inference_ctx inference_ctx, struct dy_core_expr *new_expr, struct dy_constraint *constraint, bool *did_generate_constraint);

DY_CORE_API bool dy_check_recursion(struct dy_core_ctx ctx, struct dy_core_recursion recursion, struct dy_core_recursion *new_recursion, struct dy_constraint *constraint, bool *did_generate_constraint);

#endif // DY_CHECK_H
