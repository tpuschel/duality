/*
 * Copyright 2017 - 2019, Thorben Hasenpusch
 * Licensed under the MIT license.
 */

#include <duality/syntax/parser.h>
#include <duality/syntax/ast_to_core.h>

#include <duality/core/check.h>
#include <duality/core/eval.h>

#include <duality/support/assert.h>

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

static void print_string(FILE *stream, dy_string_t s);

static void print_unbound_vars_error(dy_array_t *unbound_vars);

static void print_parser_error(dy_string_t text, size_t text_index, struct dy_parser_error_without_index error);

int main(int argc, const char *argv[])
{
    dy_string_t program_text;
    if (argc == 1) {
        program_text = read_stdin_until_eof();
    } else if (argc == 2) {
        program_text = read_file_whole(argv[1]);
    } else {
        fprintf(stderr, "Too many arguments; accepting only up to one.\n");
        return -1;
    }

    struct dy_allocator allocator = {
        .alloc = malloc_,
        .realloc = realloc_,
        .free = free_,
        .env = NULL
    };

    size_t running_id = 0;
    size_t index_out;
    struct dy_parser_error parser_error;

    struct dy_parser_ctx parser_ctx = {
        .text = program_text,
        .index_in = 0,
        .index_out = &index_out,
        .error = &parser_error,
        .allocator = allocator
    };

    struct dy_ast_do_block program_ast;
    if (!dy_parse_file(parser_ctx, &program_ast)) {
        fprintf(stderr, "Failed to parse program.\n");
        print_parser_error(program_text, parser_error.text_index, parser_error.error_without_index);
        return -1;
    }

    dy_array_t *unbound_vars = dy_array_create(allocator, sizeof(dy_string_t), 2);

    struct dy_ast_to_core_ctx ast_to_core_ctx = {
        .running_id = &running_id,
        .bound_vars = dy_array_create(allocator, sizeof(struct dy_ast_to_core_bound_var), 64),
        .allocator = allocator,
        .unbound_vars = unbound_vars
    };

    struct dy_core_expr program;
    if (!dy_ast_do_block_to_core(ast_to_core_ctx, program_ast, &program)) {
        print_unbound_vars_error(unbound_vars);
        return -1;
    }

    printf("Core:\n");
    print_core_expr(program, allocator);
    printf("\n");

    struct dy_check_ctx check_ctx = {
        .running_id = &running_id,
        .allocator = allocator
    };

    struct dy_core_expr checked_program;
    dy_ternary_t is_valid = dy_check_expr(check_ctx, program, &checked_program);

    if (is_valid == DY_NO) {
        fprintf(stderr, "Program failed check.\n");
        return -1;
    }

    struct dy_core_expr result;
    is_valid = dy_eval_expr(check_ctx, checked_program, &result);
    if (is_valid == DY_NO) {
        fprintf(stderr, "Program execution failed.\n");
        return -1;
    }

    printf("Result:\n");
    print_core_expr(result, allocator);
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

void print_parser_error(dy_string_t text, size_t text_index, struct dy_parser_error_without_index error)
{
    switch (error.tag) {
    case DY_PARSER_ERROR_EXPECTED_ANY_CHAR:
        fprintf(stderr, "Unexpected end of file.\n");
        return;
    case DY_PARSER_ERROR_EXPECTED_CHAR:
        if (text_index == text.size) {
            fprintf(stderr, "Unexpected end of file, expected '%c'.\n", error.expected_char);
        } else {
            fprintf(stderr, "Encountered '%c', expected '%c'.\n", text.ptr[text_index], error.expected_char);
        }

        return;
    case DY_PARSER_ERROR_CHOICE: {
        switch (error.choice.tag) {
        case DY_PARSER_ERROR_CHOICE_EXPRESSION:
            if (text_index == text.size) {
                fprintf(stderr, "Unexpected end of file, expected an expression.\n");
            } else {
                fprintf(stderr, "Expected an expression instead of '%c'.\n", text.ptr[text_index]);
            }
            return;
        case DY_PARSER_ERROR_CHOICE_LOWERCASE_LETTERS:
            fprintf(stderr, "Expected a lowercase letter instead of '%c'.\n", text.ptr[text_index]);
            return;
        case DY_PARSER_ERROR_CHOICE_LOWERCASE_LETTERS_DIGITS_UNDERSCORE_AND_QUESTION_MARK:
            fprintf(stderr, "Expected either a lowercase letter, a digit, an underscore or a question mark instead of '%c'.\n", text.ptr[text_index]);
            return;
        case DY_PARSER_ERROR_CHOICE_NO_SUMMARY:
            break;
        }

        size_t num_choices = dy_array_size(error.choice.errors_without_index);
        if (num_choices == 1) {
            print_parser_error(text, text_index, *(struct dy_parser_error_without_index *)dy_array_buffer(error.choice.errors_without_index));
            return;
        }

        printf("num choices: %zu\n", num_choices);

        for (size_t i = 0; i < num_choices; ++i) {
            struct dy_parser_error_without_index err;
            dy_array_get(error.choice.errors_without_index, i, &err);

            print_parser_error(text, text_index, err);
        }

        return;
    }
    }

    DY_IMPOSSIBLE_ENUM();
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
