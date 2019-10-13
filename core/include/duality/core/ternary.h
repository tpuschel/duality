/*
 * Copyright 2017-2019 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_TERNARY_H
#define DY_TERNARY_H

#include <duality/core/api.h>

typedef enum dy_ternary {
    DY_YES,
    DY_NO,
    DY_MAYBE
} dy_ternary_t;

DY_CORE_API dy_ternary_t dy_ternary_conjunction(dy_ternary_t t1, dy_ternary_t t2);

DY_CORE_API dy_ternary_t dy_ternary_disjunction(dy_ternary_t t1, dy_ternary_t t2);

#endif // DY_TERNARY_H
