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
    let cText = allocate(intArrayFromString(program, true), 'i8', ALLOC_NORMAL)
    let cTextLength = lengthBytesUTF8(program)

    let textBuffer = _dy_array_create(1, cTextLength)
    _memcpy(_dy_array_buffer(textBuffer), cText, cTextLength)
    _dy_array_set_size(textBuffer, cTextLength)

    let zero = 0


    // First, parsing
    let parserCtx = _malloc(20)

    setValue(parserCtx, streamCallbackPointer, 'i32') // stream
    setValue(parserCtx + 4, textBuffer, 'i32') // buffer
    setValue(parserCtx + 8, zero, 'i32') // env
    setValue(parserCtx + 12, zero, 'i32') // current_index

    let stringArrays = _dy_array_create(4, 4)
    setValue(parserCtx + 16, stringArrays, 'i32') // arrays

    // actually parse
    let resultDoBlock = _malloc(20)
    let parseDidSucceed = _dy_parse_file(parserCtx, resultDoBlock)
    if (!parseDidSucceed) {
        return 'Parsing failed.'
    }


    // Second, AST -> Core translation
    let astToCoreCtx = _malloc(12)

    // creating running_id with initial value 0
    let runningId = _malloc(4)
    setValue(runningId, 0, 'i32')

    // setting running_id
    setValue(astToCoreCtx, runningId, 'i32')

    // setting bound_vars
    let boundVars = _dy_array_create(16, 8)
    setValue(astToCoreCtx + 4, boundVars, 'i32')

    // setting unbound_vars
    let unboundVars = _dy_array_create(dyStringSize, 4)
    setValue(astToCoreCtx + 8, unboundVars, 'i32')

    let sourceMaps = _dy_array_create(40, 4)

    let coreExprSize = 28

    let coreExprResult = _malloc(coreExprSize)
    let astToCoreDidSucceed = _dy_ast_do_block_to_core(astToCoreCtx, resultDoBlock, coreExprResult, sourceMaps)
    if (!astToCoreDidSucceed) {
        return 'Unbound variables: ' + unboundVarsList(unboundVars).reduce((accum, val) => accum + ', ' + val) + '.'
    }


    // Third, checking
    let checkingCtx = _malloc(8)

    // Setting running_id
    setValue(checkingCtx, runningId, 'i32')

    let boundConstraints = _dy_array_create(36, 8)
    setValue(checkingCtx + 4, boundConstraints, 'i32')

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


    let coreExprStringDyArray = _dy_array_create(1, 64)
    _dy_core_expr_to_string(newCoreExpr, coreExprStringDyArray)

    return 'Result: ' + UTF8ToString(_dy_array_buffer(coreExprStringDyArray), _dy_array_size(coreExprStringDyArray))
}