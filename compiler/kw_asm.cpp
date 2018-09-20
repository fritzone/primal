#include "kw_asm.h"
#include "asm_compiler.h"
#include "generate.h"

sequence::prepared_type kw_asm::prepare(std::vector<token> &tokens)
{
    if(tokens.empty())
    {
        return sequence::prepared_type::PT_INVALID;
    }

    // fetching the opcode
    std::string opcode = tokens[0].data();
    tokens.erase(tokens.begin());

    // see if it's registered or not
    if(opcode_compiler_store::instance().have_opcode(opcode))
    {
        m_precompiled = opcode_compiler_store::instance().opcode_compiler(opcode)(tokens);
    }

    return sequence::prepared_type::PT_PRECOMPILED;
}

bool kw_asm::compile(compiler *c)
{
    for(auto b : m_precompiled)
    {
        compiled_code::instance(c).append(b);
    }
    return true;
}

