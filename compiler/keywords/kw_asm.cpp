#include "kw_asm.h"
#include "asm_compiler.h"
#include "generate.h"
#include "options.h"
#include "hal.h"

#include <exceptions.h>
#include <iomanip>

using namespace primal;

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
    if(!opcode_compiler_store::instance().have_opcode(opcode))
    {
        throw syntax_error("Invalid opcode: " + opcode);
    }

    m_opcode = opcode;
    m_tokens = tokens;

    return sequence::prepared_type::PT_PRECOMPILED;
}

bool kw_asm::compile(compiler *c)
{

    if(options::instance().generate_assembly())
    {
        options::instance().asm_stream() << "===" << m_string_seq << "===" << std::endl;
    }

    if(options::instance().generate_assembly())
    {
        options::instance().asm_stream() << std::setfill(' ') << std::right << std::setw(5) << std::dec << compiled_code::instance(c).location() + PRIMAL_HEADER_SIZE << ": ";
    }
    m_precompiled = opcode_compiler_store::instance().opcode_compiler(m_opcode)(m_tokens,c);

    for(auto b : m_precompiled)
    {
        compiled_code::instance(c).append(b);
    }
    return true;
}

