/*
 * Copyright 2017-2019 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_CORE_API_H
#define DY_CORE_API_H

#ifdef DY_CORE_EXPORT
#    ifdef _WIN32
#        define DY_CORE_API __declspec(dllexport)
#    else
#        define DY_CORE_API __attribute__((visibility("default")))
#    endif
#elif defined DY_CORE_IMPORT && defined _WIN32
#    define DY_CORE_API __declspec(dllimport)
#elif defined __EMSCRIPTEN__
#    define DY_CORE_API __attribute__((used))
#else
#    define DY_CORE_API
#endif

#endif // DY_CORE_API_H
