/*
 * Copyright 2017-2019 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <duality/support/assert.h>

#include <stdio.h>
#include <stdlib.h>

#ifdef DY_HAVE_EXECINFO
#    include <execinfo.h>
#    include <unistd.h>
#endif

static void print_backtrace(int depth);

void dy_fatal(int depth, const char *f, int l, const char *func, const char *expr)
{
    ++depth;

    fprintf(stderr, "%s:%d: error in function %s: violated condition: %s\n", f, l, func, expr);

    print_backtrace(depth);

    abort();
}

void print_backtrace(int depth)
{
    ++depth;

    fprintf(stderr, "Backtrace:\n");

#ifdef DY_HAVE_EXECINFO
    void *stack[256];
    int size = backtrace(stack, sizeof stack / sizeof *stack);
    backtrace_symbols_fd(stack + depth, size - depth, STDERR_FILENO);
#else
    fprintf(stderr, "Backtraces are not available.\n");
#endif
}
