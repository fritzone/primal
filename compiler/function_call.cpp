#include "function_call.h"
#include <algorithm>

primal::sequence::prepared_type primal::function_call::prepare(std::vector<primal::token>&)
{

    if(m_tokens.empty())
    {
        return prepared_type::PT_INVALID;
    }

    m_function_name = m_tokens[0].data();
    m_tokens.erase(m_tokens.begin());

    // firstly, find the open parenthesis:
    size_t i = 0;
    while(i < m_tokens.size() && m_tokens[i].get_type() != primal::token::type::TT_OPEN_PARENTHESES) i++;

    i++;
    // now build a vector of tokens, where each represents a parameter
    // for now this disables the function calls as parameters, more logic is needed for that
    while(i < m_tokens.size())
    {
        std::vector<primal::token> current_par_toks;
        while(i < m_tokens.size() && m_tokens[i].get_type() != primal::token::type::TT_COMMA
              && m_tokens[i].get_type() != primal::token::type::TT_CLOSE_PARENTHESES)
        {
            current_par_toks.push_back(m_tokens[i]);
            i++;
        }

        i++;
        parameter p(current_par_toks);
        m_params.push_back(p);
    }

    return prepared_type::PT_FUNCTION_CALL;
}

bool primal::function_call::compile(primal::compiler *c)
{
    // firstly: compile the parameters
    for(auto& p : m_params)
    {
        sequence::traverse_ast(0, p.root(), c);
    }
    return true;
}
