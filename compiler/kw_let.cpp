#include "kw_let.h"
#include "variable.h"
#include "generate.h"
#include "opcodes.h"
#include "registers.h"
#include "compiler.h"

using namespace opcodes;

#include <iostream>

bool kw_let::prepare(std::vector<token> &tokens)
{
    if(tokens.empty())
    {
        return false;
    }
    // fetching the name of the token, see if it's a valid identifier or not
    std::string name = tokens[0].data();
    tokens.erase(tokens.begin());

    // now add the variable name into the variables
    if (!variable::has_variable(name))
    {
        m_variable = variable::create(name);
    }
    else
    {
        m_variable = variable::variables[name];
    }

    // erasing the equality sign, check for error
    tokens.erase(tokens.begin());
    if(tokens.empty())
    {
        return false;
    }

    return true;
}

bool kw_let::compile(compiler* c)
{
    sequence::compile(c);
    (*c->gen_code()) << MOV() << m_variable << reg(0);
    return false;
}
