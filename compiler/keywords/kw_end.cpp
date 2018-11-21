#include "kw_end.h"

using namespace primal;

sequence::prepared_type kw_end::prepare(std::vector<token>&)
{
    return sequence::prepared_type::PT_NORMAL;
}

bool kw_end::compile(compiler*)
{
    return true;
}
