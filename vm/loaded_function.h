#ifndef PRIMAL_FUNCTION_LOADER_H
#define PRIMAL_FUNCTION_LOADER_H

#include <vector>
#include <string>
#include <stdexcept>
#include <cstdint>
#include <cstring>

#include <hal.h>
#include <numeric_decl.h>
#include <types.h>

namespace primal
{
/**
 * @brief A simple, plain structure to hold information about a single function
 *        loaded from the bytecode's symbol table.
 */
struct loaded_function
{
    /** The name of the function as defined in the bytecode. */
    std::string name;

    /** The address of the function in the compiled bytecode. */
    word_t address;

    /** Whether this function refers to an external C++ call. */
    bool is_extern;

    /** The return type of the function, encoded as @ref entity_type. */
    entity_type return_type;

    /** The parameter types of the function, encoded as @ref entity_type. */
    std::vector<entity_type> parameters;
};

/**
 * @brief Loads the function symbol table from a compiled Primal binary.
 *
 * Parses the provided bytecode (.pric file) and extracts all function
 * definitions into @ref loaded_function structures. Each structure contains
 * the function name, its bytecode address, return type, and parameter types.
 *
 * @param bytecode The raw bytes of the compiled Primal program.
 * @return A vector of @ref loaded_function, one entry for each function defined.
 *
 * @throws std::runtime_error If the bytecode symbol table is malformed or cannot be parsed.
 */
std::vector<loaded_function> load_function_table(const std::vector<uint8_t>& bytecode);

} // namespace primal

#endif // PRIMAL_FUNCTION_LOADER_H
