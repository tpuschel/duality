/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "ast.h"
#include "parser.h"
#include "unbound_variable.h"
#include "def.h"

#include "../core/core.h"

#include "../support/array.h"
#include "../support/range.h"

/**
 * Implements the tranformation from AST to Core.
 */

/** Represents a variable in the source text and its replacement id + type for Core. */
struct dy_ast_to_core_bound_var {
    dy_string_t name;
    size_t replacement_id;
    struct dy_core_expr type;
};

struct dy_ast_to_core_ctx {
    size_t running_id;
    dy_array_t bound_vars;
};

static inline struct dy_core_expr dy_ast_expr_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_expr expr);

static inline struct dy_core_expr dy_ast_positive_type_map_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_type_map type_map);

static inline struct dy_core_expr dy_ast_negative_type_map_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_type_map type_map);

static inline struct dy_core_expr dy_ast_list_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_list list);

static inline struct dy_core_expr dy_ast_choice_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_list choice);

static inline struct dy_core_expr dy_ast_try_block_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_list try_block);

static inline struct dy_core_expr dy_ast_positive_equality_map_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_equality_map equality_map);

static inline struct dy_core_expr dy_ast_negative_equality_map_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_equality_map equality_map);

static inline struct dy_core_expr dy_ast_do_block_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_do_block do_block);

static inline struct dy_core_expr dy_ast_equality_map_elim_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_equality_map_elim elim);

static inline struct dy_core_expr dy_ast_type_map_elim_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_type_map_elim elim);

static inline struct dy_core_expr dy_ast_juxtaposition_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_juxtaposition juxtaposition);

static inline struct dy_core_expr dy_ast_variable_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_literal variable);

static inline struct dy_core_expr dy_ast_positive_recursion_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_recursion rec);

static inline struct dy_core_expr dy_ast_negative_recursion_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_recursion rec);

static inline struct dy_core_expr ast_type_map_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_type_map type_map, enum dy_core_polarity polarity);

static inline struct dy_core_expr ast_equality_map_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_equality_map equality_map, enum dy_core_polarity polarity);

static inline struct dy_core_expr ast_list_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_list_inner list, enum dy_core_polarity polarity);

static inline struct dy_core_expr ast_try_block_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_list_inner try_block);

static inline struct dy_core_expr ast_do_block_body_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_do_block_body do_block);

static inline struct dy_core_expr do_block_equality_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_do_block_equality equality);

static inline struct dy_core_expr do_block_let_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_do_block_let let);

static inline struct dy_core_expr do_block_inverted_let_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_do_block_let inverted_let);

static inline struct dy_core_expr do_block_ignored_expr_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_do_block_ignored_expr ignored_expr);

static inline struct dy_core_expr do_block_def_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_do_block_def def);

static inline struct dy_core_expr recursion_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_recursion rec, enum dy_core_polarity polarity);

static inline struct dy_core_variable create_inference_var(struct dy_ast_to_core_ctx *ctx);

