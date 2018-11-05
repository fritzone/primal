#ifndef PRIMITIVE_OPCODE_H
#define PRIMITIVE_OPCODE_H

#include <string>
#include <cstdint>
#include <functional>
#include <map>

namespace primal
{
    namespace opcodes
    {
        /* The class which this opcode takes part from */
        enum class opcode_family : uint8_t
        {
            OF_JUMP = 0,            // jump family
            OF_ARITH = 1,           // arithmetic family
            OF_BIN = 2,             // binary operations
            OF_STACK = 3,           // stack manipulation
            OF_COMP = 4,            // comparison
            OF_MEM = 5,             // memory operations
            OF_CONTROLFLOW = 6,     // control flow alteration which is not jump
            OF_OTHER = 7            // other unspecified
        };

        struct opcode
        {
            virtual ~opcode() = default;
            virtual uint8_t bin() const = 0;
            virtual std::string name() const = 0;
            virtual size_t paramcount() const = 0;
            virtual opcode_family family() const = 0;
        };
    }
}

#endif //PRIMITIVE_OPCODE_H
