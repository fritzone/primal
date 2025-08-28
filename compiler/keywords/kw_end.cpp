#include "kw_end.h"

#include <options.h>

using namespace primal;

sequence::prepared_type kw_end::prepare(std::vector<token>&)
{
    return sequence::prepared_type::PT_NORMAL;
}

bool kw_end::compile(compiler*)
{

    if(options::instance().generate_assembly())
    {
        options::instance().asm_stream() << "===" << m_string_seq << "===" << std::endl;
    }
    return true;
}
