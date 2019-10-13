/*
 * Copyright 2017-2019 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <duality/support/assert.h>

#include <stdio.h>
#include <stdlib.h>

void dy_fatal(const char *f, int l, const char *func, const char *expr)
{
    fprintf(stderr, "%s:%d: error in function %s: violated condition: %s\n", f, l, func, expr);

    fprintf(stderr, "[INFO] Backtraces are not available.\n");

    abort();
}
