#include "kw_let.h"
#include "variable.h"
#include "generate.h"
#include "opcodes.h"
#include "registers.h"
#include "compiler.h"
#include "exceptions.h"

using namespace primal;
using namespace primal::opcodes;

sequence::prepared_type kw_let::prepare(std::vector<token> &tokens)
{
    if(tokens.empty())
    {
        return sequence::prepared_type::PT_INVALID;
    }
    // fetching the name of the token, see if it's a valid identifier or not
    m_name = tokens[0].data();
    tokens.erase(tokens.begin());
    if(!variable::has_variable(m_name))
    {
        throw primal::syntax_error("Not found a variable declaration: " + m_name);
    }

    // erasing the equality sign, check for error
    tokens.erase(tokens.begin());
    if(tokens.empty())
    {
        return sequence::prepared_type::PT_INVALID;
    }

    return sequence::prepared_type::PT_NORMAL;
}

bool kw_let::compile(compiler* c)
{
    // now add the variable name into the variables
    if (!c->has_variable(m_name))
    {
        m_variable = c->create_variable(m_name);
    }
    else
    {
        m_variable = c->get_variable(m_name);
    }

    sequence::compile(c);
    (*c->generator()) << MOV() << m_variable << reg(0);
    return false;
}
