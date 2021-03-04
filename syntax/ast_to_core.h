/*
 * Copyright 2021 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "ast.h"
#include "string.h"
#include "def.h"
#include "unbound_variable.h"

#include "../core/core.h"

struct dy_ast_to_core_ctx {
    size_t running_id;
    dy_array_t variable_replacements;
};

struct dy_variable_replacement {
    dy_string_t variable;
    size_t replacement_id;
};

enum dy_converted_map_either_body_tag {
    DY_CONVERTED_MAP_EITHER_BODY_ASSUMPTION,
    DY_CONVERTED_MAP_EITHER_BODY_CHOICE_MAP
};

struct dy_converted_map_either_body {
    union {
        struct dy_core_assumption assumption;
        struct dy_core_map_choice choice_map;
    };

    enum dy_converted_map_either_body_tag tag;
};

static inline struct dy_core_expr dy_ast_expr_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_expr expr);

static inline struct dy_core_expr dy_ast_function_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_function function);

static inline struct dy_core_expr dy_ast_list_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_list list);

static inline struct dy_core_expr dy_ast_recursion_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_recursion recursion);

static inline struct dy_core_expr dy_ast_simple_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_simple solution);

static inline struct dy_core_expr dy_ast_juxtaposition_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_juxtaposition juxtaposition);

static inline struct dy_core_expr dy_ast_do_block_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_do_block do_block);

static inline struct dy_core_expr dy_ast_variable_to_core(struct dy_ast_to_core_ctx *ctx, const dy_array_t *variable);

static inline struct dy_core_expr dy_ast_any_to_core(struct dy_ast_to_core_ctx *ctx);

static inline struct dy_core_expr dy_ast_void_to_core(struct dy_ast_to_core_ctx *ctx);

static inline struct dy_core_expr dy_ast_string_to_core(struct dy_ast_to_core_ctx *ctx, const dy_array_t *string);

static inline struct dy_core_expr dy_ast_map_some_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_map_some map_some);

static inline struct dy_core_expr dy_ast_map_either_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_map_either map_either);

static inline struct dy_core_expr dy_ast_map_fin_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_map_fin map_fin);


static inline struct dy_core_expr dy_construct_function(struct dy_ast_to_core_ctx *ctx, struct dy_ast_binding binding, struct dy_ast_expr expr, bool is_implicit, bool is_negative);

static inline struct dy_core_assumption dy_construct_bare_function(struct dy_ast_to_core_ctx *ctx, struct dy_ast_binding binding, struct dy_ast_expr expr, size_t *inference_id, bool *have_inference_id);

static inline struct dy_core_expr dy_make_function_without_type(struct dy_ast_to_core_ctx *ctx, size_t id, struct dy_core_expr expr, bool is_implicit, enum dy_polarity polarity);

static inline struct dy_core_expr dy_process_pattern(struct dy_ast_to_core_ctx *ctx, struct dy_core_expr pattern_expr, struct dy_ast_pattern pattern, struct dy_ast_expr final_expr);

static inline struct dy_core_expr dy_convert_juxtaposition_without_type(struct dy_ast_to_core_ctx *ctx, struct dy_core_expr left, struct dy_core_expr right, bool is_implicit);

static inline struct dy_core_expr dy_convert_juxtaposition(struct dy_ast_to_core_ctx *ctx, struct dy_core_expr left, struct dy_core_expr right, struct dy_core_expr type, bool is_implicit);

static inline struct dy_core_expr dy_convert_argument_app_without_type(struct dy_ast_to_core_ctx *ctx, struct dy_core_expr left, struct dy_ast_argument right, bool is_implicit);

static inline struct dy_core_expr dy_make_direction_app_without_type(struct dy_ast_to_core_ctx *ctx, struct dy_core_expr left, enum dy_direction direction, bool is_implicit);

static inline struct dy_core_expr dy_convert_index_app_without_type(struct dy_ast_to_core_ctx *ctx, struct dy_core_expr left, struct dy_ast_argument_index index, bool is_implicit);

static inline struct dy_core_expr dy_convert_unfold_app_without_type(struct dy_ast_to_core_ctx *ctx, struct dy_core_expr left, bool is_implicit);

static inline struct dy_core_expr dy_convert_unwrap_app_without_type(struct dy_ast_to_core_ctx *ctx, struct dy_core_expr left, bool is_implicit);

static inline struct dy_core_expr dy_convert_argument_app(struct dy_ast_to_core_ctx *ctx, struct dy_core_expr left, struct dy_ast_argument right, bool is_implicit, struct dy_core_expr type);

static inline struct dy_core_expr dy_make_direction_app(struct dy_ast_to_core_ctx *ctx, struct dy_core_expr left, enum dy_direction direction, struct dy_core_expr type, bool is_implicit);

static inline struct dy_core_expr dy_convert_index_app(struct dy_ast_to_core_ctx *ctx, struct dy_core_expr left, struct dy_ast_argument_index index, struct dy_core_expr type, bool is_implicit);

static inline struct dy_core_expr dy_convert_unfold_app(struct dy_ast_to_core_ctx *ctx, struct dy_core_expr left, struct dy_core_expr type, bool is_implicit);

static inline struct dy_core_expr dy_convert_unwrap_app(struct dy_ast_to_core_ctx *ctx, struct dy_core_expr left, struct dy_core_expr type, bool is_implicit);

static inline struct dy_core_expr dy_convert_list_body(struct dy_ast_to_core_ctx *ctx, struct dy_ast_list_body list_body, bool is_implicit, bool is_negative);

static inline struct dy_converted_map_either_body dy_convert_map_either_body(struct dy_ast_to_core_ctx *ctx, struct dy_ast_map_either_body body, bool is_implicit, dy_array_t *inference_ids);

struct dy_core_expr dy_ast_expr_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_expr expr)
{
    switch (expr.tag) {
    case DY_AST_EXPR_FUNCTION:
        return dy_ast_function_to_core(ctx, expr.function);
    case DY_AST_EXPR_LIST:
        return dy_ast_list_to_core(ctx, expr.list);
    case DY_AST_EXPR_RECURSION:
        return dy_ast_recursion_to_core(ctx, expr.recursion);
    case DY_AST_EXPR_SIMPLE:
        return dy_ast_simple_to_core(ctx, expr.simple);
    case DY_AST_EXPR_JUXTAPOSITION:
        return dy_ast_juxtaposition_to_core(ctx, expr.juxtaposition);
    case DY_AST_EXPR_DO_BLOCK:
        return dy_ast_do_block_to_core(ctx, expr.do_block);
    case DY_AST_EXPR_VARIABLE:
        return dy_ast_variable_to_core(ctx, &expr.variable);
    case DY_AST_EXPR_ANY:
        return dy_ast_any_to_core(ctx);
    case DY_AST_EXPR_VOID:
        return dy_ast_void_to_core(ctx);
    case DY_AST_EXPR_STRING:
        return dy_ast_string_to_core(ctx, &expr.string);
    case DY_AST_EXPR_STRING_TYPE:
        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_CUSTOM,
            .custom = dy_string_type_create()
        };
    case DY_AST_EXPR_MAP_SOME:
        return dy_ast_map_some_to_core(ctx, expr.map_some);
    case DY_AST_EXPR_MAP_EITHER:
        return dy_ast_map_either_to_core(ctx, expr.map_either);
    case DY_AST_EXPR_MAP_FIN:
        return dy_ast_map_fin_to_core(ctx, expr.map_fin);
    }

    dy_bail("impossible");
}

struct dy_core_expr dy_ast_function_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_function function)
{
    return dy_construct_function(ctx, function.binding, *function.expr, function.is_implicit, function.is_some);
}

struct dy_core_expr dy_ast_list_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_list list)
{
    return dy_convert_list_body(ctx, list.body, list.is_implicit, list.is_either);
}

struct dy_core_expr dy_ast_recursion_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_recursion recursion)
{
    size_t id = ctx->running_id++;

    dy_array_add(&ctx->variable_replacements, &(struct dy_variable_replacement){
        .variable = dy_array_view(&recursion.name),
        .replacement_id = id
    });

    struct dy_core_expr e = dy_ast_expr_to_core(ctx, *recursion.expr);

    ctx->variable_replacements.num_elems--;

    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_INTRO,
        .intro = {
            .polarity = recursion.is_fin ? DY_POLARITY_NEGATIVE : DY_POLARITY_POSITIVE,
            .is_implicit = recursion.is_implicit,
            .tag = DY_CORE_INTRO_COMPLEX,
            .complex = {
                .tag = DY_CORE_COMPLEX_RECURSION,
                .recursion = {
                    .id = id,
                    .expr = dy_core_expr_new(e)
                }
            }
        }
    };
}

struct dy_core_expr dy_ast_simple_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_simple simple)
{
    struct dy_core_expr out = dy_ast_expr_to_core(ctx, *simple.expr);

    switch (simple.argument.tag) {
    case DY_AST_ARGUMENT_EXPR: {
        struct dy_core_expr in = dy_ast_expr_to_core(ctx, *simple.argument.expr);

        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_INTRO,
            .intro = {
                .is_implicit = simple.is_implicit,
                .polarity = simple.is_negative ? DY_POLARITY_NEGATIVE : DY_POLARITY_POSITIVE,
                .tag = DY_CORE_INTRO_SIMPLE,
                .simple = {
                    .tag = DY_CORE_SIMPLE_PROOF,
                    .proof = dy_core_expr_new(in),
                    .out = dy_core_expr_new(out)
                }
            }
        };
    }
    case DY_AST_ARGUMENT_INDEX: {
        struct dy_core_expr e;
        if (simple.argument.index.is_maximum) {
            e = out;
        } else {
            e = (struct dy_core_expr){
                .tag = DY_CORE_EXPR_INTRO,
                .intro = {
                    .is_implicit = simple.is_implicit,
                    .polarity = simple.is_negative ? DY_POLARITY_NEGATIVE : DY_POLARITY_POSITIVE,
                    .tag = DY_CORE_INTRO_SIMPLE,
                    .simple = {
                        .tag = DY_CORE_SIMPLE_DECISION,
                        .direction = DY_LEFT,
                        .out = dy_core_expr_new(out)
                    }
                }
            };
        }

        for (size_t i = 1; i < simple.argument.index.value; ++i) {
            e = (struct dy_core_expr){
                .tag = DY_CORE_EXPR_INTRO,
                .intro = {
                    .polarity = simple.is_negative ? DY_POLARITY_NEGATIVE : DY_POLARITY_POSITIVE,
                    .is_implicit = simple.is_implicit,
                    .tag = DY_CORE_INTRO_SIMPLE,
                    .simple = {
                        .tag = DY_CORE_SIMPLE_DECISION,
                        .direction = DY_RIGHT,
                        .out = dy_core_expr_new(e)
                    }
                }
            };
        }

        return e;
    }
    case DY_AST_ARGUMENT_UNFOLD:
        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_INTRO,
            .intro = {
                .polarity = simple.is_negative ? DY_POLARITY_NEGATIVE : DY_POLARITY_POSITIVE,
                .is_implicit = simple.is_implicit,
                .tag = DY_CORE_INTRO_SIMPLE,
                .simple = {
                    .tag = DY_CORE_SIMPLE_UNFOLD,
                    .out = dy_core_expr_new(out)
                }
            }
        };
    case DY_AST_ARGUMENT_UNWRAP:
        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_INTRO,
            .intro = {
                .polarity = simple.is_negative ? DY_POLARITY_NEGATIVE : DY_POLARITY_POSITIVE,
                .is_implicit = simple.is_implicit,
                .tag = DY_CORE_INTRO_SIMPLE,
                .simple = {
                    .tag = DY_CORE_SIMPLE_UNWRAP,
                    .out = dy_core_expr_new(out)
                }
            }
        };
    }

    dy_bail("impossible");
}

struct dy_core_expr dy_ast_juxtaposition_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_juxtaposition juxtaposition)
{
    struct dy_core_expr left = dy_ast_expr_to_core(ctx, *juxtaposition.left);

    if (!juxtaposition.type) {
        return dy_convert_argument_app_without_type(ctx, left, juxtaposition.right, juxtaposition.is_implicit);
    }

    struct dy_core_expr type = dy_ast_expr_to_core(ctx, *juxtaposition.type);

    return dy_convert_argument_app(ctx, left, juxtaposition.right, juxtaposition.is_implicit, type);
}

struct dy_core_expr dy_ast_do_block_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_do_block do_block)
{
    if (!do_block.rest) {
        if (do_block.stmnt.tag != DY_AST_DO_BLOCK_STMNT_EXPR || do_block.stmnt.expr.is_inverted) {
            dy_bail("impossible");
        }

        return dy_ast_expr_to_core(ctx, *do_block.stmnt.expr.expr);
    }

    switch (do_block.stmnt.tag) {
    case DY_AST_DO_BLOCK_STMNT_EXPR: {
        struct dy_ast_expr fun = {
            .tag = DY_AST_EXPR_FUNCTION,
            .function = {
                .binding = {
                    .have_name = false,
                    .tag = DY_AST_BINDING_NOTHING
                },
                .expr = dy_ast_expr_new((struct dy_ast_expr){
                    .tag = DY_AST_EXPR_DO_BLOCK,
                    .do_block = dy_ast_do_block_retain(*do_block.rest)
                }),
                .is_implicit = false,
                .is_some = false
            }
        };

        if (do_block.stmnt.expr.is_inverted) {
            struct dy_ast_expr juxt = {
                .tag = DY_AST_EXPR_JUXTAPOSITION,
                .juxtaposition = {
                    .left = dy_ast_expr_retain_ptr(do_block.stmnt.expr.expr),
                    .right = {
                        .tag = DY_AST_ARGUMENT_EXPR,
                        .expr = dy_ast_expr_new(fun)
                    },
                    .type = NULL,
                    .is_implicit = false
                }
            };

            struct dy_core_expr ret = dy_ast_expr_to_core(ctx, juxt);

            dy_ast_expr_release(juxt);

            return ret;
        } else {
            struct dy_ast_expr juxt = {
                .tag = DY_AST_EXPR_JUXTAPOSITION,
                .juxtaposition = {
                    .left = dy_ast_expr_new(fun),
                    .right = {
                        .tag = DY_AST_ARGUMENT_EXPR,
                        .expr = dy_ast_expr_retain_ptr(do_block.stmnt.expr.expr)
                    },
                    .type = NULL,
                    .is_implicit = false
                }
            };

            struct dy_core_expr ret = dy_ast_expr_to_core(ctx, juxt);

            dy_ast_expr_release(juxt);

            return ret;
        }
    }
    case DY_AST_DO_BLOCK_STMNT_LET: {
        struct dy_ast_expr fun = {
            .tag = DY_AST_EXPR_FUNCTION,
            .function = {
                .binding = dy_ast_binding_retain(do_block.stmnt.let.binding),
                .expr = dy_ast_expr_new((struct dy_ast_expr){
                    .tag = DY_AST_EXPR_DO_BLOCK,
                    .do_block = dy_ast_do_block_retain(*do_block.rest)
                }),
                .is_implicit = false,
                .is_some = false
            }
        };

        if (do_block.stmnt.let.is_inverted) {
            struct dy_ast_expr juxt = {
                .tag = DY_AST_EXPR_JUXTAPOSITION,
                .juxtaposition = {
                    .left = dy_ast_expr_retain_ptr(do_block.stmnt.let.expr),
                    .right = {
                        .tag = DY_AST_ARGUMENT_EXPR,
                        .expr = dy_ast_expr_new(fun)
                    },
                    .type = NULL,
                    .is_implicit = false
                }
            };

            struct dy_core_expr ret = dy_ast_expr_to_core(ctx, juxt);

            dy_ast_expr_release(juxt);

            return ret;
        } else {
            struct dy_ast_expr juxt = {
                .tag = DY_AST_EXPR_JUXTAPOSITION,
                .juxtaposition = {
                    .left = dy_ast_expr_new(fun),
                    .right = {
                        .tag = DY_AST_ARGUMENT_EXPR,
                        .expr = dy_ast_expr_retain_ptr(do_block.stmnt.let.expr)
                    },
                    .type = NULL,
                    .is_implicit = false
                }
            };

            struct dy_core_expr ret = dy_ast_expr_to_core(ctx, juxt);

            dy_ast_expr_release(juxt);

            return ret;
        }
    }
    case DY_AST_DO_BLOCK_STMNT_DEF: {
        struct dy_core_expr arg = dy_ast_expr_to_core(ctx, *do_block.stmnt.def.expr);

        size_t id = ctx->running_id++;

        dy_array_add(&ctx->variable_replacements, &(struct dy_variable_replacement){
            .variable = dy_array_view(&do_block.stmnt.def.name),
            .replacement_id = id
        });

        struct dy_core_expr body = dy_ast_do_block_to_core(ctx, *do_block.rest);

        ctx->variable_replacements.num_elems--;

        struct dy_def_data d = {
            .arg = arg,
            .id = id,
            .body = body
        };

        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_CUSTOM,
            .custom = dy_def_create(d)
        };
    }
    }

    dy_bail("impossible");
}

struct dy_core_expr dy_ast_variable_to_core(struct dy_ast_to_core_ctx *ctx, const dy_array_t *variable)
{
    dy_string_t s = dy_array_view(variable);

    for (size_t i = ctx->variable_replacements.num_elems; i-- > 0;) {
        const struct dy_variable_replacement *replacement = dy_array_pos(&ctx->variable_replacements, i);

        if (dy_string_are_equal(replacement->variable, s)) {
            return (struct dy_core_expr){
                .tag = DY_CORE_EXPR_VARIABLE,
                .variable_id = replacement->replacement_id
            };
        }
    }

    dy_array_retain(variable);

    struct dy_uv_data d = {
        .var = *variable
    };

    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_CUSTOM,
        .custom = dy_uv_create(d)
    };
}

struct dy_core_expr dy_ast_any_to_core(struct dy_ast_to_core_ctx *ctx)
{
    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_ANY
    };
}

struct dy_core_expr dy_ast_void_to_core(struct dy_ast_to_core_ctx *ctx)
{
    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_VOID
    };
}

struct dy_core_expr dy_ast_string_to_core(struct dy_ast_to_core_ctx *ctx, const dy_array_t *string)
{
    dy_array_retain(string);

    struct dy_string_data d = {
        .value = *string
    };

    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_CUSTOM,
        .custom = dy_string_create(d)
    };
}

struct dy_core_expr dy_ast_map_some_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_map_some map_some)
{
    size_t inference_id1 = ctx->running_id++;
    bool have_inference_id1 = false;

    struct dy_core_expr type;
    if (map_some.type) {
        type = dy_ast_expr_to_core(ctx, *map_some.type);
    } else {
        have_inference_id1 = true;

        type = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_INFERENCE_VAR,
            .inference_var_id = inference_id1
        };
    }

    size_t replacement_id = ctx->running_id++;

    dy_array_add(&ctx->variable_replacements, &(struct dy_variable_replacement){
        .variable = dy_array_view(&map_some.name),
        .replacement_id = replacement_id
    });

    size_t inference_id2;
    bool have_inference_id2 = false;
    struct dy_core_assumption ass = dy_construct_bare_function(ctx, map_some.binding, *map_some.expr, &inference_id2, &have_inference_id2);

    ctx->variable_replacements.num_elems--;

    struct dy_core_expr map = {
        .tag = DY_CORE_EXPR_MAP,
        .map = {
            .tag = DY_CORE_MAP_ASSUMPTION,
            .assumption = {
                .id = replacement_id,
                .type = dy_core_expr_new(type),
                .assumption = ass,
                .dependence = DY_CORE_MAP_DEPENDENCE_NOT_CHECKED
            },
            .is_implicit = map_some.is_implicit
        }
    };

    if (have_inference_id2) {
        map = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_INFERENCE_CTX,
            .inference_ctx = {
                .id = inference_id2,
                .polarity = DY_POLARITY_NEGATIVE,
                .expr = dy_core_expr_new(map)
            }
        };
    }

    if (have_inference_id1) {
        map = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_INFERENCE_CTX,
            .inference_ctx = {
                .id = inference_id1,
                .polarity = DY_POLARITY_NEGATIVE,
                .expr = dy_core_expr_new(map)
            }
        };
    }

    return map;
}

struct dy_core_expr dy_ast_map_either_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_map_either map_either)
{
    dy_array_t inference_ids = dy_array_create(sizeof(size_t), DY_ALIGNOF(size_t), 8);
    struct dy_converted_map_either_body converted_body = dy_convert_map_either_body(ctx, map_either.body, map_either.is_implicit, &inference_ids);

    struct dy_core_expr e;
    switch (converted_body.tag) {
    case DY_CONVERTED_MAP_EITHER_BODY_ASSUMPTION:
        e = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_INTRO,
            .intro = {
                .polarity = DY_POLARITY_POSITIVE,
                .is_implicit = map_either.is_implicit,
                .tag = DY_CORE_INTRO_COMPLEX,
                .complex = {
                    .tag = DY_CORE_COMPLEX_ASSUMPTION,
                    .assumption = converted_body.assumption
                }
            }
        };
        break;
    case DY_CONVERTED_MAP_EITHER_BODY_CHOICE_MAP:
        e = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_MAP,
            .map = {
                .is_implicit = map_either.is_implicit,
                .tag = DY_CORE_MAP_CHOICE,
                .choice = converted_body.choice_map
            }
        };
        break;
    }

    for (size_t i = inference_ids.num_elems; i-- > 0;) {
        size_t id = *(size_t *)dy_array_pos(&inference_ids, i);

        e = (struct dy_core_expr){
            .tag = DY_CORE_EXPR_INFERENCE_CTX,
            .inference_ctx = {
                .id = id,
                .polarity = DY_POLARITY_NEGATIVE,
                .expr = dy_core_expr_new(e)
            }
        };
    }

    dy_array_release(&inference_ids);

    return e;
}

struct dy_core_expr dy_ast_map_fin_to_core(struct dy_ast_to_core_ctx *ctx, struct dy_ast_map_fin map_fin)
{
    size_t replacement_id = ctx->running_id++;

    dy_array_add(&ctx->variable_replacements, &(struct dy_variable_replacement){
        .variable = dy_array_view(&map_fin.name),
        .replacement_id = replacement_id
    });

    size_t inference_id;
    bool have_inference_id = false;
    struct dy_core_assumption ass = dy_construct_bare_function(ctx, map_fin.binding, *map_fin.expr, &inference_id, &have_inference_id);

    ctx->variable_replacements.num_elems--;

    struct dy_core_expr map = {
        .tag = DY_CORE_EXPR_MAP,
        .map = {
            .tag = DY_CORE_MAP_RECURSION,
            .recursion = {
                .id = replacement_id,
                .assumption = ass,
                .dependence = DY_CORE_MAP_DEPENDENCE_NOT_CHECKED
            },
            .is_implicit = map_fin.is_implicit
        }
    };

    if (!have_inference_id) {
        return map;
    }

    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_INFERENCE_CTX,
        .inference_ctx = {
            .id = inference_id,
            .polarity = DY_POLARITY_NEGATIVE,
            .expr = dy_core_expr_new(map)
        }
    };
}

struct dy_core_expr dy_construct_function(struct dy_ast_to_core_ctx *ctx, struct dy_ast_binding binding, struct dy_ast_expr expr, bool is_implicit, bool is_negative)
{
    enum dy_polarity polarity = is_negative ? DY_POLARITY_NEGATIVE : DY_POLARITY_POSITIVE;

    switch (binding.tag) {
    case DY_AST_BINDING_NOTHING: {
        size_t id = ctx->running_id++;

        if (binding.have_name) {
            dy_array_add(&ctx->variable_replacements, &(struct dy_variable_replacement){
                .variable = dy_array_view(&binding.name),
                .replacement_id = id
            });
        }

        struct dy_core_expr e = dy_ast_expr_to_core(ctx, expr);

        if (binding.have_name) {
            ctx->variable_replacements.num_elems--;
        }

        return dy_make_function_without_type(ctx, id, e, is_implicit, polarity);
    }
    case DY_AST_BINDING_TYPE: {
        struct dy_core_expr type = dy_ast_expr_to_core(ctx, *binding.type);

        size_t id = ctx->running_id++;

        if (binding.have_name) {
            dy_array_add(&ctx->variable_replacements, &(struct dy_variable_replacement){
                .variable = dy_array_view(&binding.name),
                .replacement_id = id
            });
        }

        struct dy_core_expr e = dy_ast_expr_to_core(ctx, expr);

        if (binding.have_name) {
            ctx->variable_replacements.num_elems--;
        }

        return (struct dy_core_expr){
            .tag = DY_CORE_EXPR_INTRO,
            .intro = {
                .is_implicit = is_implicit,
                .polarity = polarity,
                .tag = DY_CORE_INTRO_COMPLEX,
                .complex = {
                    .tag = DY_CORE_COMPLEX_ASSUMPTION,
                    .assumption = {
                        .id = id,
                        .type = dy_core_expr_new(type),
                        .expr = dy_core_expr_new(e)
                    }
                }
            }
        };
    }
    case DY_AST_BINDING_PATTERN: {
        size_t id = ctx->running_id++;

        if (binding.have_name) {
            dy_array_add(&ctx->variable_replacements, &(struct dy_variable_replacement){
                .variable = dy_array_view(&binding.name),
                .replacement_id = id
            });
        }

        struct dy_core_expr id_expr = {
            .tag = DY_CORE_EXPR_VARIABLE,
            .variable_id = id
        };

        struct dy_core_expr e = dy_process_pattern(ctx, id_expr, binding.pattern, expr);

        if (binding.have_name) {
            ctx->variable_replacements.num_elems--;
        }

        return dy_make_function_without_type(ctx, id, e, is_implicit, polarity);
    }
    }

    dy_bail("impossible");
}

struct dy_core_assumption dy_construct_bare_function(struct dy_ast_to_core_ctx *ctx, struct dy_ast_binding binding, struct dy_ast_expr expr, size_t *inference_id, bool *have_inference_id)
{
    switch (binding.tag) {
    case DY_AST_BINDING_NOTHING: {
        size_t id = ctx->running_id++;

        if (binding.have_name) {
            dy_array_add(&ctx->variable_replacements, &(struct dy_variable_replacement){
                .variable = dy_array_view(&binding.name),
                .replacement_id = id
            });
        }

        struct dy_core_expr e = dy_ast_expr_to_core(ctx, expr);

        if (binding.have_name) {
            ctx->variable_replacements.num_elems--;
        }

        *inference_id = ctx->running_id++;
        *have_inference_id = true;

        return (struct dy_core_assumption){
            .id = id,
            .type = dy_core_expr_new((struct dy_core_expr){
                .tag = DY_CORE_EXPR_INFERENCE_VAR,
                .inference_var_id = *inference_id
            }),
            .expr = dy_core_expr_new(e)
        };
    }
    case DY_AST_BINDING_TYPE: {
        struct dy_core_expr type = dy_ast_expr_to_core(ctx, *binding.type);

        size_t id = ctx->running_id++;

        if (binding.have_name) {
            dy_array_add(&ctx->variable_replacements, &(struct dy_variable_replacement){
                .variable = dy_array_view(&binding.name),
                .replacement_id = id
            });
        }

        struct dy_core_expr e = dy_ast_expr_to_core(ctx, expr);

        if (binding.have_name) {
            ctx->variable_replacements.num_elems--;
        }

        *have_inference_id = false;

        return (struct dy_core_assumption){
            .id = id,
            .type = dy_core_expr_new(type),
            .expr = dy_core_expr_new(e)
        };
    }
    case DY_AST_BINDING_PATTERN: {
        size_t id = ctx->running_id++;

        if (binding.have_name) {
            dy_array_add(&ctx->variable_replacements, &(struct dy_variable_replacement){
                .variable = dy_array_view(&binding.name),
                .replacement_id = id
            });
        }

        struct dy_core_expr id_expr = {
            .tag = DY_CORE_EXPR_VARIABLE,
            .variable_id = id
        };

        struct dy_core_expr e = dy_process_pattern(ctx, id_expr, binding.pattern, expr);

        if (binding.have_name) {
            ctx->variable_replacements.num_elems--;
        }

        *inference_id = ctx->running_id++;
        *have_inference_id = true;

        return (struct dy_core_assumption){
            .id = id,
            .type = dy_core_expr_new((struct dy_core_expr){
                .tag = DY_CORE_EXPR_INFERENCE_VAR,
                .inference_var_id = *inference_id
            }),
            .expr = dy_core_expr_new(e)
        };
    }
    }

    dy_bail("impossible");
}

struct dy_core_expr dy_make_function_without_type(struct dy_ast_to_core_ctx *ctx, size_t id, struct dy_core_expr expr, bool is_implicit, enum dy_polarity polarity)
{
    size_t inference_id = ctx->running_id++;

    struct dy_core_expr inference_id_expr = {
        .tag = DY_CORE_EXPR_INFERENCE_VAR,
        .inference_var_id = inference_id
    };

    struct dy_core_expr fun = {
        .tag = DY_CORE_EXPR_INTRO,
        .intro = {
            .is_implicit = is_implicit,
            .polarity = polarity,
            .tag = DY_CORE_INTRO_COMPLEX,
            .complex = {
                .tag = DY_CORE_COMPLEX_ASSUMPTION,
                .assumption = {
                    .id = id,
                    .type = dy_core_expr_new(inference_id_expr),
                    .expr = dy_core_expr_new(expr)
                }
            }
        }
    };

    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_INFERENCE_CTX,
        .inference_ctx = {
            .id = inference_id,
            .polarity = DY_POLARITY_NEGATIVE,
            .expr = dy_core_expr_new(fun)
        }
    };
}

struct dy_core_expr dy_process_pattern(struct dy_ast_to_core_ctx *ctx, struct dy_core_expr pattern_expr, struct dy_ast_pattern pattern, struct dy_ast_expr final_expr)
{
    switch (pattern.tag) {
    case DY_AST_PATTERN_SIMPLE: {
        struct dy_core_expr left = dy_construct_function(ctx, *pattern.simple.binding, final_expr, false, false);

        struct dy_core_expr right = dy_convert_argument_app_without_type(ctx, pattern_expr, pattern.simple.arg, pattern.simple.is_implicit);

        return dy_convert_juxtaposition_without_type(ctx, left, right, false);
    }
    case DY_AST_PATTERN_LIST:
        if (pattern.list.body.next) {
            struct dy_ast_expr new_final_expr = {
                .tag = DY_AST_EXPR_FUNCTION,
                .function = {
                    .binding = {
                        .have_name = false,
                        .tag = DY_AST_BINDING_PATTERN,
                        .pattern = {
                            .tag = DY_AST_PATTERN_LIST,
                            .list = {
                                .is_implicit = pattern.list.is_implicit,
                                .body = *pattern.list.body.next
                            }
                        }
                    },
                    .expr = dy_ast_expr_new(final_expr),
                    .is_implicit = false,
                    .is_some = false
                }
            };

            struct dy_core_expr left = dy_construct_function(ctx, *pattern.list.body.binding, new_final_expr, false, false);

            struct dy_core_expr right = dy_make_direction_app_without_type(ctx, pattern_expr, DY_LEFT, false);

            left = dy_convert_juxtaposition_without_type(ctx, left, right, false);

            right = dy_make_direction_app_without_type(ctx, pattern_expr, DY_RIGHT, false);

            return dy_convert_juxtaposition_without_type(ctx, left, right, false);
        } else {
            struct dy_core_expr left = dy_construct_function(ctx, *pattern.list.body.binding, final_expr, false, false);
            return dy_convert_juxtaposition_without_type(ctx, left, pattern_expr, false);
        }
    }

    dy_bail("impossible");
}

struct dy_core_expr dy_convert_juxtaposition_without_type(struct dy_ast_to_core_ctx *ctx, struct dy_core_expr left, struct dy_core_expr right, bool is_implicit)
{
    size_t inference_id = ctx->running_id++;

    struct dy_core_expr inference_id_expr = {
        .tag = DY_CORE_EXPR_INFERENCE_VAR,
        .inference_var_id = inference_id
    };

    struct dy_core_expr app = {
        .tag = DY_CORE_EXPR_ELIM,
        .elim = {
            .expr = dy_core_expr_new(left),
            .simple = {
                .tag = DY_CORE_SIMPLE_PROOF,
                .proof = dy_core_expr_new(right),
                .out = dy_core_expr_new(inference_id_expr)
            },
            .check_result = DY_MAYBE,
            .is_implicit = is_implicit
        }
    };

    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_INFERENCE_CTX,
        .inference_ctx = {
            .id = inference_id,
            .polarity = DY_POLARITY_POSITIVE,
            .expr = dy_core_expr_new(app)
        }
    };
}

struct dy_core_expr dy_convert_juxtaposition(struct dy_ast_to_core_ctx *ctx, struct dy_core_expr left, struct dy_core_expr right, struct dy_core_expr type, bool is_implicit)
{
    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_ELIM,
        .elim = {
            .expr = dy_core_expr_new(left),
            .simple = {
                .tag = DY_CORE_SIMPLE_PROOF,
                .proof = dy_core_expr_new(right),
                .out = dy_core_expr_new(type)
            },
            .check_result = DY_MAYBE,
            .is_implicit = is_implicit
        }
    };
}

struct dy_core_expr dy_convert_argument_app_without_type(struct dy_ast_to_core_ctx *ctx, struct dy_core_expr left, struct dy_ast_argument right, bool is_implicit)
{
    switch (right.tag) {
    case DY_AST_ARGUMENT_EXPR:
        return dy_convert_juxtaposition_without_type(ctx, left, dy_ast_expr_to_core(ctx, *right.expr), is_implicit);
    case DY_AST_ARGUMENT_INDEX:
        return dy_convert_index_app_without_type(ctx, left, right.index, is_implicit);
    case DY_AST_ARGUMENT_UNFOLD:
        return dy_convert_unfold_app_without_type(ctx, left, is_implicit);
    case DY_AST_ARGUMENT_UNWRAP:
        return dy_convert_unwrap_app_without_type(ctx, left, is_implicit);
    }

    dy_bail("impossible");
}

struct dy_core_expr dy_make_direction_app_without_type(struct dy_ast_to_core_ctx *ctx, struct dy_core_expr left, enum dy_direction direction, bool is_implicit)
{
    size_t inference_id = ctx->running_id++;

    struct dy_core_expr inference_id_expr = {
        .tag = DY_CORE_EXPR_INFERENCE_VAR,
        .inference_var_id = inference_id
    };

    struct dy_core_expr app = {
        .tag = DY_CORE_EXPR_ELIM,
        .elim = {
            .expr = dy_core_expr_new(left),
            .simple = {
                .tag = DY_CORE_SIMPLE_DECISION,
                .direction = direction,
                .out = dy_core_expr_new(inference_id_expr)
            },
            .is_implicit = is_implicit,
            .check_result = DY_MAYBE
        }
    };

    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_INFERENCE_CTX,
        .inference_ctx = {
            .id = inference_id,
            .polarity = DY_POLARITY_POSITIVE,
            .expr = dy_core_expr_new(app)
        }
    };
}

struct dy_core_expr dy_convert_index_app_without_type(struct dy_ast_to_core_ctx *ctx, struct dy_core_expr left, struct dy_ast_argument_index index, bool is_implicit)
{
    if (index.value == 1) {
        if (index.is_maximum) {
            return left;
        } else {
            return dy_make_direction_app_without_type(ctx, left, DY_LEFT, is_implicit);
        }
    }

    struct dy_core_expr e = dy_make_direction_app_without_type(ctx, left, DY_RIGHT, is_implicit);

    index.value -= 1;

    return dy_convert_index_app_without_type(ctx, e, index, is_implicit);
}

struct dy_core_expr dy_convert_unfold_app_without_type(struct dy_ast_to_core_ctx *ctx, struct dy_core_expr left, bool is_implicit)
{
    size_t inference_id = ctx->running_id++;

    struct dy_core_expr inference_id_expr = {
        .tag = DY_CORE_EXPR_INFERENCE_VAR,
        .inference_var_id = inference_id
    };

    struct dy_core_expr app = {
        .tag = DY_CORE_EXPR_ELIM,
        .elim = {
            .expr = dy_core_expr_new(left),
            .simple = {
                .tag = DY_CORE_SIMPLE_UNFOLD,
                .out = dy_core_expr_new(inference_id_expr)
            },
            .is_implicit = is_implicit,
            .check_result = DY_MAYBE
        }
    };

    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_INFERENCE_CTX,
        .inference_ctx = {
            .id = inference_id,
            .polarity = DY_POLARITY_POSITIVE,
            .expr = dy_core_expr_new(app)
        }
    };
}

struct dy_core_expr dy_convert_unwrap_app_without_type(struct dy_ast_to_core_ctx *ctx, struct dy_core_expr left, bool is_implicit)
{
    size_t inference_id = ctx->running_id++;

    struct dy_core_expr inference_id_expr = {
        .tag = DY_CORE_EXPR_INFERENCE_VAR,
        .inference_var_id = inference_id
    };

    struct dy_core_expr app = {
        .tag = DY_CORE_EXPR_ELIM,
        .elim = {
            .expr = dy_core_expr_new(left),
            .simple = {
                .tag = DY_CORE_SIMPLE_UNWRAP,
                .out = dy_core_expr_new(inference_id_expr)
            },
            .is_implicit = is_implicit,
            .check_result = DY_MAYBE
        }
    };

    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_INFERENCE_CTX,
        .inference_ctx = {
            .id = inference_id,
            .polarity = DY_POLARITY_POSITIVE,
            .expr = dy_core_expr_new(app)
        }
    };
}


struct dy_core_expr dy_convert_argument_app(struct dy_ast_to_core_ctx *ctx, struct dy_core_expr left, struct dy_ast_argument right, bool is_implicit, struct dy_core_expr type)
{
    switch (right.tag) {
    case DY_AST_ARGUMENT_EXPR:
        return dy_convert_juxtaposition(ctx, left, dy_ast_expr_to_core(ctx, *right.expr), type, is_implicit);
    case DY_AST_ARGUMENT_INDEX:
        return dy_convert_index_app(ctx, left, right.index, type, is_implicit);
    case DY_AST_ARGUMENT_UNFOLD:
        return dy_convert_unfold_app(ctx, left, type, is_implicit);
    case DY_AST_ARGUMENT_UNWRAP:
        return dy_convert_unwrap_app(ctx, left, type, is_implicit);
    }

    dy_bail("impossible");
}

struct dy_core_expr dy_make_direction_app(struct dy_ast_to_core_ctx *ctx, struct dy_core_expr left, enum dy_direction direction, struct dy_core_expr type, bool is_implicit)
{
    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_ELIM,
        .elim = {
            .expr = dy_core_expr_new(left),
            .simple = {
                .tag = DY_CORE_SIMPLE_DECISION,
                .direction = direction,
                .out = dy_core_expr_new(type)
            },
            .is_implicit = is_implicit,
            .check_result = DY_MAYBE
        }
    };
}

struct dy_core_expr dy_convert_index_app(struct dy_ast_to_core_ctx *ctx, struct dy_core_expr left, struct dy_ast_argument_index index, struct dy_core_expr type, bool is_implicit)
{
    if (index.value == 1) {
        if (index.is_maximum) {
            return left;
        } else {
            return dy_make_direction_app(ctx, left, DY_LEFT, type, is_implicit);
        }
    }

    struct dy_core_expr e = dy_make_direction_app(ctx, left, DY_RIGHT, type, is_implicit);

    index.value -= 1;

    return dy_convert_index_app(ctx, e, index, type, is_implicit);
}

struct dy_core_expr dy_convert_unfold_app(struct dy_ast_to_core_ctx *ctx, struct dy_core_expr left, struct dy_core_expr type, bool is_implicit)
{
    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_ELIM,
        .elim = {
            .expr = dy_core_expr_new(left),
            .simple = {
                .tag = DY_CORE_SIMPLE_UNFOLD,
                .out = dy_core_expr_new(type)
            },
            .is_implicit = is_implicit,
            .check_result = DY_MAYBE
        }
    };
}

struct dy_core_expr dy_convert_unwrap_app(struct dy_ast_to_core_ctx *ctx, struct dy_core_expr left, struct dy_core_expr type, bool is_implicit)
{
    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_ELIM,
        .elim = {
            .expr = dy_core_expr_new(left),
            .simple = {
                .tag = DY_CORE_SIMPLE_UNWRAP,
                .out = dy_core_expr_new(type)
            },
            .is_implicit = is_implicit,
            .check_result = DY_MAYBE
        }
    };
}

struct dy_core_expr dy_convert_list_body(struct dy_ast_to_core_ctx *ctx, struct dy_ast_list_body list_body, bool is_implicit, bool is_negative)
{
    struct dy_core_expr e = dy_ast_expr_to_core(ctx, *list_body.expr);

    if (!list_body.next) {
        return e;
    }

    struct dy_core_expr rest = dy_convert_list_body(ctx, *list_body.next, is_implicit, is_negative);

    return (struct dy_core_expr){
        .tag = DY_CORE_EXPR_INTRO,
        .intro = {
            .is_implicit = is_implicit,
            .polarity = is_negative ? DY_POLARITY_NEGATIVE : DY_POLARITY_POSITIVE,
            .tag = DY_CORE_INTRO_COMPLEX,
            .complex = {
                .tag = DY_CORE_COMPLEX_CHOICE,
                .choice = {
                    .left = dy_core_expr_new(e),
                    .right = dy_core_expr_new(rest)
                }
            }
        }
    };
}

struct dy_converted_map_either_body dy_convert_map_either_body(struct dy_ast_to_core_ctx *ctx, struct dy_ast_map_either_body body, bool is_implicit, dy_array_t *inference_ids)
{
    size_t inference_id;
    bool have_inference_id = false;
    struct dy_core_assumption ass = dy_construct_bare_function(ctx, body.binding, *body.expr, &inference_id, &have_inference_id);

    if (have_inference_id) {
        dy_array_add(inference_ids, &inference_id);
    }

    if (!body.next) {
        return (struct dy_converted_map_either_body){
            .tag = DY_CONVERTED_MAP_EITHER_BODY_ASSUMPTION,
            .assumption = ass
        };
    }

    struct dy_converted_map_either_body rest = dy_convert_map_either_body(ctx, *body.next, is_implicit, inference_ids);

    if (rest.tag == DY_CONVERTED_MAP_EITHER_BODY_ASSUMPTION) {
        return (struct dy_converted_map_either_body){
            .tag = DY_CONVERTED_MAP_EITHER_BODY_CHOICE_MAP,
            .choice_map = {
                .assumption_left = ass,
                .left_dependence = DY_CORE_MAP_DEPENDENCE_NOT_CHECKED,
                .assumption_right = rest.assumption,
                .right_dependence = DY_CORE_MAP_DEPENDENCE_NOT_CHECKED
            }
        };
    }

    if (rest.tag != DY_CONVERTED_MAP_EITHER_BODY_CHOICE_MAP) {
        dy_bail("impossible");
    }

    size_t id = ctx->running_id++;
    size_t id_type_inference_id = ctx->running_id++;
    size_t elim_result_inference_id = ctx->running_id++;

    dy_array_add(inference_ids, &id_type_inference_id);

    struct dy_core_expr elim = {
        .tag = DY_CORE_EXPR_ELIM,
        .elim = {
            .expr = dy_core_expr_new((struct dy_core_expr){
                .tag = DY_CORE_EXPR_MAP,
                .map = {
                    .is_implicit = is_implicit,
                    .tag = DY_CORE_MAP_CHOICE,
                    .choice = rest.choice_map
                }
            }),
            .simple = {
                .tag = DY_CORE_SIMPLE_PROOF,
                .proof = dy_core_expr_new((struct dy_core_expr){
                    .tag = DY_CORE_EXPR_VARIABLE,
                    .variable_id = id
                }),
                .out = dy_core_expr_new((struct dy_core_expr){
                    .tag = DY_CORE_EXPR_INFERENCE_VAR,
                    .inference_var_id = elim_result_inference_id
                })
            },
            .is_implicit = false,
            .check_result = DY_MAYBE,
            .eval_immediately = false
        }
    };

    struct dy_core_expr inference_ctx = {
        .tag = DY_CORE_EXPR_INFERENCE_CTX,
        .inference_ctx = {
            .id = elim_result_inference_id,
            .polarity = DY_POLARITY_POSITIVE,
            .expr = dy_core_expr_new(elim)
        }
    };

    return (struct dy_converted_map_either_body){
        .tag = DY_CONVERTED_MAP_EITHER_BODY_CHOICE_MAP,
        .choice_map = {
            .assumption_left = ass,
            .left_dependence = DY_CORE_MAP_DEPENDENCE_NOT_CHECKED,
            .assumption_right = {
                .id = id,
                .type = dy_core_expr_new((struct dy_core_expr){
                    .tag = DY_CORE_EXPR_INFERENCE_VAR,
                    .inference_var_id = id_type_inference_id
                }),
                .expr = dy_core_expr_new(inference_ctx)
            },
            .right_dependence = DY_CORE_MAP_DEPENDENCE_NOT_CHECKED
        }
    };
}
