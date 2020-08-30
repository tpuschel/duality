/*
 * Copyright 2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#define _GNU_SOURCE // For glibc's asprintf.

#include "syntax/parser.h"
#include "syntax/ast_to_core.h"

#include "core/check.h"
#include "core/eval.h"

#include <string.h>

/**
 * This file declares and implements the functions used
 * by the browser version of Duality via emscripten.
 */

const char *process_code(const char *text, size_t text_length_in_bytes);

static inline struct dy_stream stream_from_string(dy_string_t s);
static inline void null_stream(dy_array_t *buffer, void *env);

const char *process_code(const char *text, size_t text_length_in_bytes)
{
    dy_string_t s = {
        .ptr = text,
        .size = text_length_in_bytes
    };

    struct dy_parser_ctx parser_ctx = {
        .stream = stream_from_string(s),
        .string_arrays = dy_array_create(sizeof(dy_array_t *), DY_ALIGNOF(dy_array_t *), 32)
    };

    struct dy_ast_do_block program_ast;
    if (!dy_parse_file(&parser_ctx, &program_ast)) {
        return "Failed to parse program.\n";
    }

    struct dy_ast_to_core_ctx ast_to_core_ctx = {
        .running_id = 0,
        .bound_vars = dy_array_create(sizeof(struct dy_ast_to_core_bound_var), DY_ALIGNOF(struct dy_ast_to_core_bound_var), 64)
    };

    struct dy_core_expr program = dy_ast_do_block_to_core(&ast_to_core_ctx, program_ast);

    dy_ast_do_block_release(program_ast.body);

    struct dy_core_ctx core_ctx = {
        .running_id = ast_to_core_ctx.running_id,
        .bound_inference_vars = dy_array_create(sizeof(struct dy_bound_inference_var), DY_ALIGNOF(struct dy_bound_inference_var), 64),
        .already_visited_ids = dy_array_create(sizeof(size_t), DY_ALIGNOF(size_t), 64),
        .subtype_assumption_cache = dy_array_create(sizeof(struct dy_subtype_assumption), DY_ALIGNOF(struct dy_subtype_assumption), 64),
        .supertype_assumption_cache = dy_array_create(sizeof(struct dy_subtype_assumption), DY_ALIGNOF(struct dy_subtype_assumption), 64),
        .bindings = dy_array_create(sizeof(struct dy_core_binding), DY_ALIGNOF(struct dy_core_binding), 64),
        .equal_variables = dy_array_create(sizeof(struct dy_equal_variables), DY_ALIGNOF(struct dy_equal_variables), 64),
        .subtype_implicits = dy_array_create(sizeof(struct dy_core_binding), DY_ALIGNOF(struct dy_core_binding), 64),
        .free_ids_arrays = dy_array_create(sizeof(dy_array_t), DY_ALIGNOF(dy_array_t), 8),
        .constraints = dy_array_create(sizeof(struct dy_constraint), DY_ALIGNOF(struct dy_constraint), 64)
    };

    struct dy_core_expr checked_program;
    if (dy_check_expr(&core_ctx, program, &checked_program)) {
        dy_core_expr_release(program);
        program = checked_program;
    }

    bool is_value = false;
    struct dy_core_expr result = dy_eval_expr(&core_ctx, program, &is_value);

    dy_array_t stringified_expr = dy_array_create(sizeof(char), DY_ALIGNOF(char), 64);
    dy_core_expr_to_string(result, &stringified_expr);
    
    dy_core_expr_release(result);

    dy_array_add(&stringified_expr, &(char){ '\0' });

    return stringified_expr.buffer;
}

struct dy_stream stream_from_string(dy_string_t s)
{
    dy_array_t buffer = dy_array_create(sizeof(char), DY_ALIGNOF(char), s.size);
    for (size_t i = 0; i < s.size; ++i) {
        dy_array_add(&buffer, s.ptr + i);
    }

    return (struct dy_stream){
        .get_chars = null_stream,
        .buffer = buffer,
        .env = NULL,
        .current_index = 0
    };
}

void null_stream(dy_array_t *buffer, void *env)
{
    (void)buffer;
    (void)env;
    return;
}
