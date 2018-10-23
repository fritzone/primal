#ifndef PRIMITIVE_ASM_COMPILER_H
#define PRIMITIVE_ASM_COMPILER_H

#include "token.h"

#include <string>
#include <functional>
#include <map>
#include <vector>
#include <set>

namespace primate
{
    class opcode_compiler_store
    {
    public:

        static opcode_compiler_store& instance()
        {
            static opcode_compiler_store i;
            return i;
        }

        template<class T>
        void register_opcode(T&& v, std::function<std::vector<uint8_t>(std::vector<token>&)> cmplr)
        {
            instance().register_opcode_by_name(v.name(), cmplr);
        }

        bool have_opcode(const std::string& opc)
        {
            return opcode_compilers.count(opc) > 0;
        }

        std::function<std::vector<uint8_t>(std::vector<token>&)>& opcode_compiler(const std::string& opc)
        {
            if(!have_opcode(opc))
            {
                throw "opcode not registered for compilation: " + opc;
            }
            return opcode_compilers[opc];
        }

    private:

        void register_opcode_by_name(const std::string& name, const std::function<std::vector<uint8_t>(std::vector<token>&)>& cmplr)
        {
            opcode_compilers[name] = cmplr;
        }


        std::map<std::string, std::function<std::vector<uint8_t>(std::vector<token>&)>> opcode_compilers;
    };

/**
 * Class representing an assembly compiler. Will take in a vector of tokens and will produce a vector of binary sumbols
 **/
    class asm_compiler
    {
    public:

        // will generate the assebmly code for the given token holding assembly instructions
        static void generate_assembly_code(const std::vector<token>& tokens, std::vector<uint8_t>& result);
    };

}


#endif //PRIMITIVE_ASM_COMPILER_H
