#include "kw_endif.h"

using namespace primate;

sequence::prepared_type kw_endif::prepare(std::vector<token>&)
{
    return sequence::prepared_type::PT_NORMAL;
}

bool kw_endif::compile(compiler*)
{
    return true;
}