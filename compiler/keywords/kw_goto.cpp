#include "kw_goto.h"
#include "opcodes.h"
#include "generate.h"

#include <compiler.h>
#include <options.h>
#include <util.h>

using namespace primal;
using namespace primal::opcodes;

sequence::prepared_type kw_goto::prepare(std::vector<token>& tokens)
{
    if(tokens.size() != 1)
    {
        return sequence::prepared_type::PT_INVALID;
    }
    m_label.set_name(tokens[0].data());

    return sequence::prepared_type::PT_CONSUMED;
}

bool kw_goto::compile(compiler* c)
{

    if(options::instance().generate_assembly())
    {
        options::instance().asm_stream() << "===" << m_string_seq << "===" << std::endl;
    }

    (*c->generator()) << DJMP() << m_label;

    return true;
}
