/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core.h"

#include "../support/util.h"

/**
 * This file implements substitution for every object of Core.
 */

static inline bool substitute(struct dy_core_ctx *ctx, struct dy_core_expr expr, size_t id, struct dy_core_expr sub, struct dy_core_expr *result);

static inline bool substitute_equality_map(struct dy_core_ctx *ctx, struct dy_core_equality_map equality_map, size_t id, struct dy_core_expr sub, struct dy_core_equality_map *result);

static inline bool substitute_type_map(struct dy_core_ctx *ctx, struct dy_core_type_map inference_type_map, size_t id, struct dy_core_expr sub, struct dy_core_type_map *result);

static inline bool substitute_inference_type_map(struct dy_core_ctx *ctx, struct dy_core_inference_type_map type_map, size_t id, struct dy_core_expr sub, struct dy_core_inference_type_map *result);

static inline bool substitute_recursion(struct dy_core_ctx *ctx, struct dy_core_recursion recursion, size_t id, struct dy_core_expr sub, struct dy_core_recursion *result);

bool substitute(struct dy_core_ctx *ctx, struct dy_core_expr expr, size_t id, struct dy_core_expr sub, struct dy_core_expr *result)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_CUSTOM:
        return expr.custom.substitute(expr.custom.data, ctx, id, sub, result);
    case DY_CORE_EXPR_END:
        // fallthrough
    case DY_CORE_EXPR_SYMBOL:
        return false;
    case DY_CORE_EXPR_EQUALITY_MAP:
        if (substitute_equality_map(ctx, expr.equality_map, id, sub, &expr.equality_map)) {
            *result = expr;
            return true;
        } else {
            return false;
        }
    case DY_CORE_EXPR_TYPE_MAP:
        if (substitute_type_map(ctx, expr.type_map, id, sub, &expr.type_map)) {
            *result = expr;
            return true;
        } else {
            return false;
        }
    case DY_CORE_EXPR_VARIABLE:
        if (expr.variable_id == id) {
            *result = dy_core_expr_retain(sub);
            return true;
        }
        
        for (size_t i = 0, size = ctx->equal_variables.num_elems; i < size; ++i) {
            struct dy_equal_variables v;
            dy_array_get(ctx->equal_variables, i, &v);
            
            if (v.id1 == expr.variable_id) {
                expr.variable_id = v.id2;
                *result = expr;
                return true;
            }
        }
        
        return false;
    case DY_CORE_EXPR_JUNCTION: {
        struct dy_core_expr e1;
        bool e1_is_new = substitute(ctx, *expr.junction.e1, id, sub, &e1);
        
        struct dy_core_expr e2;
        bool e2_is_new = substitute(ctx, *expr.junction.e2, id, sub, &e2);
        
        if (!e1_is_new && !e2_is_new) {
            return false;
        }
        
        if (e1_is_new) {
            expr.junction.e1 = dy_core_expr_new(e1);
        } else {
            dy_core_expr_retain_ptr(expr.junction.e1);
        }
        
        if (e2_is_new) {
            expr.junction.e2 = dy_core_expr_new(e2);
        } else {
            dy_core_expr_retain_ptr(expr.junction.e2);
        }
        
        *result = expr;
        return true;
    }
    case DY_CORE_EXPR_ALTERNATIVE: {
        struct dy_core_expr first;
        bool first_is_new = substitute(ctx, *expr.alternative.first, id, sub, &first);
        
        struct dy_core_expr second;
        bool second_is_new = substitute(ctx, *expr.alternative.second, id, sub, &second);
        
        if (!first_is_new && !second_is_new) {
            return false;
        }
        
        if (first_is_new) {
            expr.alternative.first = dy_core_expr_new(first);
        } else {
            dy_core_expr_retain_ptr(expr.alternative.first);
        }
        
        if (second_is_new) {
            expr.alternative.second = dy_core_expr_new(second);
        } else {
            dy_core_expr_retain_ptr(expr.alternative.second);
        }
        
        *result = expr;
        return true;
    }
    case DY_CORE_EXPR_EQUALITY_MAP_ELIM: {
        struct dy_core_expr e;
        bool e_is_new = substitute(ctx, *expr.equality_map_elim.expr, id, sub, &e);
        
        struct dy_core_equality_map equality_map;
        bool equality_map_is_new = substitute_equality_map(ctx, expr.equality_map_elim.map, id, sub, &equality_map);
        
        if (!e_is_new && !equality_map_is_new) {
            return false;
        }
        
        if (e_is_new) {
            expr.equality_map_elim.expr = dy_core_expr_new(e);
        } else {
            dy_core_expr_retain_ptr(expr.equality_map_elim.expr);
        }
        
        if (equality_map_is_new) {
            expr.equality_map_elim.map = equality_map;
        } else {
            dy_core_expr_retain_ptr(expr.equality_map_elim.map.e1);
            dy_core_expr_retain_ptr(expr.equality_map_elim.map.e2);
        }
        
        *result = expr;
        return true;
    }
    case DY_CORE_EXPR_TYPE_MAP_ELIM: {
        struct dy_core_expr e;
        bool e_is_new = substitute(ctx, *expr.type_map_elim.expr, id, sub, &e);
        
        struct dy_core_type_map type_map;
        bool type_map_is_new = substitute_type_map(ctx, expr.type_map_elim.map, id, sub, &type_map);
        
        if (!e_is_new && !type_map_is_new) {
            return false;
        }
        
        if (e_is_new) {
            expr.type_map_elim.expr = dy_core_expr_new(e);
        } else {
            dy_core_expr_retain_ptr(expr.type_map_elim.expr);
        }
        
        if (type_map_is_new) {
            expr.type_map_elim.map = type_map;
        } else {
            dy_core_expr_retain_ptr(expr.type_map_elim.map.type);
            dy_core_expr_retain_ptr(expr.type_map_elim.map.expr);
        }
        
        *result = expr;
        return true;
    }
    case DY_CORE_EXPR_INFERENCE_TYPE_MAP:
        if (substitute_inference_type_map(ctx, expr.inference_type_map, id, sub, &expr.inference_type_map)) {
            *result = expr;
            return true;
        } else {
            return false;
        }
    case DY_CORE_EXPR_RECURSION:
        if (substitute_recursion(ctx, expr.recursion, id, sub, &expr.recursion)) {
            *result = expr;
            return true;
        } else {
            return false;
        }
    }

    dy_bail("Impossible object type.");
}

