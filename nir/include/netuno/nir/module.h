#ifndef NIR_MODULE_H
#define NIR_MODULE_H

#include "netuno/nir/type.h"
#include <netuno/common.h>

NT_HANDLE(NT_STRING)
NT_HANDLE(NIR_FUNCTION)
NT_HANDLE(NIR_MODULE)

/**
 * @brief Creates a new module with a name.
 *
 * @param name The name of new module.
 * @return NIR_MODULE* A new module with that name.
 */
NIR_MODULE *nirCreateModule(const char_t *name);

/**
 * @brief Destroys a module object.
 *
 * @param module The module to destroy.
 */
void nirDestroyModule(NIR_MODULE *module);

/**
 * @brief Gets source file name.
 *
 * @param module The module to get from.
 * @return const NIR_STRING* Pointer to name.
 */
NT_STRING *nirGetSourceFileName(NIR_MODULE *module);

/**
 * @brief Sets module source file name.
 *
 * @param module Module to set source file name.
 * @param sourceFileName New source file name.
 */
void nirSetSourceFileName(NIR_MODULE *module, const char_t *sourceFileName);

/**
 * @brief Sets module name.
 *
 * @param module Module to set name.
 * @param name New module name.
 */
void nirSetModuleIdentifier(NIR_MODULE *module, const char_t *name);

/**
 * @brief Lookup the specified function in the module. If it doesn't exist, add
 * a new function and return it. This function guarantees to return a constant
 * pointer to the specified function type. If function type are wrong, return
 * the function with right type.
 *
 * @param module Module to get function from.
 * @param name Function name.
 * @param type Function type.
 * @return NIR_FUNCTION* The function with that name.
 */
NIR_FUNCTION *nirGetOrInsertFunction(NIR_MODULE *module, const char_t *name,
                                     NIR_TYPE *type);

/**
 * @brief Lookup the specified function in the module symbol table.
 *
 * @param module Module to get function from.
 * @param name Function name.
 * @return NIR_FUNCTION* The function that name, or if doesn't exist, NULL.
 */
NIR_FUNCTION *nirGetFunction(NIR_MODULE *module, const char_t *name);

void nirPrintModule(NIR_MODULE *module);

#endif
