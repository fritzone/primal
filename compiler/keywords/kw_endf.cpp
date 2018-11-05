#include "kw_endf.h"

using namespace primal;

sequence::prepared_type kw_endf::prepare(std::vector<token>&)
{
    return sequence::prepared_type::PT_NORMAL;
}

bool kw_endf::compile(compiler*)
{
    return true;
}
