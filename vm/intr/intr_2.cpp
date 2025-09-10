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
// - Next: Function name (as an address into the string table)
// - Next: Argument N (value)
// - Next: Argument N type (if variadic)
// - ...
// - Next: Argument 1 (value)
// - Next: Argument 1 type (if variadic)
//
// Return Value:
// The result of the C++ function call is placed in register r0.
// - For numbers, it's the numeric value.
// - For strings, it's the address of the new string in memory.
// - For void, it's 0.
bool intr_2(vm* v)
{
    // 1. Pop argument count and function name
    word_t arg_count = v->pop();
    word_t func_name_addr = v->pop();

    // The address is in the VM's memory space, so we need to read the string from there
    uint8_t len = v->get_mem_byte(func_name_addr);
    std::string func_name;
    func_name.reserve(len);
    for (uint8_t i = 0; i < len; ++i) {
        func_name += static_cast<char>(v->get_mem_byte(func_name_addr + 1 + i));
    }


    // 2. Pop arguments and their types
    ScriptArgs script_args;
    script_args.reserve(arg_count);

    for (word_t i = 0; i < arg_count; ++i) {
        // Pop type and then value
        auto type = static_cast<entity_type>(v->pop());
        word_t value = v->pop();

        if (type == entity_type::ET_STRING) {
            // It's an address to a string in VM memory
            uint8_t str_len = v->get_mem_byte(value);
            std::string arg_str;
            arg_str.reserve(str_len);
            for (uint8_t j = 0; j < str_len; ++j) {
                arg_str += static_cast<char>(v->get_mem_byte(value + 1 + j));
            }
            script_args.push_back(arg_str);
        } else { // ET_NUMERIC
            script_args.push_back(value);
        }
    }
    // C++ functions expect arguments in forward order
    std::reverse(script_args.begin(), script_args.end());

    try {
        // 3. Call the C++ function via the registry
        ScriptValue result = FunctionRegistry::instance().call(func_name, script_args);

        // 4. Handle the return value
        if (std::holds_alternative<word_t>(result)) {
            v->r(0) = std::get<word_t>(result);
        } else if (std::holds_alternative<std::string>(result)) {
            // If the C++ function returns a string, we need to add it to the VM's
            // string table and put its address in r0.
            const auto& str_result = std::get<std::string>(result);
            //word_t str_idx = stringtable::instance().add(str_result);
            
            // This is a bit of a hack: we need to find where the string table will be
            // in memory. For now, we use a large, fixed offset.
            //word_t str_addr = stringtable::instance().e(str_idx).in_mem_location + STRING_TABLE_INDEX_IN_MEM;
            //v->r(0) = str_addr;
        } else { // monostate (void)
            v->r(0) = 0;
        }
    } catch (const std::exception& e) {
        v->panic(e.what());
        return false;
    }

    return true;
}

}
