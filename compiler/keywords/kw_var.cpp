#include "kw_var.h"
#include "kw_var.h"
#include "variable.h"
#include "generate.h"
#include "opcodes.h"
#include "registers.h"
#include "compiler.h"
#include "types.h"

#include <exceptions.h>
#include <iostream>
#include <options.h>

using namespace primal;
using namespace primal::opcodes;

sequence::prepared_type kw_var::prepare(std::vector<token> &tokens)
{
    if(tokens.empty())
    {
        return sequence::prepared_type::PT_INVALID;
    }

    entity_type current_var_type = entity_type::ET_UNKNOWN;

    if((current_var_type = get_entity_type(tokens[0].data())) == entity_type::ET_UNKNOWN)
    {
        current_var_type = entity_type::ET_NUMERIC;
    }
    else
    {
        tokens.erase(tokens.begin());
        if(tokens.empty())
        {
            return sequence::prepared_type::PT_INVALID;
        }
    }

    for(size_t i = 0; i < tokens.size(); ++i)
    {
        const auto& t = tokens[i];
        if(t.get_type() == token::type::TT_COMMA)
        {
            continue;
        }

        entity_type type_check = get_entity_type(t.data());
        if(type_check != entity_type::ET_UNKNOWN)
        {
            current_var_type = type_check;
        }
        else if (t.get_type() == token::type::TT_IDENTIFIER)
        {
            std::string name = t.data();
            word_t array_size = 1; // Default to scalar

            if (i + 1 < tokens.size() && tokens[i+1].data() == "[") {
                if (i + 3 >= tokens.size() || tokens[i+3].data() != "]") {
                    throw syntax_error("Malformed array declaration for '" + name + "'. Expected ']'.");
                }
                if (tokens[i+2].get_type() != token::type::TT_NUMBER) {
                    throw syntax_error("Array size for '" + name + "' must be a constant integer.");
                }
                array_size = tokens[i+2].to_number();
                if (array_size <= 0) {
                    throw syntax_error("Array size for '" + name + "' must be positive.");
                }
                i += 3; // Consume `[`, `size`, and `]`
            }
            variable::introduce_name(name, current_var_type, entity_origin::EO_VARIABLE, array_size);
        }
    }

    return sequence::prepared_type::PT_NORMAL;
}

bool kw_var::compile(compiler*)
{

    if(options::instance().generate_assembly())
    {
        options::instance().asm_stream() << "===" << m_string_seq << "===" << std::endl;
    }

    return true;
}
