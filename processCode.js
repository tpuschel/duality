'use strict'

const processCode = program => {
    const parserCtxSize = 36
    const astDoBlockSize = 12
    const allocatorSize = 16
    const astToCoreCtxSize = 24
    const boundVarsSize = 16
    const coreExprSize = 22
    const checkCtxSize = 20

    const cText = allocate(intArrayFromString(program, true), 'i8', ALLOC_NORMAL)
    const cTextLength = lengthBytesUTF8(program)

    // First, parsing
    const parserCtx = _malloc(parserCtxSize)

    // setting text
    setValue(parserCtx, cText, 'i32')
    setValue(parserCtx + 4, cTextLength, 'i32')

    // setting index_in
    setValue(parserCtx + 8, 0, 'i32')

    // setting allocator
    const allocatorPtr = _malloc(allocatorSize)
    _dy_allocator_stdlib(allocatorPtr)
    setValue(parserCtx + 12, getValue(allocatorPtr, 'i32'), 'i32')
    setValue(parserCtx + 16, getValue(allocatorPtr + 4, 'i32'), 'i32')
    setValue(parserCtx + 20, getValue(allocatorPtr + 8, 'i32'), 'i32')
    setValue(parserCtx + 24, getValue(allocatorPtr + 12, 'i32'), 'i32')

    // setting index_out
    const indexOutPtr = _malloc(4)
    setValue(parserCtx + 28, indexOutPtr, 'i32')

    // setting errors
    const errors = _dy_array_create(parserCtx + 12, 8, 1)
    setValue(parserCtx + 32, errors, 'i32')

    // actually parse
    const resultDoBlock = _malloc(astDoBlockSize)
    const parseDidSucceed = _dy_parse_file(parserCtx, resultDoBlock)
    if (!parseDidSucceed) {
        return 'Parsing failed.'
    }


    // Second, AST -> Core translation
    const astToCoreCtx = _malloc(astToCoreCtxSize)

    // setting running_id
    const runningId = _malloc(4)
    setValue(astToCoreCtx, runningId, 'i32')

    // setting allocator
    setValue(astToCoreCtx + 4, getValue(allocatorPtr, 'i32'), 'i32')
    setValue(astToCoreCtx + 8, getValue(allocatorPtr + 4, 'i32'), 'i32')
    setValue(astToCoreCtx + 12, getValue(allocatorPtr + 8, 'i32'), 'i32')
    setValue(astToCoreCtx + 16, getValue(allocatorPtr + 12, 'i32'), 'i32')

    // setting bound_vars
    const boundVars = _dy_array_create(parserCtx + 12, boundVarsSize, 8)
    setValue(astToCoreCtx + 20, boundVars, 'i32')

    const coreExprResult = _malloc(coreExprSize)
    const astToCoreDidSucceed = _dy_ast_do_block_to_core(astToCoreCtx, resultDoBlock, coreExprResult)
    if (!astToCoreDidSucceed) {
        return 'AST to Core translation failed.'
    }


    // Third, checking
    const checkingCtx = _malloc(checkCtxSize)

    // Setting running_id
    setValue(checkingCtx, runningId, 'i32')

    // setting allocator
    setValue(checkingCtx + 4, getValue(allocatorPtr, 'i32'), 'i32')
    setValue(checkingCtx + 8, getValue(allocatorPtr + 4, 'i32'), 'i32')
    setValue(checkingCtx + 12, getValue(allocatorPtr + 8, 'i32'), 'i32')
    setValue(checkingCtx + 16, getValue(allocatorPtr + 12, 'i32'), 'i32')

    const checkDidSucceed = _dy_check_expr(checkingCtx, coreExprResult)
    if (checkDidSucceed == 1) {
        // Fail
        return 'Failed check.'
    }


    // Lastly, evaluating
    const newCoreExpr = _malloc(coreExprSize)
    const evalDidSucceed = _dy_eval_expr(checkingCtx, coreExprResult, newCoreExpr)
    if (evalDidSucceed == 1) {
        // Fail
        return 'Evaluation failed.'
    }

    const coreExprStringDyArray = _dy_array_create(parserCtx + 12, 1, 64)
    _dy_core_expr_to_string(newCoreExpr, coreExprStringDyArray)

    return 'Result: ' + UTF8ToString(_dy_array_buffer(coreExprStringDyArray), _dy_array_size(coreExprStringDyArray))
}