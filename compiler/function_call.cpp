#include "function_call.h"
#include <algorithm>

primal::sequence::prepared_type primal::function_call::prepare(std::vector<primal::token>&)
{

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
        m_params_tokens.push_back(current_par_toks);
    }

    return prepared_type::PT_FUNCTION_CALL;
}
