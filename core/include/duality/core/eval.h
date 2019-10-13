/*
 * Copyright 2017-2019 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_EVAL_H
#define DY_EVAL_H

#include <duality/core/check.h>

DY_CORE_API dy_ternary_t dy_eval_expr(struct dy_check_ctx ctx, struct dy_core_expr expr, struct dy_core_expr *new_expr);

DY_CORE_API dy_ternary_t dy_eval_value_map(struct dy_check_ctx ctx, struct dy_core_value_map value_map, struct dy_core_expr *new_expr);

DY_CORE_API dy_ternary_t dy_eval_type_map(struct dy_check_ctx ctx, struct dy_core_type_map type_map, struct dy_core_expr *new_expr);

DY_CORE_API dy_ternary_t dy_eval_value_map_elim(struct dy_check_ctx ctx, struct dy_core_value_map_elim elim, struct dy_core_expr *new_expr);

DY_CORE_API dy_ternary_t dy_eval_type_map_elim(struct dy_check_ctx ctx, struct dy_core_type_map_elim elim, struct dy_core_expr *new_expr);

DY_CORE_API dy_ternary_t dy_eval_both(struct dy_check_ctx ctx, struct dy_core_pair both, struct dy_core_expr *new_expr);

DY_CORE_API dy_ternary_t dy_eval_one_of(struct dy_check_ctx ctx, struct dy_core_pair one_of, struct dy_core_expr *new_expr);

DY_CORE_API dy_ternary_t dy_eval_any_of(struct dy_check_ctx ctx, struct dy_core_pair any_of, struct dy_core_expr *new_expr);

#endif // DY_EVAL_H
