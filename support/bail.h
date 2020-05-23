/*
 * Copyright 2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_BAIL_H
#define DY_BAIL_H

#include "util.h"

#ifndef DY_FREESTANDING
#    include <stdio.h>
#    include <stdlib.h>

#    define dy_bail(msg)                                                                               \
        do {                                                                                           \
            fprintf(stderr, "%s:%d in %s: Aborting. Cause: %s\n", __FILE__, __LINE__, __func__, #msg); \
            abort();                                                                                   \
        } while (0)

#endif // !DY_FREESTANDING

#endif // DY_BAIL_H
