#include "kw_fun.h"
#include "kw_endf.h"
#include "parser.h"
#include "opcodes.h"
#include "compiler.h"
#include "function.h"

#include <iostream>

using namespace primal;

sequence::prepared_type kw_fun::prepare(std::vector<token> &tokens)
{
    if(tokens.empty())
    {
        return sequence::prepared_type::PT_INVALID;
    }

    // function name should be the first token
    m_function = fun::register_function(tokens[0].data(), m_src);

    m_function->parse();

    return sequence::prepared_type::PT_FUNCTION_DECL;
}

bool kw_fun::compile(compiler* c)
{
    return m_function->compile(c);
}