bool substitute_equality_map(struct dy_core_ctx *ctx, struct dy_core_equality_map equality_map, size_t id, struct dy_core_expr sub, struct dy_core_equality_map *result)
{
    struct dy_core_expr e1;
    bool e1_is_new = substitute(ctx, *equality_map.e1, id, sub, &e1);
    
    struct dy_core_expr e2;
    bool e2_is_new = substitute(ctx, *equality_map.e2, id, sub, &e2);
    
    if (!e1_is_new && !e2_is_new) {
        return false;
    }
    
    if (e1_is_new) {
        equality_map.e1 = dy_core_expr_new(e1);
    } else {
        dy_core_expr_retain_ptr(equality_map.e1);
    }
    
    if (e2_is_new) {
        equality_map.e2 = dy_core_expr_new(e2);
    } else {
        dy_core_expr_retain_ptr(equality_map.e2);
    }
    
    *result = equality_map;
    return true;
}

bool substitute_type_map(struct dy_core_ctx *ctx, struct dy_core_type_map type_map, size_t id, struct dy_core_expr sub, struct dy_core_type_map *result)
{
    struct dy_core_expr type;
    bool type_is_new = substitute(ctx, *type_map.type, id, sub, &type);
    
    if (id != type_map.id) {
        if (dy_core_expr_is_bound(type_map.id, sub)) {
            size_t new_id = ctx->running_id++;
            
            dy_array_add(&ctx->equal_variables, &(struct dy_equal_variables){
                .id1 = type_map.id,
                .id2 = new_id
            });
            
            struct dy_core_expr expr;
            bool expr_is_new = substitute(ctx, *type_map.expr, id, sub, &expr);
            
            --ctx->equal_variables.num_elems;
            
            if (!type_is_new && !expr_is_new) {
                return false;
            }
            
            if (type_is_new) {
                type_map.type = dy_core_expr_new(type);
            } else {
                dy_core_expr_retain_ptr(type_map.type);
            }
            
            if (expr_is_new) {
                type_map.expr = dy_core_expr_new(expr);
            } else {
                dy_core_expr_retain_ptr(type_map.expr);
            }
            
            type_map.id = new_id;
            
            *result = type_map;
            return true;
        } else {
            struct dy_core_expr expr;
            bool expr_is_new = substitute(ctx, *type_map.expr, id, sub, &expr);
            
            if (!type_is_new && !expr_is_new) {
                return false;
            }
            
            if (type_is_new) {
                type_map.type = dy_core_expr_new(type);
            } else {
                dy_core_expr_retain_ptr(type_map.type);
            }
            
            if (expr_is_new) {
                type_map.expr = dy_core_expr_new(expr);
            } else {
                dy_core_expr_retain_ptr(type_map.expr);
            }
            
            *result = type_map;
            return true;
        }
    } else {
        if (!type_is_new) {
            return false;
        }
        
        if (type_is_new) {
            type_map.type = dy_core_expr_new(type);
        } else {
            dy_core_expr_retain_ptr(type_map.type);
        }
        
        type_map.expr = dy_core_expr_retain_ptr(type_map.expr);
        
        *result = type_map;
        return true;
    }
}