struct dy_core_expr dy_ast_expr_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_expr expr)
{
    switch (expr.tag) {
    case DY_AST_EXPR_POSITIVE_TYPE_MAP:
        return dy_ast_positive_type_map_to_core(ctx, expr.positive_type_map);
    case DY_AST_EXPR_NEGATIVE_TYPE_MAP:
        return dy_ast_negative_type_map_to_core(ctx, expr.negative_type_map);
    case DY_AST_EXPR_LIST:
        return dy_ast_list_to_core(ctx, expr.list);
    case DY_AST_EXPR_POSITIVE_EQUALITY_MAP:
        return dy_ast_positive_equality_map_to_core(ctx, expr.positive_equality_map);
    case DY_AST_EXPR_NEGATIVE_EQUALITY_MAP:
        return dy_ast_negative_equality_map_to_core(ctx, expr.negative_equality_map);
    case DY_AST_EXPR_DO_BLOCK:
        return dy_ast_do_block_to_core(ctx, expr.do_block);
    case DY_AST_EXPR_CHOICE:
        return dy_ast_choice_to_core(ctx, expr.choice);
    case DY_AST_EXPR_TRY_BLOCK:
        return dy_ast_try_block_to_core(ctx, expr.try_block);
    case DY_AST_EXPR_EQUALITY_MAP_ELIM:
        return dy_ast_equality_map_elim_to_core(ctx, expr.equality_map_elim);
    case DY_AST_EXPR_TYPE_MAP_ELIM:
        return dy_ast_type_map_elim_to_core(ctx, expr.type_map_elim);
    case DY_AST_EXPR_VARIABLE:
        return dy_ast_variable_to_core(ctx, expr.variable);
    case DY_AST_EXPR_JUXTAPOSITION:
        return dy_ast_juxtaposition_to_core(ctx, expr.juxtaposition);
    case DY_AST_EXPR_POSITIVE_RECURSION:
        return dy_ast_positive_recursion_to_core(ctx, expr.positive_recursion);
    case DY_AST_EXPR_NEGATIVE_RECURSION:
        return dy_ast_negative_recursion_to_core(ctx, expr.positive_recursion);
    case DY_AST_EXPR_ALL:
        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_END,
            .end_polarity = DY_CORE_POLARITY_POSITIVE
        };
    case DY_AST_EXPR_ANY:
        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_END,
            .end_polarity = DY_CORE_POLARITY_NEGATIVE
        };
    case DY_AST_EXPR_SYMBOL:
        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_SYMBOL
        };
    }

    dy_bail("Impossible ast type.");
}

struct dy_core_expr dy_ast_variable_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_literal variable)
{
    for (size_t i = ctx->bound_vars.num_elems; i-- > 0;) {
        struct dy_ast_to_core_bound_var bound_var;
        dy_array_get(ctx->bound_vars, i, &bound_var);

        if (dy_string_are_equal(variable.value, bound_var.name)) {
            return (struct dy_core_expr){
                .tag = DY_CORE_EXPR_VARIABLE,
                .variable = {
                    .id = bound_var.replacement_id,
                    .type = dy_core_expr_new(dy_core_expr_retain(bound_var.type)),
                }
            };
        }
    }

    struct dy_uv_data data = {
        .var = variable
    };

    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_CUSTOM,
        .custom = dy_uv_create(data)
    };
}

struct dy_core_expr ast_type_map_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_type_map type_map, enum dy_core_polarity polarity)
{
    if (type_map.arg.has_type) {
        struct dy_core_expr type = dy_ast_expr_to_core(ctx, *type_map.arg.type);

        size_t id = ctx->running_id++;

        size_t old_size;
        if (type_map.arg.has_name) {
            struct dy_ast_to_core_bound_var bound_var = {
                .name = type_map.arg.name.value,
                .replacement_id = id,
                .type = type
            };

            old_size = dy_array_add(&ctx->bound_vars, &bound_var);
        } else {
            old_size = ctx->bound_vars.num_elems;
        }

        struct dy_core_expr e = dy_ast_expr_to_core(ctx, *type_map.expr);

        ctx->bound_vars.num_elems = old_size;

        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_TYPE_MAP,
            .type_map = {
                .binding = {
                    .id = id,
                    .type = dy_core_expr_new(dy_core_expr_retain(type)),
                },
                .expr = dy_core_expr_new(e),
                .polarity = polarity,
                .is_implicit = type_map.is_implicit,
            }
        };
    } else {
        struct dy_core_variable variable = create_inference_var(ctx);

        struct dy_core_expr type = {
            .tag = DY_CORE_EXPR_INFERENCE_VARIABLE,
            .inference_variable = variable
        };

        size_t id = ctx->running_id++;

        size_t old_size;
        if (type_map.arg.has_name) {
            struct dy_ast_to_core_bound_var bound_var = {
                .name = type_map.arg.name.value,
                .replacement_id = id,
                .type = type
            };

            old_size = dy_array_add(&ctx->bound_vars, &bound_var);
        } else {
            old_size = ctx->bound_vars.num_elems;
        }

        struct dy_core_expr e = dy_ast_expr_to_core(ctx, *type_map.expr);

        ctx->bound_vars.num_elems = old_size;

        struct dy_core_expr result_type_map = {
            .tag = DY_CORE_EXPR_TYPE_MAP,
            .type_map = {
                .binding = {
                    .id = id,
                    .type = dy_core_expr_new(dy_core_expr_retain(type)),
                },
                .expr = dy_core_expr_new(e),
                .polarity = polarity,
                .is_implicit = type_map.is_implicit,
            }
        };

        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_INFERENCE_TYPE_MAP,
            .inference_type_map = {
                .binding = {
                    .id = variable.id,
                    .type = dy_core_expr_retain_ptr(variable.type),
                },
                .expr = dy_core_expr_new(result_type_map),
                .polarity = DY_CORE_POLARITY_NEGATIVE,
            }
        };
    }
}

