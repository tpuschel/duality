/*
 * Copyright 2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_CONSTRAINT_H
#define DY_CONSTRAINT_H

#include <duality/core/check.h>

struct dy_constraint_single {
    size_t id;
    enum dy_core_polarity polarity_position;
    struct dy_core_expr expr;
};

struct dy_constraint_multiple {
    const struct dy_constraint *c1;
    const struct dy_constraint *c2;
    enum dy_core_polarity polarity;
};

enum dy_constraint_tag {
    DY_CONSTRAINT_SINGLE,
    DY_CONSTRAINT_MULTIPLE
};

struct dy_constraint {
    union {
        struct dy_constraint_single single;
        struct dy_constraint_multiple multiple;
    };

    enum dy_constraint_tag tag;
};

bool dy_constraint_solve(struct dy_check_ctx ctx, struct dy_constraint constraint, size_t id, struct dy_core_expr *subtype, bool *have_subtype, struct dy_core_expr *supertype, bool *have_supertype, struct dy_constraint *new_constraint, bool *have_new_constraint);

#endif // DY_CONSTRAINT_H
