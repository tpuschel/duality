/*
 * Copyright 2017-2019 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <duality/core/ternary.h>

dy_ternary_t dy_ternary_conjunction(dy_ternary_t t1, dy_ternary_t t2)
{
    if (t1 == DY_NO || t2 == DY_NO) {
        return DY_NO;
    }

    if (t1 == DY_MAYBE || t2 == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_YES;
}

dy_ternary_t dy_ternary_disjunction(dy_ternary_t t1, dy_ternary_t t2)
{
    if (t1 == DY_YES || t2 == DY_YES) {
        return DY_YES;
    }

    if (t1 == DY_MAYBE || t2 == DY_MAYBE) {
        return DY_MAYBE;
    }

    return DY_NO;
}
