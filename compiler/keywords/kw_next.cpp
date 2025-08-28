#include "kw_next.h"

#include <options.h>

using namespace primal;

sequence::prepared_type kw_next::prepare(std::vector<token>&) {
    // The 'NEXT' keyword doesn't need to parse anything further.
    // It just acts as a delimiter for the FOR loop body.
    return sequence::prepared_type::PT_CONSUMED;
}

bool kw_next::compile(compiler*) {


    if(options::instance().generate_assembly())
    {
        options::instance().asm_stream() << "===" << m_string_seq << "===" << std::endl;
    }

    // The compilation logic is entirely handled by kw_for.
    // This function does nothing.
    return true;
}
