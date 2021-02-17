/*
 * Copyright 2020-2021 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "syntax/utf8_to_ast.h"
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

    struct dy_utf8_to_ast_ctx utf8_to_ast_ctx = {
        .stream = stream_from_string(s)
    };

    struct dy_ast_do_block ast;
    if (!dy_utf8_to_ast_file(&utf8_to_ast_ctx, &ast)) {
        return "Failed to parse program.\n";
    }

    dy_array_t custom_shared = dy_array_create(sizeof(struct dy_core_custom_shared), DY_ALIGNOF(struct dy_core_custom_shared), 3);

    dy_uv_register(&custom_shared);
    dy_def_register(&custom_shared);
    dy_string_register(&custom_shared);
    dy_string_type_register(&custom_shared);

    struct dy_ast_to_core_ctx ast_to_core_ctx = {
        .running_id = 0,
        .variable_replacements = dy_array_create(sizeof(struct dy_variable_replacement), DY_ALIGNOF(struct dy_variable_replacement), 128)
    };

    struct dy_core_expr core = dy_ast_do_block_to_core(&ast_to_core_ctx, ast);

    dy_ast_do_block_release(ast);

    struct dy_core_ctx core_ctx = {
        .running_id = ast_to_core_ctx.running_id,
        .free_variables = dy_array_create(sizeof(struct dy_free_var), DY_ALIGNOF(struct dy_free_var), 64),
        .captured_inference_vars = dy_array_create(sizeof(struct dy_captured_inference_var), DY_ALIGNOF(struct dy_captured_inference_var), 64),
        .recovered_negative_inference_ids = dy_array_create(sizeof(size_t), DY_ALIGNOF(size_t), 8),
        .past_subtype_checks = dy_array_create(sizeof(struct dy_core_past_subtype_check), DY_ALIGNOF(struct dy_core_past_subtype_check), 64),
        .equal_variables = dy_array_create(sizeof(struct dy_equal_variables), DY_ALIGNOF(struct dy_equal_variables), 64),
        .free_ids_arrays = dy_array_create(sizeof(dy_array_t), DY_ALIGNOF(dy_array_t), 8),
        .constraints = dy_array_create(sizeof(struct dy_constraint), DY_ALIGNOF(struct dy_constraint), 64),
        .custom_shared = custom_shared
    };

    struct dy_core_expr new_core;
    if (dy_check_expr(&core_ctx, core, &new_core)) {
        dy_core_expr_release(&core_ctx, core);
        core = new_core;
    }

    bool is_value = false;
    if (dy_eval_expr(&core_ctx, core, &is_value, &new_core)) {
        dy_core_expr_release(&core_ctx, core);
        core = new_core;
    }

    dy_array_t stringified_expr = dy_array_create(sizeof(char), DY_ALIGNOF(char), 64);
    dy_core_expr_to_string(&core_ctx, core, &stringified_expr);

    dy_core_expr_release(&core_ctx, core);

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
}