struct dy_core_expr dy_ast_positive_type_map_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_type_map type_map)
{
    return ast_type_map_to_core(ctx, type_map, DY_CORE_POLARITY_POSITIVE);
}

struct dy_core_expr dy_ast_negative_type_map_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_type_map type_map)
{
    return ast_type_map_to_core(ctx, type_map, DY_CORE_POLARITY_NEGATIVE);
}

struct dy_core_expr ast_list_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_list_inner list, enum dy_core_polarity polarity)
{
    struct dy_core_expr e1 = dy_ast_expr_to_core(ctx, *list.expr);

    if (!list.next_or_null) {
        return e1;
    }

    struct dy_core_expr e2 = ast_list_to_core(ctx, *list.next_or_null, polarity);

    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_JUNCTION,
        .junction = {
            .e1 = dy_core_expr_new(e1),
            .e2 = dy_core_expr_new(e2),
            .polarity = polarity,
        }
    };
}

struct dy_core_expr dy_ast_list_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_list list)
{
    return ast_list_to_core(ctx, list.inner, DY_CORE_POLARITY_POSITIVE);
}

struct dy_core_expr dy_ast_choice_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_list choice)
{
    return ast_list_to_core(ctx, choice.inner, DY_CORE_POLARITY_NEGATIVE);
}

struct dy_core_expr ast_try_block_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_list_inner try_block)
{
    struct dy_core_expr e1 = dy_ast_expr_to_core(ctx, *try_block.expr);

    if (!try_block.next_or_null) {
        return e1;
    }

    struct dy_core_expr e2 = ast_try_block_to_core(ctx, *try_block.next_or_null);

    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_ALTERNATIVE,
        .alternative = {
            .first = dy_core_expr_new(e1),
            .second = dy_core_expr_new(e2),
        }
    };
}

struct dy_core_expr dy_ast_try_block_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_list try_block)
{
    return ast_try_block_to_core(ctx, try_block.inner);
}

struct dy_core_expr ast_equality_map_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_equality_map equality_map, enum dy_core_polarity polarity)
{
    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_EQUALITY_MAP,
        .equality_map = {
            .e1 = dy_core_expr_new(dy_ast_expr_to_core(ctx, *equality_map.e1)),
            .e2 = dy_core_expr_new(dy_ast_expr_to_core(ctx, *equality_map.e2)),
            .polarity = polarity,
            .is_implicit = equality_map.is_implicit,
        }
    };
}

struct dy_core_expr dy_ast_positive_equality_map_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_equality_map positive_equality_map)
{
    return ast_equality_map_to_core(ctx, positive_equality_map, DY_CORE_POLARITY_POSITIVE);
}

struct dy_core_expr dy_ast_negative_equality_map_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_equality_map negative_equality_map)
{
    return ast_equality_map_to_core(ctx, negative_equality_map, DY_CORE_POLARITY_NEGATIVE);
}

struct dy_core_expr ast_do_block_body_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_do_block_body do_block)
{
    switch (do_block.tag) {
    case DY_AST_DO_BLOCK_END_EXPR:
        return dy_ast_expr_to_core(ctx, *do_block.end_expr);
    case DY_AST_DO_BLOCK_LET:
        return do_block_let_to_core(ctx, do_block.let);
    case DY_AST_DO_BLOCK_INVERTED_LET:
        return do_block_inverted_let_to_core(ctx, do_block.inverted_let);
    case DY_AST_DO_BLOCK_IGNORED_EXPR:
        return do_block_ignored_expr_to_core(ctx, do_block.ignored_expr);
    case DY_AST_DO_BLOCK_EQUALITY:
        return do_block_equality_to_core(ctx, do_block.equality);
    case DY_AST_DO_BLOCK_DEF:
        return do_block_def_to_core(ctx, do_block.def);
    }

