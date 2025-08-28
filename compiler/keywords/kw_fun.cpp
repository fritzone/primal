#include "kw_fun.h"
#include "kw_end.h"
#include "parser.h"
#include "opcodes.h"
#include "compiler.h"
#include "function.h"
#include "variable.h"

#include <iostream>
#include <options.h>

using namespace primal;

sequence::prepared_type kw_fun::prepare(std::vector<token> &tokens)
{
    if(tokens.empty())
    {
        return sequence::prepared_type::PT_INVALID;
    }

    // function name should be the first token
    m_function = fun::register_function(tokens[0].data(), m_src);

    variable::enter_function(tokens[0].data());

    tokens.erase(tokens.begin());

    if(!tokens.empty())
    {
        // then the parameters
        m_function->identify_parameters(tokens);
    }

    m_function->parse();

    variable::leave_function();

    return sequence::prepared_type::PT_FUNCTION_DECL;
}

bool kw_fun::compile(compiler* c)
{

    if(options::instance().generate_assembly())
    {
        options::instance().asm_stream() << "===" << m_string_seq << "===" << std::endl;
    }

    return m_function->compile(c);
}
