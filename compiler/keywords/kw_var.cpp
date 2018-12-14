#include "kw_var.h"
#include "kw_var.h"
#include "variable.h"
#include "generate.h"
#include "opcodes.h"
#include "registers.h"
#include "compiler.h"
#include "types.h"

#include <iostream>

using namespace primal;
using namespace primal::opcodes;

sequence::prepared_type kw_var::prepare(std::vector<token> &tokens)
{
    if(tokens.empty())
    {
        return sequence::prepared_type::PT_INVALID;
    }

    entity_type var_types = entity_type::ET_UNKNOWN;
    // first step: see if there is a type identifier following the var: integer (default) or string
    if((var_types = get_entity_type(tokens[0].data())) == entity_type::ET_UNKNOWN)
    {
        var_types = entity_type::ET_NUMERIC;
    }
    else
    {
        // remove the first one, obviously it's a valid variable type
        tokens.erase(tokens.begin());
        if(tokens.empty())
        {
            return sequence::prepared_type::PT_INVALID;
        }
    }

    // fetching the name of the token, see if it's a valid identifier or not, this can be a comma separated list

    for(const auto& t : tokens)
    {
        if(t.get_type() != token::type::TT_COMMA)
        {
            entity_type var_type_2 = get_entity_type(t.data());
            if(var_type_2 == entity_type::ET_UNKNOWN) // still a variable name
            {
                std::string name = t.data();
                variable::introduce_name(name, var_types, entity_origin::EO_PARAMETER);
            }
            else    // switched over to a new type
            {
                var_types = var_type_2;
            }
        }
    }

    return sequence::prepared_type::PT_NORMAL;
}

bool kw_var::compile(compiler*)
{
    return true;
}