    dy_bail("Impossible do block type.");
}

struct dy_core_expr dy_ast_do_block_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_do_block do_block)
{
    return ast_do_block_body_to_core(ctx, do_block.body);
}

struct dy_core_expr do_block_def_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_do_block_def def)
{
    struct dy_core_expr expr = dy_ast_expr_to_core(ctx, *def.expr);

    size_t id = ctx->running_id++;

    struct dy_core_expr arg_type = {
        .tag = DY_CORE_EXPR_END,
        .end_polarity = DY_CORE_POLARITY_NEGATIVE
    };

    struct dy_ast_to_core_bound_var bound_var = {
        .name = def.name.value,
        .replacement_id = id,
        .type = arg_type
    };

    size_t old_size = dy_array_add(&ctx->bound_vars, &bound_var);

    struct dy_core_expr rest = ast_do_block_body_to_core(ctx, *def.rest);

    ctx->bound_vars.num_elems = old_size;

    struct dy_def_data data = {
        .id = id,
        .arg = expr,
        .body = rest
    };

    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_CUSTOM,
        .custom = dy_def_create(data)
    };
}

struct dy_core_expr do_block_equality_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_do_block_equality equality)
{
    struct dy_core_expr e1 = dy_ast_expr_to_core(ctx, *equality.e1);

    struct dy_core_expr e2 = dy_ast_expr_to_core(ctx, *equality.e2);

    struct dy_core_expr rest = ast_do_block_body_to_core(ctx, *equality.rest);

    struct dy_core_expr positive_equality_map = {
        .tag = DY_CORE_EXPR_EQUALITY_MAP,
        .equality_map = {
            .e1 = dy_core_expr_new(e1),
            .e2 = dy_core_expr_new(rest),
            .polarity = DY_CORE_POLARITY_POSITIVE,
            .is_implicit = false,
        }
    };

    struct dy_core_expr result_type = {
        .tag = DY_CORE_EXPR_INFERENCE_VARIABLE,
        .inference_variable = create_inference_var(ctx)
    };

    struct dy_core_expr elim = {
        .tag = DY_CORE_EXPR_EQUALITY_MAP_ELIM,
        .equality_map_elim = {
            .expr = dy_core_expr_new(positive_equality_map),
            .map = {
                .e1 = dy_core_expr_new(e2),
                .e2 = dy_core_expr_new(dy_core_expr_retain(result_type)),
                .polarity = DY_CORE_POLARITY_NEGATIVE,
                .is_implicit = false,
            },
            .check_result = DY_MAYBE,
        }
    };

    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_INFERENCE_TYPE_MAP,
        .inference_type_map = {
            .binding = {
                .id = result_type.variable.id,
                .type = result_type.variable.type,
            },
            .expr = dy_core_expr_new(elim),
            .polarity = DY_CORE_POLARITY_POSITIVE,
        }
    };
}

struct dy_core_expr do_block_inverted_let_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_do_block_let inverted_let)
{
    struct dy_core_expr e = dy_ast_expr_to_core(ctx, *inverted_let.expr);

    size_t id = ctx->running_id++;

    struct dy_core_variable variable = create_inference_var(ctx);

    struct dy_core_expr arg_type = {
        .tag = DY_CORE_EXPR_INFERENCE_VARIABLE,
        .inference_variable = variable
    };

    struct dy_ast_to_core_bound_var bound_var = {
        .name = inverted_let.arg_name.value,
        .replacement_id = id,
        .type = arg_type
    };

    size_t old_size = dy_array_add(&ctx->bound_vars, &bound_var);

    struct dy_core_expr rest = ast_do_block_body_to_core(ctx, *inverted_let.rest);

    ctx->bound_vars.num_elems = old_size;

    struct dy_core_expr positive_type_map = {
        .tag = DY_CORE_EXPR_TYPE_MAP,
        .type_map = {
            .binding = {
                .id = id,
                .type = dy_core_expr_new(dy_core_expr_retain(arg_type)),
            },
            .expr = dy_core_expr_new(rest),
            .polarity = DY_CORE_POLARITY_POSITIVE,
            .is_implicit = false,
        }
    };

