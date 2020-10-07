/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#define _GNU_SOURCE // For glibc's asprintf.

#include "syntax/parser.h"
#include "syntax/ast_to_core.h"

#include "core/check.h"
#include "core/eval.h"
#include "core/constraint.h"

#include "lsp/server.h"

#include "support/bail.h"

#include <string.h>

static void read_chunk(dy_array_t *buffer, void *env);

static const size_t CHUNK_SIZE = 1024;

static void print_core_expr(struct dy_core_ctx *ctx, FILE *file, struct dy_core_expr expr);

static bool core_has_error(struct dy_core_expr expr);

static bool print_core_errors(dy_array_t text_sources, FILE *file, struct dy_core_expr expr, const char *text, size_t text_size);

static void print_error_fragment(FILE *file, struct dy_range range, const char *text, size_t text_size);

int main(int argc, const char *argv[])
{
    if (argc > 1 && strcmp(argv[1], "--server") == 0) {
        return dy_lsp_run_server(stdin, stdout);
    }

    if (argc > 1 && strcmp(argv[1], "--debugger") == 0) {
        fprintf(stderr, "DAP not yet implemented!\n");
        return -1;
    }

    FILE *stream;
    if (argc > 1) {
        stream = fopen(argv[1], "r");
        if (stream == NULL) {
            perror("Error reading file: ");
            return -1;
        }
    } else {
        stream = stdin;
    }

    struct dy_parser_ctx parser_ctx = {
        .stream = {
            .get_chars = read_chunk,
            .buffer = dy_array_create(sizeof(char), DY_ALIGNOF(char), CHUNK_SIZE),
            .env = stream,
            .current_index = 0,
        },
        .string_arrays = dy_array_create(sizeof(dy_array_t *), DY_ALIGNOF(dy_array_t *), 32)
    };

    struct dy_ast_do_block program_ast;
    if (!dy_parse_file(&parser_ctx, &program_ast)) {
        fprintf(stderr, "Failed to parse program.\n");
        return -1;
    }

    struct dy_core_ctx core_ctx = {
        .running_id = 0,
        .bound_inference_vars = dy_array_create(sizeof(struct dy_bound_inference_var), DY_ALIGNOF(struct dy_bound_inference_var), 64),
        .already_visited_ids = dy_array_create(sizeof(size_t), DY_ALIGNOF(size_t), 64),
        .subtype_assumption_cache = dy_array_create(sizeof(struct dy_subtype_assumption), DY_ALIGNOF(struct dy_subtype_assumption), 64),
        .supertype_assumption_cache = dy_array_create(sizeof(struct dy_subtype_assumption), DY_ALIGNOF(struct dy_subtype_assumption), 64),
        .bindings = dy_array_create(sizeof(struct dy_core_binding), DY_ALIGNOF(struct dy_core_binding), 64),
        .equal_variables = dy_array_create(sizeof(struct dy_equal_variables), DY_ALIGNOF(struct dy_equal_variables), 64),
        .subtype_implicits = dy_array_create(sizeof(struct dy_core_binding), DY_ALIGNOF(struct dy_core_binding), 64),
        .free_ids_arrays = dy_array_create(sizeof(dy_array_t), DY_ALIGNOF(dy_array_t), 8),
        .constraints = dy_array_create(sizeof(struct dy_constraint), DY_ALIGNOF(struct dy_constraint), 64),
        .custom_shared = dy_array_create(sizeof(struct dy_core_custom_shared), DY_ALIGNOF(struct dy_core_custom_shared), 3)
    };

    dy_uv_register(&core_ctx);
    dy_def_register(&core_ctx);
    dy_string_register(&core_ctx);

    struct dy_ast_to_core_ctx ast_to_core_ctx = {
        .core_ctx = core_ctx,
        .bound_vars = dy_array_create(sizeof(struct dy_ast_to_core_bound_var), DY_ALIGNOF(struct dy_ast_to_core_bound_var), 64),
        .text_sources = dy_array_create(sizeof(struct dy_ast_to_core_text_source), DY_ALIGNOF(struct dy_ast_to_core_text_source), 64),
    };

    struct dy_core_expr program = dy_ast_do_block_to_core(&ast_to_core_ctx, program_ast);

    core_ctx = ast_to_core_ctx.core_ctx;

    dy_ast_do_block_release(program_ast.body);

    size_t constraint_start = core_ctx.constraints.num_elems;
    struct dy_core_expr checked_program;
    if (dy_check_expr(&core_ctx, program, &checked_program)) {
        dy_core_expr_release(&core_ctx, program);
        program = checked_program;
    }
    assert(constraint_start == core_ctx.constraints.num_elems);

    if (core_has_error(program)) {
        fprintf(stderr, "Errors:\n");
        print_core_errors(ast_to_core_ctx.text_sources, stderr, program, parser_ctx.stream.buffer.buffer, parser_ctx.stream.buffer.num_elems);
        return -1;
    }

    bool is_value = false;
    struct dy_core_expr result = dy_eval_expr(&core_ctx, program, &is_value);

    print_core_expr(&core_ctx, stdout, result);
    printf("\n");

    if (!is_value) {
        fprintf(stderr, "Unable to fully evaluate the program.\n");

        if (core_has_error(result)) {
            fprintf(stderr, "Errors:\n");
            print_core_errors(ast_to_core_ctx.text_sources, stderr, result, parser_ctx.stream.buffer.buffer, parser_ctx.stream.buffer.num_elems);
        }
        return -1;
    } else {
        return 0;
    }
}

