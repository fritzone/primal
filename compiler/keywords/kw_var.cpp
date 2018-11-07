#include "kw_var.h"
#include "kw_var.h"
#include "variable.h"
#include "generate.h"
#include "opcodes.h"
#include "registers.h"
#include "compiler.h"

#include <iostream>

using namespace primal;
using namespace primal::opcodes;

sequence::prepared_type kw_var::prepare(std::vector<token> &tokens)
{
    if(tokens.empty())
    {
        return sequence::prepared_type::PT_INVALID;
    }
    // fetching the name of the token, see if it's a valid identifier or not, this can be a comma separated list

    for(const auto& t : tokens)
    {
        if(t.get_type() != token::type::TT_COMMA)
        {
            std::string name = t.data();
            variable::introduce_name(name);
        }
    }

    return sequence::prepared_type::PT_NORMAL;
}

bool kw_var::compile(compiler* c)
{
    return true;
}