    struct dy_core_expr result_inference_type_map = {
        .tag = DY_CORE_EXPR_INFERENCE_TYPE_MAP,
        .inference_type_map = {
            .binding = {
                .id = variable.id,
                .type = dy_core_expr_retain_ptr(variable.type),
            },
            .expr = dy_core_expr_new(positive_type_map),
            .polarity = DY_CORE_POLARITY_NEGATIVE,
        }
    };

    struct dy_core_expr result_type = {
        .tag = DY_CORE_EXPR_INFERENCE_VARIABLE,
        .inference_variable = create_inference_var(ctx)
    };

    struct dy_core_expr elim = {
        .tag = DY_CORE_EXPR_EQUALITY_MAP_ELIM,
        .equality_map_elim = {
            .expr = dy_core_expr_new(e),
            .map = {
                .e1 = dy_core_expr_new(result_inference_type_map),
                .e2 = dy_core_expr_new(dy_core_expr_retain(result_type)),
                .polarity = DY_CORE_POLARITY_NEGATIVE,
                .is_implicit = false,
            },
            .check_result = DY_MAYBE,
        }
    };

    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_INFERENCE_TYPE_MAP,
        .inference_type_map = {
            .binding = {
                .id = result_type.variable.id,
                .type = result_type.variable.type,
            },
            .expr = dy_core_expr_new(elim),
            .polarity = DY_CORE_POLARITY_POSITIVE,
        }
    };
}

struct dy_core_expr do_block_let_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_do_block_let let)
{
    struct dy_core_expr e = dy_ast_expr_to_core(ctx, *let.expr);

    size_t id = ctx->running_id++;

    struct dy_core_variable variable = create_inference_var(ctx);

    struct dy_core_expr arg_type = {
        .tag = DY_CORE_EXPR_INFERENCE_VARIABLE,
        .inference_variable = variable
    };

    struct dy_ast_to_core_bound_var bound_var = {
        .name = let.arg_name.value,
        .replacement_id = id,
        .type = arg_type
    };

    size_t old_size = dy_array_add(&ctx->bound_vars, &bound_var);

    struct dy_core_expr rest = ast_do_block_body_to_core(ctx, *let.rest);

    ctx->bound_vars.num_elems = old_size;

    struct dy_core_expr positive_type_map = {
        .tag = DY_CORE_EXPR_TYPE_MAP,
        .type_map = {
            .binding = {
                .id = id,
                .type = dy_core_expr_new(dy_core_expr_retain(arg_type)),
            },
            .expr = dy_core_expr_new(rest),
            .polarity = DY_CORE_POLARITY_POSITIVE,
            .is_implicit = false,
        }
    };

    struct dy_core_expr result_inference_type_map = {
        .tag = DY_CORE_EXPR_INFERENCE_TYPE_MAP,
        .inference_type_map = {
            .binding = {
                .id = variable.id,
                .type = dy_core_expr_retain_ptr(variable.type),
            },
            .expr = dy_core_expr_new(positive_type_map),
            .polarity = DY_CORE_POLARITY_NEGATIVE,
        }
    };

    struct dy_core_expr result_type = {
        .tag = DY_CORE_EXPR_INFERENCE_VARIABLE,
        .inference_variable = create_inference_var(ctx)
    };

    struct dy_core_expr elim = {
        .tag = DY_CORE_EXPR_EQUALITY_MAP_ELIM,
        .equality_map_elim = {
            .expr = dy_core_expr_new(result_inference_type_map),
            .map = {
                .e1 = dy_core_expr_new(e),
                .e2 = dy_core_expr_new(dy_core_expr_retain(result_type)),
                .polarity = DY_CORE_POLARITY_NEGATIVE,
                .is_implicit = false,
            },
            .check_result = DY_MAYBE,
        }
    };

    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_INFERENCE_TYPE_MAP,
        .inference_type_map = {
            .binding = {
                .id = result_type.variable.id,
                .type = result_type.variable.type,
            },
            .expr = dy_core_expr_new(elim),
            .polarity = DY_CORE_POLARITY_POSITIVE,
        }
    };
}