void print_core_expr(struct dy_core_ctx *ctx, FILE *file, struct dy_core_expr expr)
{
    dy_array_t s = dy_array_create(sizeof(char), DY_ALIGNOF(char), 64);
    dy_core_expr_to_string(ctx, expr, &s);

    for (size_t i = 0; i < s.num_elems; ++i) {
        char c;
        dy_array_get(s, i, &c);
        fprintf(file, "%c", c);
    }

    dy_array_destroy(s);
}

void read_chunk(dy_array_t *buffer, void *env)
{
    FILE *stream = env;

    if (feof(stream) || ferror(stream)) {
        return;
    }

    dy_array_set_excess_capacity(buffer, CHUNK_SIZE);

    size_t num_bytes_read = fread(dy_array_excess_buffer(*buffer), sizeof(char), CHUNK_SIZE, stream);

    dy_array_add_to_size(buffer, num_bytes_read);
}

bool core_has_error(struct dy_core_expr expr)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_EQUALITY_MAP:
        return core_has_error(*expr.equality_map.e1) || core_has_error(*expr.equality_map.e2);
    case DY_CORE_EXPR_TYPE_MAP:
        return core_has_error(*expr.type_map.type) || core_has_error(*expr.type_map.expr);
    case DY_CORE_EXPR_EQUALITY_MAP_ELIM:
        if (expr.equality_map_elim.check_result == DY_NO) {
            return true;
        }

        return core_has_error(*expr.equality_map_elim.expr) || core_has_error(*expr.equality_map_elim.map.e1)
               || core_has_error(*expr.equality_map_elim.map.e2);
    case DY_CORE_EXPR_TYPE_MAP_ELIM:
        if (expr.type_map_elim.check_result == DY_NO) {
            return true;
        }

        return core_has_error(*expr.type_map_elim.expr) || core_has_error(*expr.type_map_elim.map.type)
               || core_has_error(*expr.type_map_elim.map.expr);
    case DY_CORE_EXPR_JUNCTION:
        return core_has_error(*expr.junction.e1) || core_has_error(*expr.junction.e2);
    case DY_CORE_EXPR_ALTERNATIVE:
        return core_has_error(*expr.alternative.first) || core_has_error(*expr.alternative.second);
    case DY_CORE_EXPR_VARIABLE:
        return false;
    case DY_CORE_EXPR_CUSTOM:
        if (expr.custom.id == dy_uv_id) {
            return true;
        }

        if (expr.custom.id == dy_def_id) {
            struct dy_def_data *data = expr.custom.data;

            return core_has_error(data->arg) || core_has_error(data->body);
        }

        return false;
    case DY_CORE_EXPR_INFERENCE_TYPE_MAP:
        dy_bail("should never be reached");
    case DY_CORE_EXPR_RECURSION:
        return core_has_error(*expr.recursion.type) || core_has_error(*expr.recursion.expr);
    case DY_CORE_EXPR_END:
        // fallthrough
    case DY_CORE_EXPR_SYMBOL:
        return false;
    }

    dy_bail("Impossible object type.");
}

