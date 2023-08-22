#ifndef NIL_MODULE_H
#define NIL_MODULE_H

#include <netuno/array.h>
#include <netuno/nil/type.h>
#include <netuno/common.h>

NT_HANDLE(NT_STRING)
NT_HANDLE(NIL_FUNCTION)
NT_HANDLE(NIL_MODULE)

/**
 * @brief Creates a new module with a name.
 *
 * @param name The name of new module.
 * @return NIL_MODULE* A new module with that name.
 */
NIL_MODULE *nilCreateModule(const char_t *name);

/**
 * @brief Destroys a module object.
 *
 * @param module The module to destroy.
 */
void nilDestroyModule(NIL_MODULE *module);

/**
 * @brief Gets source file name.
 *
 * @param module The module to get from.
 * @return const NIL_STRING* Pointer to name.
 */
NT_STRING *nilGetSourceFileName(NIL_MODULE *module);

/**
 * @brief Sets module source file name.
 *
 * @param module Module to set source file name.
 * @param sourceFileName New source file name.
 */
void nilSetSourceFileName(NIL_MODULE *module, const char_t *sourceFileName);

/**
 * @brief Sets module name.
 *
 * @param module Module to set name.
 * @param name New module name.
 */
void nilSetModuleIdentifier(NIL_MODULE *module, const char_t *name);

/**
 * @brief Lookup the specified function in the module. If it doesn't exist, add
 * a new function and return it. This function guarantees to return a constant
 * pointer to the specified function type. If function type are wrong, return
 * the function with right type.
 *
 * @param module Module to get function from.
 * @param name Function name.
 * @param type Function type.
 * @return NIL_FUNCTION* The function with that name.
 */
NIL_FUNCTION *nilGetOrInsertFunction(NIL_MODULE *module, const char_t *name,
                                     NIL_TYPE *type);

/**
 * @brief Lookup the specified function in the module symbol table.
 *
 * @param module Module to get function from.
 * @param name Function name.
 * @return NIL_FUNCTION* The function that name, or if doesn't exist, NULL.
 */
NIL_FUNCTION *nilGetFunction(NIL_MODULE *module, const char_t *name);

bool nilSerializeModule(NIL_MODULE *module, NT_ARRAY *array);
void nilPrintModule(NIL_MODULE *module);

#endif
