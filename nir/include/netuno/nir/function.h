#ifndef NIR_FUNCTION_H
#define NIR_FUNCTION_H

#include "netuno/nir/instruction.h"
#include <netuno/common.h>

NT_HANDLE(NIR_FUNCTION)

size_t nirGetParamCount(NIR_FUNCTION *function);
NIR_VALUE *nirGetParamValue(NIR_FUNCTION *function, size_t i);

void nirPrintFunction(NIR_FUNCTION* function);

#endif
