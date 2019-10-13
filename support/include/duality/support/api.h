/*
 * Copyright 2017-2019 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_SUPPORT_API_H
#define DY_SUPPORT_API_H

#ifdef DY_SUPPORT_EXPORT
#    ifdef _WIN32
#        define DY_SUPPORT_API __declspec(dllexport)
#    else
#        define DY_SUPPORT_API __attribute__((visibility("default")))
#    endif
#elif defined DY_SUPPORT_IMPORT && defined _WIN32
#    define DY_SUPPORT_API __declspec(dllimport)
#elif defined __EMSCRIPTEN__
#    define DY_SUPPORT_API __attribute__((used))
#else
#    define DY_SUPPORT_API
#endif

#endif // DY_SUPPORT_API_H