bool print_core_errors(dy_array_t text_sources, FILE *file, struct dy_core_expr expr, const char *text, size_t text_size)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_EQUALITY_MAP:
        return print_core_errors(text_sources, file, *expr.equality_map.e1, text, text_size)
        && print_core_errors(text_sources, file, *expr.equality_map.e2, text, text_size);
    case DY_CORE_EXPR_TYPE_MAP:
        return print_core_errors(text_sources, file, *expr.type_map.type, text, text_size)
        && print_core_errors(text_sources, file, *expr.type_map.expr, text, text_size);
    case DY_CORE_EXPR_EQUALITY_MAP_ELIM: {
        bool err1 = print_core_errors(text_sources, file, *expr.equality_map_elim.expr, text, text_size);
        bool err2 = print_core_errors(text_sources, file, *expr.equality_map_elim.map.e1, text, text_size);
        bool err3 = print_core_errors(text_sources, file, *expr.equality_map_elim.map.e2, text, text_size);

        if (expr.equality_map_elim.check_result == DY_NO || !err1) {
            const struct dy_range *r = get_text_range(text_sources, expr.equality_map_elim.id);
            if (r == NULL) {
                return false;
            }

            print_error_fragment(stderr, *r, text, text_size);
        }

        return err2 && err3;
    }
    case DY_CORE_EXPR_TYPE_MAP_ELIM:
        return print_core_errors(text_sources, file, *expr.type_map_elim.expr, text, text_size)
        && print_core_errors(text_sources, file, *expr.type_map_elim.map.type, text, text_size)
        && print_core_errors(text_sources, file, *expr.type_map_elim.map.expr, text, text_size);
    case DY_CORE_EXPR_JUNCTION:
        return print_core_errors(text_sources, file, *expr.junction.e1, text, text_size)
        && print_core_errors(text_sources, file, *expr.junction.e2, text, text_size);
    case DY_CORE_EXPR_ALTERNATIVE:
        return print_core_errors(text_sources, file, *expr.alternative.first, text, text_size)
        && print_core_errors(text_sources, file, *expr.alternative.second, text, text_size);
    case DY_CORE_EXPR_VARIABLE:
        return true;
    case DY_CORE_EXPR_INFERENCE_TYPE_MAP:
        dy_bail("should never be reached");
    case DY_CORE_EXPR_RECURSION:
        return print_core_errors(text_sources, file, *expr.recursion.type, text, text_size)
        && print_core_errors(text_sources, file, *expr.recursion.expr, text, text_size);
    case DY_CORE_EXPR_END:
        return true;
    case DY_CORE_EXPR_CUSTOM:
        if (expr.custom.id == dy_uv_id) {
            struct dy_uv_data *data = expr.custom.data;

            fprintf(file, "Unbound variable: ");
            print_error_fragment(file, data->var.text_range, text, text_size);
        }

        if (expr.custom.id == dy_def_id) {
            struct dy_def_data *data = expr.custom.data;

            print_core_errors(text_sources, file, data->arg, text, text_size);
            print_core_errors(text_sources, file, data->body, text, text_size);
        }

        return true;
    case DY_CORE_EXPR_SYMBOL:
        return true;
    }

    dy_bail("Impossible object type.");
}

void print_error_fragment(FILE *file, struct dy_range range, const char *text, size_t text_size)
{
    assert(range.start < text_size);
    assert(range.end < text_size);

    for (size_t i = range.start; i < range.end; ++i) {
        fprintf(file, "%c", text[i]);
    }
    fprintf(file, "\n");
}
