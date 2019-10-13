/*
 * Copyright 2017-2019 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_ASSERT_H
#define DY_ASSERT_H

#include <duality/support/util.h>
#include <duality/support/log.h>

DY_SUPPORT_API DY_NORETURN void dy_fatal(const char *f, int l, const char *func, const char *expr);

#define dy_assert(x) (x) ? (void)0 : dy_fatal(__FILE__, __LINE__, __func__, #x)

#define dy_bail(x) dy_fatal(__FILE__, __LINE__, __func__, #x)

#define DY_IMPOSSIBLE_ENUM() dy_bail("Impossible enum value")

#endif // DY_ASSERT_H
