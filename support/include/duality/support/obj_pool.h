/*
 * Copyright 2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef DY_OBJ_POOL_H
#define DY_OBJ_POOL_H

#include <duality/support/api.h>

#include <stddef.h>

typedef struct dy_obj_pool dy_obj_pool_t;

typedef int (*dy_obj_pool_is_parent_cb)(const void *parent, const void *child);

DY_SUPPORT_API dy_obj_pool_t *dy_obj_pool_create(size_t obj_size, size_t obj_align);

DY_SUPPORT_API void *dy_obj_pool_alloc(dy_obj_pool_t *pool);

DY_SUPPORT_API void *dy_obj_pool_new(dy_obj_pool_t *pool, const void *ptr);

DY_SUPPORT_API void *dy_obj_pool_retain(dy_obj_pool_t *pool, const void *ptr);

DY_SUPPORT_API size_t dy_obj_pool_release(dy_obj_pool_t *pool, const void *ptr);

DY_SUPPORT_API void dy_obj_pool_set_is_parent_cb(dy_obj_pool_t *pool, dy_obj_pool_is_parent_cb cb);

DY_SUPPORT_API void dy_obj_pool_destroy(dy_obj_pool_t *pool);

#endif // DY_OBJ_POOL_H
