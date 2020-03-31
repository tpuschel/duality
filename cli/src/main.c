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

#include <duality/lsp/server.h>

#include <duality/support/assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

static void read_chunk(dy_array_t *buffer, void *env);

static const size_t CHUNK_SIZE = 1024;

static void print_core_expr(struct dy_core_expr expr);

static void print_string(FILE *stream, dy_string_t s);

static void print_unbound_vars_error(dy_array_t *unbound_vars);

static void print_constraint(struct dy_constraint c);

static size_t size_t_max(size_t a, size_t b);

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

    dy_obj_pool_t *ast_pool = dy_obj_pool_create(sizeof(struct dy_ast_expr), _Alignof(struct dy_ast_expr));

    struct dy_parser_ctx parser_ctx = {
        .stream = {
            .get_chars = read_chunk,
            .buffer = dy_array_create(sizeof(char), CHUNK_SIZE),
            .env = stream,
            .current_index = 0,
        },
        .string_arrays = dy_array_create(sizeof(dy_array_t *), 32),
        .pool = ast_pool
    };

    struct dy_ast_do_block program_ast;
    if (!dy_parse_file(&parser_ctx, &program_ast)) {
        fprintf(stderr, "Failed to parse program.\n");
        return -1;
    }

    dy_array_destroy(parser_ctx.stream.buffer);

    dy_obj_pool_t *core_expr_pool = dy_obj_pool_create(sizeof(struct dy_core_expr), _Alignof(struct dy_core_expr));

    dy_obj_pool_set_is_parent_cb(core_expr_pool, dy_core_expr_is_parent);

    size_t running_id = 0;
    dy_array_t *unbound_vars = dy_array_create(sizeof(dy_string_t), 2);

    struct dy_ast_to_core_ctx ast_to_core_ctx = {
        .running_id = &running_id,
        .bound_vars = dy_array_create(sizeof(struct dy_ast_to_core_bound_var), 64),
        .unbound_vars = unbound_vars,
        .core_expr_pool = core_expr_pool
    };

    struct dy_core_expr program;
    if (!dy_ast_do_block_to_core(&ast_to_core_ctx, program_ast, &program)) {
        print_unbound_vars_error(unbound_vars);
        return -1;
    }

    dy_ast_do_block_release(ast_pool, program_ast);

    /*
    printf("Core:\n");
    print_core_expr(program);
    printf("\n");
    */

    struct dy_core_ctx core_ctx = {
        .running_id = &running_id,
        .bound_constraints = dy_array_create(sizeof(struct dy_bound_constraint), 64),
        .expr_pool = core_expr_pool
    };

    struct dy_core_expr checked_program;
    struct dy_constraint constraint;
    bool have_constraint = false;
    if (!dy_check_expr(core_ctx, program, &checked_program, &constraint, &have_constraint)) {
        fprintf(stderr, "Program failed check.\n");
        return -1;
    }

    /*
    printf("Checked Core:\n");
    print_core_expr(checked_program);
    printf("\n");
    */

    if (have_constraint) {
        fprintf(stderr, "Constraint on top level??.\n");
        print_constraint(constraint);
        fprintf(stderr, "\n");
        return -1;
    }

    struct dy_core_expr result;
    dy_ternary_t is_valid = dy_eval_expr(core_ctx, checked_program, &result);
    if (is_valid == DY_NO) {
        fprintf(stderr, "Program execution failed.\n");
        return -1;
    }

    printf("Result: ");
    print_core_expr(result);
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

void print_core_expr(struct dy_core_expr expr)
{
    dy_array_t *s = dy_array_create(1, 64);
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

void print_constraint(struct dy_constraint c)
{
    switch (c.tag) {
    case DY_CONSTRAINT_SINGLE:
        if (c.single.range.have_subtype) {
            print_core_expr(c.single.range.subtype);
            printf(" <: ");
        }
        printf("%zu", c.single.id);
        if (c.single.range.have_supertype) {
            printf(" <: ");
            print_core_expr(c.single.range.supertype);
        }
        break;
    case DY_CONSTRAINT_MULTIPLE:
        printf("(");
        print_constraint(*c.multiple.c1);
        switch (c.multiple.polarity) {
        case DY_CORE_POLARITY_POSITIVE:
            printf(" & ");
            break;
        case DY_CORE_POLARITY_NEGATIVE:
            printf(" | ");
            break;
        }
        print_constraint(*c.multiple.c2);
        printf(")");
    }
}

size_t size_t_max(size_t a, size_t b)
{
    if (a > b) {
        return a;
    } else {
        return b;
    }
}
