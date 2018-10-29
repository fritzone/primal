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
            OF_JUMP = 0,
            OF_ARITH = 1,
            OF_BIN = 2,
            OF_STACK = 3,
            OF_COMP = 4,
            OF_MEM = 5,
            OF_OTHER = 6
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
