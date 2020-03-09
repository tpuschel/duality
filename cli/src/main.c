/*
 * Copyright 2017-2020 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <duality/syntax/parser.h>
#include <duality/syntax/ast_to_core.h>

#include <duality/core/check.h>
#include <duality/core/eval.h>
#include <duality/core/constraint.h>

#include <duality/support/assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "lsp_server.h"

static void read_chunk(dy_array_t *buffer, void *env);

static const size_t CHUNK_SIZE = 1024;

static void print_core_expr(struct dy_core_expr expr, struct dy_allocator allocator);

static void print_string(FILE *stream, dy_string_t s);

static void print_unbound_vars_error(dy_array_t *unbound_vars);

int main(int argc, const char *argv[])
{
    if (argc > 1 && strcmp(argv[1], "--server") == 0) {
        return run_lsp_server();
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
            .buffer = dy_array_create(dy_allocator_stdlib(), sizeof(char), CHUNK_SIZE),
            .env = stream,
            .current_index = 0,
        },
        .allocator = dy_allocator_stdlib(),
        .arrays = dy_array_create(dy_allocator_stdlib(), sizeof(dy_array_t *), 32)
    };

    struct dy_ast_do_block program_ast;
    if (!dy_parse_file(&parser_ctx, &program_ast)) {
        fprintf(stderr, "Failed to parse program.\n");
        return -1;
    }

    size_t running_id = 0;
    dy_array_t *unbound_vars = dy_array_create(dy_allocator_stdlib(), sizeof(dy_string_t), 2);

    struct dy_ast_to_core_ctx ast_to_core_ctx = {
        .running_id = &running_id,
        .bound_vars = dy_array_create(dy_allocator_stdlib(), sizeof(struct dy_ast_to_core_bound_var), 64),
        .allocator = dy_allocator_stdlib(),
        .unbound_vars = unbound_vars
    };

    struct dy_core_expr program;
    dy_array_t *sub_maps = dy_array_create(dy_allocator_stdlib(), sizeof(struct dy_text_range_core_map), 2);
    if (!dy_ast_do_block_to_core(&ast_to_core_ctx, program_ast, &program, sub_maps)) {
        print_unbound_vars_error(unbound_vars);
        return -1;
    }

    printf("Core:\n");
    print_core_expr(program, dy_allocator_stdlib());
    printf("\n");

    struct dy_check_ctx check_ctx = {
        .running_id = &running_id,
        .allocator = dy_allocator_stdlib(),
        .successful_elims = dy_array_create(dy_allocator_stdlib(), sizeof(size_t), 32)
    };

    struct dy_core_expr checked_program;
    struct dy_constraint constraint;
    bool have_constraint = false;
    if (!dy_check_expr(check_ctx, program, &checked_program, &constraint, &have_constraint)) {
        fprintf(stderr, "Program failed check.\n");
        return -1;
    }
    if (have_constraint) {
        fprintf(stderr, "Constraint on top level??.\n");
        return -1;
    }

    printf("Checked Core:\n");
    print_core_expr(checked_program, dy_allocator_stdlib());
    printf("\n");

    struct dy_core_expr result;
    dy_ternary_t is_valid = dy_eval_expr(check_ctx, checked_program, &result);
    if (is_valid == DY_NO) {
        fprintf(stderr, "Program execution failed.\n");
        return -1;
    }

    printf("Result:\n");
    print_core_expr(result, dy_allocator_stdlib());
    printf("\n");

    return 0;
}

void print_string(FILE *stream, dy_string_t s)
{
    for (size_t i = 0; i < s.size; ++i) {
        fprintf(stream, "%c", s.ptr[i]);
    }
}

void print_unbound_vars_error(dy_array_t *unbound_vars)
{
    fprintf(stderr, "Error: Unbound variables: ");

    for (size_t i = 0; i < dy_array_size(unbound_vars); ++i) {
        dy_string_t unbound_var;
        dy_array_get(unbound_vars, i, &unbound_var);

        print_string(stderr, unbound_var);

        if (i < dy_array_size(unbound_vars) - 1) {
            fprintf(stderr, ", ");
        } else {
            fprintf(stderr, " ");
        }
    }

    fprintf(stderr, "\n");
}

void print_core_expr(struct dy_core_expr expr, struct dy_allocator allocator)
{
    dy_array_t *s = dy_array_create(allocator, 1, 64);
    dy_core_expr_to_string(expr, s);

    for (size_t i = 0; i < dy_array_size(s); ++i) {
        char c;
        dy_array_get(s, i, &c);
        printf("%c", c);
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

    size_t num_bytes_read = fread(dy_array_excess_buffer(buffer), sizeof(char), CHUNK_SIZE, stream);

    dy_array_add_to_size(buffer, num_bytes_read);
}