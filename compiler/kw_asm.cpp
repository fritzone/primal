#include "kw_asm.h"
#include "asm_compiler.h"

bool kw_asm::prepare(std::vector<token> &tokens)
{
    if(tokens.empty())
    {
        return false;
    }

    // fetching the opcode
    std::string opcode = tokens[0].data();
    tokens.erase(tokens.begin());

    // see if it's registered or not
    if(opcode_compiler_store::instance().have_opcode(opcode))
    {
        opcode_compiler_store::instance().opcode_compiler(opcode)(tokens);
    }

    // erasing the equality sign, check for error
    tokens.erase(tokens.begin());
    if(tokens.empty())
    {
        return false;
    }

    return true;
}

bool kw_asm::compile(compiler *c)
{
    return sequence::compile(c);
}

