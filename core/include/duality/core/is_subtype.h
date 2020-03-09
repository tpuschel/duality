/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_IS_SUBTYPE_H
#define DY_IS_SUBTYPE_H

#include <duality/core/check.h>
#include <duality/core/constraint.h>

DY_CORE_API dy_ternary_t dy_is_subtype(struct dy_check_ctx ctx, struct dy_core_expr subtype, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr);

DY_CORE_API dy_ternary_t dy_is_subtype_sub(struct dy_check_ctx ctx, struct dy_core_expr subtype, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint, struct dy_core_expr subtype_expr, struct dy_core_expr *new_subtype_expr, bool *did_transform_subtype_expr);

DY_CORE_API dy_ternary_t dy_is_subtype_no_transformation(struct dy_check_ctx ctx, struct dy_core_expr subtype, struct dy_core_expr supertype, struct dy_constraint *constraint, bool *did_generate_constraint);

#endif // DY_IS_SUBTYPE_H