struct dy_core_expr do_block_ignored_expr_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_do_block_ignored_expr ignored_expr)
{
    struct dy_core_expr e = dy_ast_expr_to_core(ctx, *ignored_expr.expr);

    struct dy_core_expr rest = ast_do_block_body_to_core(ctx, *ignored_expr.rest);

    struct dy_core_variable variable = create_inference_var(ctx);

    struct dy_core_expr inference_var_expr = {
        .tag = DY_CORE_EXPR_INFERENCE_VARIABLE,
        .inference_variable = variable
    };

    struct dy_core_expr positive_type_map = {
        .tag = DY_CORE_EXPR_TYPE_MAP,
        .type_map = {
            .binding = {
                .id = ctx->running_id++,
                .type = dy_core_expr_new(inference_var_expr),
            },
            .expr = dy_core_expr_new(rest),
            .polarity = DY_CORE_POLARITY_POSITIVE,
            .is_implicit = false,
        }
    };

    struct dy_core_expr result_inference_type_map = {
        .tag = DY_CORE_EXPR_INFERENCE_TYPE_MAP,
        .inference_type_map = {
            .binding = {
                .id = variable.id,
                .type = dy_core_expr_retain_ptr(variable.type),
            },
            .expr = dy_core_expr_new(positive_type_map),
            .polarity = DY_CORE_POLARITY_NEGATIVE,
        }
    };

    struct dy_core_expr result_type = {
        .tag = DY_CORE_EXPR_INFERENCE_VARIABLE,
        .inference_variable = create_inference_var(ctx)
    };

    struct dy_core_expr elim = {
        .tag = DY_CORE_EXPR_EQUALITY_MAP_ELIM,
        .equality_map_elim = {
            .expr = dy_core_expr_new(result_inference_type_map),
            .map = {
                .e1 = dy_core_expr_new(e),
                .e2 = dy_core_expr_new(dy_core_expr_retain(result_type)),
                .polarity = DY_CORE_POLARITY_NEGATIVE,
                .is_implicit = false,
            },
            .check_result = DY_MAYBE,
        }
    };

    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_INFERENCE_TYPE_MAP,
        .inference_type_map = {
            .binding = {
                .id = result_type.inference_variable.id,
                .type = dy_core_expr_retain_ptr(result_type.inference_variable.type),
            },
            .expr = dy_core_expr_new(elim),
            .polarity = DY_CORE_POLARITY_POSITIVE,
        }
    };
}

struct dy_core_expr dy_ast_equality_map_elim_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_equality_map_elim elim)
{
    struct dy_core_expr e1 = dy_ast_expr_to_core(ctx, *elim.expr);

    struct dy_core_expr e2 = dy_ast_negative_equality_map_to_core(ctx, elim.equality_map);

    assert(e2.tag == DY_CORE_EXPR_EQUALITY_MAP);

    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_EQUALITY_MAP_ELIM,
        .equality_map_elim = {
            .expr = dy_core_expr_new(e1),
            .map = e2.equality_map,
            .check_result = DY_MAYBE,
        }
    };
}

struct dy_core_expr dy_ast_type_map_elim_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_type_map_elim elim)
{
    struct dy_core_expr e1 = dy_ast_expr_to_core(ctx, *elim.expr);

    struct dy_core_expr e2 = dy_ast_negative_type_map_to_core(ctx, elim.type_map);

    assert(e2.tag == DY_CORE_EXPR_TYPE_MAP);

    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_TYPE_MAP_ELIM,
        .type_map_elim = {
            .expr = dy_core_expr_new(e1),
            .map = e2.type_map,
            .check_result = DY_MAYBE,
        }
    };
}

struct dy_core_expr dy_ast_juxtaposition_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_juxtaposition juxtaposition)
{
    struct dy_core_expr left = dy_ast_expr_to_core(ctx, *juxtaposition.left);

    struct dy_core_expr right = dy_ast_expr_to_core(ctx, *juxtaposition.right);

    struct dy_core_variable variable = create_inference_var(ctx);

    struct dy_core_expr inference_var_expr = {
        .tag = DY_CORE_EXPR_INFERENCE_VARIABLE,
        .inference_variable = variable
    };

