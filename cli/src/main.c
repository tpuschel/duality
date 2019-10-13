/*
 * Copyright 2017 - 2019, Thorben Hasenpusch
 * Licensed under the MIT license.
 */

#include <duality/syntax/parser.h>
#include <duality/syntax/ast_to_core.h>

#include <duality/core/check.h>

#include <duality/core/eval.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

static dy_string_t read_file_whole(const char *path);
static dy_string_t read_stdin_until_eof(void);

static void *malloc_(size_t size, void *env);
static void *realloc_(void *ptr, size_t size, void *env);
static void free_(void *ptr, size_t size, void *env);

static const size_t INITIAL_STDIN_BUFFER_SIZE = 256;

static void print_core_expr(struct dy_core_expr expr, struct dy_allocator allocator);

int main(int argc, const char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Missing argument\n");
        return -1;
    }

    printf("Reading program...\n");

    dy_string_t program_text;
    if (strcmp(argv[1], "-") == 0) {
        program_text = read_stdin_until_eof();
    } else {
        program_text = read_file_whole(argv[1]);
    }

    struct dy_allocator allocator = {
        .alloc = malloc_,
        .realloc = realloc_,
        .free = free_,
        .env = NULL
    };

    size_t running_id = 0;
    size_t index_out;

    struct dy_parser_ctx parser_ctx = {
        .text = program_text,
        .index_in = 0,
        .index_out = &index_out,
        .errors = dy_array_create(allocator, sizeof(int), 1),
        .allocator = allocator
    };

    printf("Parsing...\n");

    struct dy_ast_do_block program_ast;
    if (!dy_parse_file(parser_ctx, &program_ast)) {
        fprintf(stderr, "Failed to parse program.\n");
        return -1;
    }

    struct dy_ast_to_core_ctx ast_to_core_ctx = {
        .running_id = &running_id,
        .bound_vars = dy_array_create(allocator, sizeof(struct dy_ast_to_core_bound_var), 64),
        .allocator = allocator
    };

    printf("Converting AST into CORE...\n");

    struct dy_core_expr program;
    if (!dy_ast_do_block_to_core(ast_to_core_ctx, program_ast, &program)) {
        fprintf(stderr, "Failed to translate AST to CORE.\n");
        return -1;
    }

    printf("\n");
    print_core_expr(program, allocator);
    printf("\n\n");

    struct dy_check_ctx check_ctx = {
        .running_id = &running_id,
        .allocator = allocator
    };

    printf("Checking program...\n");

    struct dy_core_expr checked_program;
    dy_ternary_t is_valid = dy_check_expr(check_ctx, program, &checked_program);

    if (is_valid == DY_NO) {
        fprintf(stderr, "Program failed check.\n");
        return -1;
    }

    printf("Evaluating program...\n");

    struct dy_core_expr result;
    is_valid = dy_eval_expr(check_ctx, checked_program, &result);
    if (is_valid == DY_NO) {
        fprintf(stderr, "Program execution failed.\n");
        return -1;
    }

    printf("Result:\n\n");
    print_core_expr(result, allocator);
    printf("\n");

    return 0;
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

dy_string_t read_file_whole(const char *path)
{
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        perror("Error");
        exit(EXIT_FAILURE);
    }

    if (fseek(f, 0, SEEK_END) == -1) {
        perror("Error");
        exit(EXIT_FAILURE);
    }

    long file_size = ftell(f);
    if (file_size == -1) {
        perror("Error");
        exit(EXIT_FAILURE);
    }

    rewind(f);

    char *p = malloc((size_t)file_size);
    assert(p != NULL);

    fread(p, (size_t)file_size, 1, f);
    if (ferror(f)) {
        perror("Error");
        exit(EXIT_FAILURE);
    }

    return (dy_string_t){
        .ptr = p,
        .size = (size_t)file_size
    };
}

dy_string_t read_stdin_until_eof(void)
{
    char *p = malloc(INITIAL_STDIN_BUFFER_SIZE);
    size_t size = 0;
    size_t rest = INITIAL_STDIN_BUFFER_SIZE;

    for (;;) {
        size_t bytes_read = fread(p + size, 1, rest, stdin);
        if (ferror(stdin)) {
            perror("Error");
            exit(EXIT_FAILURE);
        }

        size += bytes_read;
        rest -= bytes_read;

        if (feof(stdin)) {
            break;
        }

        if (rest == 0) {
            p = realloc(p, size * 2);
            rest = size;
        }
    }

    return (dy_string_t){
        .ptr = p,
        .size = size
    };
}

void *malloc_(size_t size, void *env)
{
    (void)env;

    return malloc(size);
}

void *realloc_(void *ptr, size_t size, void *env)
{
    (void)env;

    return realloc(ptr, size);
}

void free_(void *ptr, size_t size, void *env)
{
    (void)env;
    (void)size;

    free(ptr);
}
