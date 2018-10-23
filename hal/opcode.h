#ifndef PRIMITIVE_OPCODE_H
#define PRIMITIVE_OPCODE_H

#include <string>
#include <cstdint>
#include <functional>
#include <map>

namespace primate
{
    namespace opcodes
    {
        struct opcode
        {
            virtual ~opcode() = default;
            virtual uint8_t bin() const = 0;
            virtual std::string name() const = 0;
            virtual size_t paramcount() const = 0;
        };
    }
}

#endif //PRIMITIVE_OPCODE_H
