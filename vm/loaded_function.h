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
// A simple, plain structure to hold information about a single function
// loaded from the bytecode's symbol table.
struct loaded_function {
    std::string name;
    word_t address;
    bool is_extern;
    entity_type return_type;
    std::vector<entity_type> parameters;
};

/**
 * @brief Loads the function symbol table from a compiled Primal binary.
 *
 * @param bytecode The raw bytes of the .pric file.
 * @return A vector of loaded_function structs, one for each function defined.
 */
std::vector<loaded_function> load_function_table(const std::vector<uint8_t>& bytecode);
} // namespace primal

#endif // PRIMAL_FUNCTION_LOADER_H
