/*
 * Copyright 2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_LSP_API_H
#define DY_LSP_API_H

#ifdef DY_LSP_EXPORT
#    ifdef _WIN32
#        define DY_LSP_API __declspec(dllexport)
#    else
#        define DY_LSP_API __attribute__((visibility("default")))
#    endif
#elif defined DY_LSP_IMPORT && defined _WIN32
#    define DY_LSP_API __declspec(dllimport)
#elif defined __EMSCRIPTEN__
#    define DY_LSP_API __attribute__((used))
#else
#    define DY_LSP_API
#endif

#endif // DY_LSP_API_H