bool substitute_inference_type_map(struct dy_core_ctx *ctx, struct dy_core_inference_type_map inference_type_map, size_t id, struct dy_core_expr sub, struct dy_core_inference_type_map *result)
{
    struct dy_core_expr type;
    bool type_is_new = substitute(ctx, *inference_type_map.type, id, sub, &type);
    
    if (id != inference_type_map.id) {
        if (dy_core_expr_is_bound(inference_type_map.id, sub)) {
            size_t new_id = ctx->running_id++;
            
            dy_array_add(&ctx->equal_variables, &(struct dy_equal_variables){
                .id1 = inference_type_map.id,
                .id2 = new_id
            });
            
            struct dy_core_expr expr;
            bool expr_is_new = substitute(ctx, *inference_type_map.expr, id, sub, &expr);
            
            --ctx->equal_variables.num_elems;
            
            if (!type_is_new && !expr_is_new) {
                return false;
            }
            
            if (type_is_new) {
                inference_type_map.type = dy_core_expr_new(type);
            } else {
                dy_core_expr_retain_ptr(inference_type_map.type);
            }
            
            if (expr_is_new) {
                inference_type_map.expr = dy_core_expr_new(expr);
            } else {
                dy_core_expr_retain_ptr(inference_type_map.expr);
            }
            
            inference_type_map.id = new_id;
            
            *result = inference_type_map;
            return true;
        } else {
            struct dy_core_expr expr;
            bool expr_is_new = substitute(ctx, *inference_type_map.expr, id, sub, &expr);
            
            if (!type_is_new && !expr_is_new) {
                return false;
            }
            
            if (type_is_new) {
                inference_type_map.type = dy_core_expr_new(type);
            } else {
                dy_core_expr_retain_ptr(inference_type_map.type);
            }
            
            if (expr_is_new) {
                inference_type_map.expr = dy_core_expr_new(expr);
            } else {
                dy_core_expr_retain_ptr(inference_type_map.expr);
            }
            
            *result = inference_type_map;
            return true;
        }
    } else {
        if (!type_is_new) {
            return false;
        }
        
        if (type_is_new) {
            inference_type_map.type = dy_core_expr_new(type);
        } else {
            dy_core_expr_retain_ptr(inference_type_map.type);
        }
        
        inference_type_map.expr = dy_core_expr_retain_ptr(inference_type_map.expr);
        
        *result = inference_type_map;
        return true;
    }
}

bool substitute_recursion(struct dy_core_ctx *ctx, struct dy_core_recursion recursion, size_t id, struct dy_core_expr sub, struct dy_core_recursion *result)
{
    struct dy_core_expr type;
    bool type_is_new = substitute(ctx, *recursion.type, id, sub, &type);
    
    if (id != recursion.id) {
        if (dy_core_expr_is_bound(recursion.id, sub)) {
            size_t new_id = ctx->running_id++;
            
            dy_array_add(&ctx->equal_variables, &(struct dy_equal_variables){
                .id1 = recursion.id,
                .id2 = new_id
            });
            
            struct dy_core_expr expr;
            bool expr_is_new = substitute(ctx, *recursion.expr, id, sub, &expr);
            
            --ctx->equal_variables.num_elems;
            
            if (!type_is_new && !expr_is_new) {
                return false;
            }
            
            if (type_is_new) {
                recursion.type = dy_core_expr_new(type);
            } else {
                dy_core_expr_retain_ptr(recursion.type);
            }
            
            if (expr_is_new) {
                recursion.expr = dy_core_expr_new(expr);
            } else {
                dy_core_expr_retain_ptr(recursion.expr);
            }
            
            recursion.id = new_id;
            
            *result = recursion;
            return true;
        } else {
            struct dy_core_expr expr;
            bool expr_is_new = substitute(ctx, *recursion.expr, id, sub, &expr);
            
            if (!type_is_new && !expr_is_new) {
                return false;
            }
            
            if (type_is_new) {
                recursion.type = dy_core_expr_new(type);
            } else {
                dy_core_expr_retain_ptr(recursion.type);
            }
            
            if (expr_is_new) {
                recursion.expr = dy_core_expr_new(expr);
            } else {
                dy_core_expr_retain_ptr(recursion.expr);
            }
            
            *result = recursion;
            return true;
        }
    } else {
        if (!type_is_new) {
            return false;
        }
        
        if (type_is_new) {
            recursion.type = dy_core_expr_new(type);
        } else {
            dy_core_expr_retain_ptr(recursion.type);
        }
        
        recursion.expr = dy_core_expr_retain_ptr(recursion.expr);
        
        *result = recursion;
        return true;
    }
}
