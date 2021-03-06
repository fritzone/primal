#include "lexer.h"
#include "util.h"
#include "variable.h"

using namespace primal;

std::vector<token> lexer::tokenize()
{
    std::vector<token> result;

    size_t i = 0;
    size_t l = m_sequence.length();
    while (i < l)
    {
        // skip the whitespace, leave if reached en of the string
        util::skip_whitepsace(i, m_sequence); if (i == m_sequence.length()) break;

        // get the type of the current token
        // now parse the token starting at expr[i]
        token current_token {m_sequence[i], token::identify_type(m_sequence[i]) };

        i++;

        if(current_token.get_type() == token::type::TT_STRING)  // just read in the string
        {
            while(i < l)
            {
                current_token.extend(m_sequence[i]);
                if(m_sequence[i] == '"')
                {
                    if(m_sequence[i - 1] != '\\')
                    {
                        break;
                    }
                    std::string dt = current_token.data();
                    if(dt.length() >= 2)
                    {
                        dt[dt.length() - 2] = '"';
                        dt = dt.substr(0, dt.length() - 1);
                        current_token.set_data(dt);
                    }
                }
                i++;
            }
            i++; // i points to the closing " move it forward
            // now remove the starting and ending double quotes from the string
            std::string dt = current_token.data();
            dt = dt.substr(1);
            dt = dt.substr(0, dt.length() - 1);
            current_token.set_data(dt);
        }
        else
        {
            while (i < m_sequence.length() && !util::is_whitespace(m_sequence[i]) )
            {
                if(result.empty())
                {
                    if (current_token.is_allowed_to_combine_token(m_sequence[i]))
                    {
                        current_token.extend(m_sequence[i]);
                        i++;
                    }
                    else
                    {
                        break;
                    }
                }
                else
                {
                    // see if this is an unary operation on a number
                    if (current_token.is_allowed_to_combine_token(m_sequence[i], result.back()))
                    {
                        current_token.extend(m_sequence[i]);

                        auto new_type = token::identify_type(m_sequence[i]);

                        if(
                                (!(current_token.get_type() == token::type::TT_IDENTIFIER && new_type == token::type::TT_NUMBER))
                             && (!(current_token.get_type() == token::type::TT_REGISTER))
                          )
                        {
                            current_token.set_type(new_type);    // this might change later
                        }

                        i++;
                    }
                    else
                    {
                        break;
                    }
                }
            }

            auto s = current_token.data();

            // now see if this current token is a variable or not
            if(variable::has_variable(s))
            {
                current_token.set_type(token::type::TT_VARIABLE);
            }
            if(util::is_comparison(s))
            {
                current_token.set_type(token::type::TT_COMPARISON);
            }
            if(util::is_logical_op(s))
            {
                current_token.set_type(token::type::TT_LOGICAL);
            }
        }
        result.push_back(current_token);
        if(current_token.get_type() == token::type::TT_COMMENT_LINE) break;
    }

    return result;
}

