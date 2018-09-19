#ifndef PRIMITIVE_OPCODE_H
#define PRIMITIVE_OPCODE_H

#include <string>
#include <cstdint>
#include <functional>
#include <map>

class vm;

namespace opcodes
{

    struct opcode
    {
        virtual ~opcode() = default;
        virtual uint8_t bin() = 0;
        virtual std::string name() = 0;
    };

}

#endif //PRIMITIVE_OPCODE_H