    struct dy_core_expr elim = {
        .tag = DY_CORE_EXPR_EQUALITY_MAP_ELIM,
        .equality_map_elim = {
            .expr = dy_core_expr_new(left),
            .map = {
                .e1 = dy_core_expr_new(right),
                .e2 = dy_core_expr_new(inference_var_expr),
                .polarity = DY_CORE_POLARITY_NEGATIVE,
                .is_implicit = false,
            },
            .check_result = DY_MAYBE,
        }
    };

    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_INFERENCE_TYPE_MAP,
        .inference_type_map = {
            .binding = {
                .id = variable.id,
                .type = dy_core_expr_retain_ptr(variable.type),
            },
            .expr = dy_core_expr_new(elim),
            .polarity = DY_CORE_POLARITY_POSITIVE,
        }
    };
}

struct dy_core_expr dy_ast_positive_recursion_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_recursion rec)
{
    return recursion_to_core(ctx, rec, DY_CORE_POLARITY_POSITIVE);
}

struct dy_core_expr dy_ast_negative_recursion_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_recursion rec)
{
    return recursion_to_core(ctx, rec, DY_CORE_POLARITY_NEGATIVE);
}

struct dy_core_expr recursion_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_recursion rec, enum dy_core_polarity polarity)
{
    if (rec.arg.has_type) {
        struct dy_core_expr type = dy_ast_expr_to_core(ctx, *rec.arg.type);

        size_t id = ctx->running_id++;

        size_t old_size;
        if (rec.arg.has_name) {
            struct dy_ast_to_core_bound_var bound_var = {
                .name = rec.arg.name.value,
                .replacement_id = id,
                .type = type
            };

            old_size = dy_array_add(&ctx->bound_vars, &bound_var);
        } else {
            old_size = ctx->bound_vars.num_elems;
        }

        struct dy_core_expr e = dy_ast_expr_to_core(ctx, *rec.expr);

        ctx->bound_vars.num_elems = old_size;

        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_RECURSION,
            .recursion = {
                .map = {
                    .binding = {
                        .id = id,
                        .type = dy_core_expr_new(dy_core_expr_retain(type)),
                    },
                    .expr = dy_core_expr_new(e),
                    .polarity = polarity,
                },
                .check_result = DY_MAYBE }
        };
    } else {
        struct dy_core_variable variable = create_inference_var(ctx);

        struct dy_core_expr type = {
            .tag = DY_CORE_EXPR_INFERENCE_VARIABLE,
            .inference_variable = variable
        };

        size_t id = ctx->running_id++;

        size_t old_size;
        if (rec.arg.has_name) {
            struct dy_ast_to_core_bound_var bound_var = {
                .name = rec.arg.name.value,
                .replacement_id = id,
                .type = type
            };

            old_size = dy_array_add(&ctx->bound_vars, &bound_var);
        } else {
            old_size = ctx->bound_vars.num_elems;
        }

        struct dy_core_expr e = dy_ast_expr_to_core(ctx, *rec.expr);

        ctx->bound_vars.num_elems = old_size;

        struct dy_core_expr result_recursion = {
            .tag = DY_CORE_EXPR_RECURSION,
            .recursion = {
                .map = {
                    .binding = {
                        .id = id,
                        .type = dy_core_expr_new(dy_core_expr_retain(type)),
                    },
                    .expr = dy_core_expr_new(e),
                    .polarity = polarity,
                },
                .check_result = DY_MAYBE,
            }
        };

        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_INFERENCE_TYPE_MAP,
            .inference_type_map = {
                .binding = {
                    .id = variable.id,
                    .type = dy_core_expr_retain_ptr(variable.type),
                },
                .expr = dy_core_expr_new(result_recursion),
                .polarity = DY_CORE_POLARITY_NEGATIVE,
            }
        };
    }
}

struct dy_core_variable create_inference_var(struct dy_ast_to_core_ctx *ctx)
{
    struct dy_core_expr any = {
        .tag = DY_CORE_EXPR_END,
        .end_polarity = DY_CORE_POLARITY_NEGATIVE
    };

    return (struct dy_core_variable){
        .id = ctx->running_id++,
        .type = dy_core_expr_new(any)
    };
}
