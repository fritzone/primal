#include "loaded_function.h"
#include <util.h>

#include <iostream>

/*
 * These two functions duplicate two from util, but we'll keep them here to have a delivery as compact as possible
 */
template<typename T>
T read_value(const std::vector<uint8_t>& bytecode, word_t& offset)
{
    if (offset + sizeof(T) > bytecode.size())
    {
        throw std::runtime_error("Unexpected end of file while reading value.");
    }
    T value;
    memcpy(&value, &bytecode[offset], sizeof(T));
    offset += sizeof(T);
    return value;
}

static std::string read_lp_string(const std::vector<uint8_t> &bytecode, word_t &offset)
{
    uint8_t len = read_value<uint8_t>(bytecode, offset);
    if (offset + len > static_cast<word_t>(bytecode.size())) {
        throw std::runtime_error("Unexpected end of file while reading string.");
    }
    std::string value(reinterpret_cast<const char*>(&bytecode[offset]), len);
    offset += len;
    return value;
}

std::vector<primal::loaded_function> primal::load_function_table(const std::vector<uint8_t> &bytecode) {
    std::vector<loaded_function> table;

    if (bytecode.size() < PRIMAL_HEADER_SIZE) {
        throw std::runtime_error("Bytecode is too small to contain a valid header.");
    }

    word_t header_offset = 0;
    // Read offsets from header to find the function table
    header_offset += 4; // Skip to function table offset, jump over .P10
    word_t func_table_offset = htovm(util::read_value<word_t>(bytecode, header_offset));
    std::cout << "--- Function Symbol Table  @ " << func_table_offset << std::endl;
    word_t funcs_offs = func_table_offset + 4;
    word_t func_table_count = htovm(util::read_value<word_t>(bytecode, funcs_offs));


    if (func_table_offset == 0 || func_table_count == 0) {
        return table; // No functions in this binary.
    }

    // 2. Iterate through the function table and load each entry ---
    word_t current_offset = funcs_offs;
    table.reserve(static_cast<size_t>(func_table_count));

    for (word_t i = 0; i < func_table_count; ++i) {
        loaded_function func_info;

        // 1. Name (length-prefixed)
        func_info.name = read_lp_string(bytecode, current_offset);

        // 2. Address
        func_info.address = htovm(util::read_value<word_t>(bytecode, current_offset));

        // 3. Is Extern flag
        func_info.is_extern = util::read_value<uint8_t>(bytecode, current_offset);

        // 4. Return Type
        func_info.return_type = static_cast<entity_type>(util::read_value<uint8_t>(bytecode, current_offset));

        // 5. Parameters
        uint8_t param_count = util::read_value<uint8_t>(bytecode, current_offset);
        if (param_count > 0) {
            func_info.parameters.reserve(param_count);
            for (uint8_t p = 0; p < param_count; ++p) {
                auto param_type = static_cast<entity_type>(util::read_value<uint8_t>(bytecode, current_offset));
                func_info.parameters.push_back(param_type);
            }
        }
        table.push_back(std::move(func_info));
    }

    return table;
}
