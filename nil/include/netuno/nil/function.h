#ifndef NIL_FUNCTION_H
#define NIL_FUNCTION_H

#include <netuno/array.h>
#include <netuno/nil/instruction.h>
#include <netuno/common.h>

NT_HANDLE(NIL_FUNCTION)

size_t nilGetParamCount(NIL_FUNCTION *function);
NIL_VALUE *nilGetParamValue(NIL_FUNCTION *function, size_t i);

bool nilSerializeFunction(NIL_FUNCTION *function, NT_ARRAY *array);
void nilPrintFunction(NIL_FUNCTION *function);

#endif
