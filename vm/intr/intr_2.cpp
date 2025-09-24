#include <vm.h>
#include <numeric_decl.h>
#include <vm_impl.h>
#include <types.h>
#include <util.h>
#include <interface.h>
#include <stringtable.h>

#include <iostream>

namespace primal
{

// Interrupt 2: Foreign Function Interface (FFI) call to C++
//
// Stack layout on entry:
// - Top of stack: Number of arguments
// - Next: Argument N (value)
// - Next: Argument N type
// - ...
// - Next: Argument 1 (value)
// - Next: Argument 1 type
//
// Reg 249 contains the address of the function in the memory, we will need to dig a bit around the function table
//
// Return Value:
// The result of the C++ function call is placed in register r0.
// - For numbers, it's the numeric value.
// - For strings, it's the address of the new string in memory.
// - For void, it's 0.
bool intr_2(vm* v)
{

    std::string func_name;
    word_t r249 = v->r(249).value();
    std::cout << std::dec << "FUN ADDR:" << r249 << std::endl;
    for(size_t i =0 ;i<v->functions().size(); i++)
    {
        std::cout <<" FUN:" << v->functions()[i].name << " @ " << v->functions()[i].address + VM_MEM_SEGMENT_SIZE<< std::endl;

        if(v->functions()[i].address + VM_MEM_SEGMENT_SIZE == r249)
        {
            std::cout << "Found:" << v->functions()[i].name << std::endl;
            func_name = v->functions()[i].name;
            break;
        }

    }

    if(func_name.empty())
    {
        return false;
    }

    // 1. Pop argument count and function name
    word_t arg_count = v->pop();

    // 2. Pop arguments and their types
    script_args scr_args;
    scr_args.reserve(arg_count);

    for (word_t i = 0; i < arg_count; ++i) {
        // Pop value, then its type
        word_t value = v->pop();
        auto type = static_cast<entity_type>(v->pop());

        if (type == entity_type::ET_STRING) {
            // It's an address to a string in VM memory
            uint8_t str_len = v->get_mem_byte(value);
            std::string arg_str;
            arg_str.reserve(str_len);
            for (uint8_t j = 0; j < str_len; ++j) {
                arg_str += static_cast<char>(v->get_mem_byte(value + 1 + j));
            }
            scr_args.push_back(arg_str);
        } else { // ET_NUMERIC
            scr_args.push_back(value);
        }
    }
    // C++ functions expect arguments in forward order
    std::reverse(scr_args.begin(), scr_args.end());

    try {
        // 3. Call the C++ function via the registry
        script_value result = function_registry::instance().call(func_name, scr_args);

        // 3. Handle the return value from the C++ function.
        if (std::holds_alternative<word_t>(result)) {
            // The function returned a number. Place it in the return register.
            v->r(0) = std::get<word_t>(result);
        } else if (std::holds_alternative<std::string>(result)) {
            // The function returned a string. Copy it to a fixed buffer in the VM's memory.
            const auto& str_result = std::get<std::string>(result);
            const word_t string_buffer_address = STRING_RESULT_INDEX_IN_MEM;

            if (str_result.length() > 255) {
                v->panic("FFI string return value is too long (max 255 characters).");
                return false;
            }

            // Write the length prefix.
            v->set_mem_byte(string_buffer_address, static_cast<uint8_t>(str_result.length()));
            // Write the string content character by character.
            for (size_t i = 0; i < str_result.length(); ++i) {
                v->set_mem_byte(string_buffer_address + 1 + i, static_cast<uint8_t>(str_result[i]));
            }
            // Place the address of the string buffer into the return register.
            v->r(0) = string_buffer_address;

        } else { // It's a std::monostate, representing a void return.
            v->r(0) = 0; // Convention for a successful void call.
        }
    } catch (const std::exception& e) {
        v->panic(e.what());
        return false;
    }

    return true;
}

}
