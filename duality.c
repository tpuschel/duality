/*
 * Copyright 2017-2021 Thorben Hasenpusch <t.hasenpusch@icloud.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "syntax/utf8_to_ast.h"
#include "syntax/ast_to_core.h"

#include "core/check.h"
#include "core/eval.h"

#include "lsp/server.h"

static void read_chunk(dy_array_t *buffer, void *env);

static const size_t CHUNK_SIZE = 1024;

static void print_core_expr(struct dy_core_ctx *ctx, FILE *file, struct dy_core_expr expr);

static bool core_has_error(struct dy_core_expr expr);

static bool print_core_errors(dy_array_t text_sources, FILE *file, struct dy_core_expr expr, const char *text, size_t text_size);

static void print_error_fragment(FILE *file, struct dy_range range, const char *text, size_t text_size);

int main(int argc, const char *argv[])
{
    FILE *stream;
    if (argc > 1) {
        if (strcmp(argv[1], "--server") == 0) {
            return dy_lsp_run_server(stdin, stdout);
        }

        if (strcmp(argv[1], "--debugger") == 0) {
            fprintf(stderr, "DAP not yet implemented!\n");
            return -1;
        }

        stream = fopen(argv[1], "r");
        if (stream == NULL) {
            perror("Error reading file");
            return -1;
        }
    } else {
        stream = stdin;
    }

    struct dy_utf8_to_ast_ctx utf8_to_ast_ctx = {
        .stream = {
            .get_chars = read_chunk,
            .buffer = dy_array_create(sizeof(char), DY_ALIGNOF(char), CHUNK_SIZE),
            .env = stream,
            .current_index = 0
        }
    };

    struct dy_ast_do_block ast;
    if (!dy_utf8_to_ast_file(&utf8_to_ast_ctx, &ast)) {
        fprintf(stderr, "Failed to parse program.\n");
        return -1;
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
        .recovered_positive_inference_ids = dy_array_create(sizeof(size_t), DY_ALIGNOF(size_t), 8),
        .past_subtype_checks = dy_array_create(sizeof(struct dy_core_past_subtype_check), DY_ALIGNOF(struct dy_core_past_subtype_check), 64),
        .equal_variables = dy_array_create(sizeof(struct dy_equal_variables), DY_ALIGNOF(struct dy_equal_variables), 64),
        .free_ids_arrays = dy_array_create(sizeof(dy_array_t), DY_ALIGNOF(dy_array_t), 8),
        .constraints = dy_array_create(sizeof(struct dy_constraint), DY_ALIGNOF(struct dy_constraint), 64),
        .custom_shared = custom_shared
    };

    printf("=== Pre-checked Core ====\n\n");
    print_core_expr(&core_ctx, stdout, core);
    printf("\n\n");

    struct dy_core_expr checked_core;
    if (dy_check_expr(&core_ctx, core, &checked_core)) {
        dy_core_expr_release(&core_ctx, core);
        core = checked_core;
    }

    printf("=== Checked Core ====\n\n");
    print_core_expr(&core_ctx, stdout, core);
    printf("\n\n");

    if (core_has_error(core)) {
        fprintf(stderr, "*** Encountered errors. Aborting. ***\n");
        return -1;
    }

    bool is_value = false;
    struct dy_core_expr result = core;
    dy_eval_expr(&core_ctx, core, &is_value, &result);

    printf("=== Evaluated Core ====\n\n");
    print_core_expr(&core_ctx, stdout, result);
    printf("\n");

    if (!is_value) {
        if (core_has_error(result)) {
            fprintf(stderr, "*** Encountered errors. Aborting. ***\n");
        } else {
            fprintf(stderr, "*** Unable to continue evaluating. ***\n");
        }

        return -1;
    }

    return 0;
}

void print_core_expr(struct dy_core_ctx *ctx, FILE *file, struct dy_core_expr expr)
{
    dy_array_t s = dy_array_create(sizeof(char), DY_ALIGNOF(char), 64);
    dy_core_expr_to_string(ctx, expr, &s);

    for (size_t i = 0; i < s.num_elems; ++i) {
        char c = *(char *)dy_array_pos(&s, i);
        fprintf(file, "%c", c);
    }

    dy_array_release(&s);
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

bool core_has_error(struct dy_core_expr expr)
{
    switch (expr.tag) {
    case DY_CORE_EXPR_INTRO:
        switch (expr.intro.tag) {
        case DY_CORE_INTRO_COMPLEX:
            switch (expr.intro.complex.tag) {
            case DY_CORE_COMPLEX_ASSUMPTION:
                return core_has_error(*expr.intro.complex.assumption.type)
                    || core_has_error(*expr.intro.complex.assumption.expr);
            case DY_CORE_COMPLEX_CHOICE:
                return core_has_error(*expr.intro.complex.choice.left)
                    || core_has_error(*expr.intro.complex.choice.right);
            case DY_CORE_COMPLEX_RECURSION:
                return core_has_error(*expr.intro.complex.recursion.expr);
            }

            dy_bail("impossible");
        case DY_CORE_INTRO_SIMPLE:
            return (expr.intro.simple.tag == DY_CORE_SIMPLE_PROOF && core_has_error(*expr.intro.simple.proof))
                || core_has_error(*expr.intro.simple.out);
        }

        dy_bail("impossible");
    case DY_CORE_EXPR_ELIM:
        return expr.elim.check_result == DY_NO
            || core_has_error(*expr.elim.expr)
            || (expr.elim.simple.tag == DY_CORE_SIMPLE_PROOF && core_has_error(*expr.elim.simple.proof))
            || core_has_error(*expr.elim.simple.out);
    case DY_CORE_EXPR_VARIABLE:
    case DY_CORE_EXPR_ANY:
    case DY_CORE_EXPR_VOID:
        return false;
    case DY_CORE_EXPR_INFERENCE_CTX:
        return core_has_error(*expr.inference_ctx.expr);
    case DY_CORE_EXPR_INFERENCE_VAR:
        return false;
    case DY_CORE_EXPR_MAP:
        switch (expr.map.tag) {
        case DY_CORE_MAP_ASSUMPTION:
            return expr.map.assumption.dependence == DY_CORE_MAP_DEPENDENCE_DEPENDENT
                || core_has_error(*expr.map.assumption.type)
                || core_has_error(*expr.map.assumption.assumption.type)
                || core_has_error(*expr.map.assumption.assumption.expr);
        case DY_CORE_MAP_CHOICE:
            return expr.map.choice.left_dependence == DY_CORE_MAP_DEPENDENCE_DEPENDENT
                || expr.map.choice.right_dependence == DY_CORE_MAP_DEPENDENCE_DEPENDENT
                || core_has_error(*expr.map.choice.assumption_left.type)
                || core_has_error(*expr.map.choice.assumption_left.expr)
                || core_has_error(*expr.map.choice.assumption_right.type)
                || core_has_error(*expr.map.choice.assumption_right.expr);
        case DY_CORE_MAP_RECURSION:
            return expr.map.recursion.dependence == DY_CORE_MAP_DEPENDENCE_DEPENDENT
                || core_has_error(*expr.map.recursion.assumption.type)
                || core_has_error(*expr.map.recursion.assumption.expr);
        }

        dy_bail("impossible");
    case DY_CORE_EXPR_CUSTOM:
        if (expr.custom.id == dy_uv_id) {
            return true;
        }

        // XXX: Add custom hook for error detection & reporting.

        return false;
    }

    dy_bail("impossible");
}
/*
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
    case DY_CORE_EXPR_PART_ELIM: {
        bool err1 = print_core_errors(text_sources, file, *expr.part_elim.expr, text, text_size);
        bool err2 = print_core_errors(text_sources, file, *expr.part_elim.type, text, text_size);

        return err1 && err2;
    }
    case DY_CORE_EXPR_PART:
        return print_core_errors(text_sources, file, *expr.part.expr, text, text_size);
    case DY_CORE_EXPR_JUNCTION:
        return print_core_errors(text_sources, file, *expr.junction.e1, text, text_size)
        && print_core_errors(text_sources, file, *expr.junction.e2, text, text_size);
    case DY_CORE_EXPR_ALTERNATIVE:
        return print_core_errors(text_sources, file, *expr.alternative.first, text, text_size)
        && print_core_errors(text_sources, file, *expr.alternative.second, text, text_size);
    case DY_CORE_EXPR_VARIABLE:
        return true;
    case DY_CORE_EXPR_INFERENCE_CTX:
        dy_bail("should never be reached");
    case DY_CORE_EXPR_RECURSION:
        return print_core_errors(text_sources, file, *expr.recursion.type, text, text_size)
        && print_core_errors(text_sources, file, *expr.recursion.expr, text, text_size);
    case DY_CORE_EXPR_END:
        return true;
    case DY_CORE_EXPR_CUSTOM:
        if (expr.custom.id == dy_uv_id) {
            //struct dy_uv_data *data = expr.custom.data;

            fprintf(file, "Unbound variable\n");
            //print_error_fragment(file, data->var.text_range, text, text_size);
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
*/
