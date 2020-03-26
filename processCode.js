'use strict'

let dyStringSize = 8

let unboundVarsList = unboundVars => {
    let len = _dy_array_size(unboundVars)

    let list = []

    for (let i = 0; i < len; ++i) {
        let varName = _malloc(dyStringSize)

        _dy_array_get(unboundVars, i, varName)

        let varNameJs = UTF8ToString(getValue(varName, 'i32'), getValue(varName + 4, 'i32'))

        list.push(varNameJs)

        _free(varName)
    }

    return list
}

let streamCallbackPointer = addFunction((buffer, env) => { }, 'vii')

let processCode = program => {
    let allocatorPtr = _malloc(16)
    _dy_allocator_stdlib(allocatorPtr)

    let cText = allocate(intArrayFromString(program, true), 'i8', ALLOC_NORMAL)
    let cTextLength = lengthBytesUTF8(program)

    let textBuffer = _dy_array_create(allocatorPtr, 1, cTextLength)
    _memcpy(_dy_array_buffer(textBuffer), cText, cTextLength)
    _dy_array_set_size(textBuffer, cTextLength)

    let zero = 0


    // First, parsing
    let parserCtx = _malloc(36)

    setValue(parserCtx, streamCallbackPointer, 'i32') // stream
    setValue(parserCtx + 4, textBuffer, 'i32') // buffer
    setValue(parserCtx + 8, zero, 'i32') // env
    setValue(parserCtx + 12, zero, 'i32') // current_index

    // setting allocator
    setValue(parserCtx + 16, getValue(allocatorPtr, 'i32'), 'i32')
    setValue(parserCtx + 20, getValue(allocatorPtr + 4, 'i32'), 'i32')
    setValue(parserCtx + 24, getValue(allocatorPtr + 8, 'i32'), 'i32')
    setValue(parserCtx + 28, getValue(allocatorPtr + 12, 'i32'), 'i32')

    let stringArrays = _dy_array_create(allocatorPtr, 4, 4)
    setValue(parserCtx + 32, stringArrays, 'i32') // arrays

    // actually parse
    let resultDoBlock = _malloc(20)
    let parseDidSucceed = _dy_parse_file(parserCtx, resultDoBlock)
    if (!parseDidSucceed) {
        return 'Parsing failed.'
    }


    // Second, AST -> Core translation
    let astToCoreCtx = _malloc(28)

    // creating running_id with initial value 0
    let runningId = _malloc(4)
    setValue(runningId, 0, 'i32')

    // setting running_id
    setValue(astToCoreCtx, runningId, 'i32')

    // setting allocator
    setValue(astToCoreCtx + 4, getValue(allocatorPtr, 'i32'), 'i32')
    setValue(astToCoreCtx + 8, getValue(allocatorPtr + 4, 'i32'), 'i32')
    setValue(astToCoreCtx + 12, getValue(allocatorPtr + 8, 'i32'), 'i32')
    setValue(astToCoreCtx + 16, getValue(allocatorPtr + 12, 'i32'), 'i32')

    // setting bound_vars
    let boundVars = _dy_array_create(allocatorPtr, 16, 8)
    setValue(astToCoreCtx + 20, boundVars, 'i32')

    // setting unbound_vars
    let unboundVars = _dy_array_create(allocatorPtr, dyStringSize, 4)
    setValue(astToCoreCtx + 24, unboundVars, 'i32')

    let sourceMaps = _dy_array_create(allocatorPtr, 44, 4)

    let coreExprSize = 32

    let coreExprResult = _malloc(coreExprSize)
    let astToCoreDidSucceed = _dy_ast_do_block_to_core(astToCoreCtx, resultDoBlock, coreExprResult, sourceMaps)
    if (!astToCoreDidSucceed) {
        return 'Unbound variables: ' + unboundVarsList(unboundVars).reduce((accum, val) => accum + ', ' + val) + '.'
    }


    // Third, checking
    let checkingCtx = _malloc(24)

    // Setting running_id
    setValue(checkingCtx, runningId, 'i32')

    // setting allocator
    setValue(checkingCtx + 4, getValue(allocatorPtr, 'i32'), 'i32')
    setValue(checkingCtx + 8, getValue(allocatorPtr + 4, 'i32'), 'i32')
    setValue(checkingCtx + 12, getValue(allocatorPtr + 8, 'i32'), 'i32')
    setValue(checkingCtx + 16, getValue(allocatorPtr + 12, 'i32'), 'i32')

    let boundConstraints = _dy_array_create(allocatorPtr, 40, 8)
    setValue(checkingCtx + 20, boundConstraints, 'i32')

    let constraint = _malloc(16)
    let have_constraint = _malloc(4)

    let checkDidSucceed = _dy_check_expr(checkingCtx, coreExprResult, coreExprResult, constraint, have_constraint)
    if (!checkDidSucceed) {
        return 'Failed check.'
    }


    // Lastly, evaluating
    let newCoreExpr = _malloc(coreExprSize)
    let evalDidSucceed = _dy_eval_expr(checkingCtx, coreExprResult, newCoreExpr)
    if (evalDidSucceed == 1) {
        // Fail
        return 'Evaluation failed.'
    }


    let coreExprStringDyArray = _dy_array_create(allocatorPtr, 1, 64)
    _dy_core_expr_to_string(newCoreExpr, coreExprStringDyArray)

    return 'Result: ' + UTF8ToString(_dy_array_buffer(coreExprStringDyArray), _dy_array_size(coreExprStringDyArray))
}