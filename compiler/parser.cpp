#include "parser.h"
#include "operators.h"
#include "util.h"
#include "source.h"
#include "sequence.h"
#include "lexer.h"
#include "ast.h"
#include "stringtable.h"

#include <iostream>
#include <map>
#include <stack>
#include <memory>
#include <sstream>

using namespace primal;

std::vector<token> parser::shuntyard(std::vector<token>& tokens)
{
    std::vector<token> result;
    std::stack<token> stck;

    for (token& t : tokens)
    {
        std::string s = t.data();
        token::type tt = t.get_type();
        if (tt == token::type::TT_NUMBER || tt == token::type::TT_VARIABLE || tt == token::type::TT_STRING)
        {
            result.insert(result.begin(), t);
            if(tt == token::type::TT_STRING)
            {
                word_t idx = stringtable::instance().add(s);
                t.set_extra_info(idx);
            }
        }
        else
        {
            if(tt == token::type::TT_EXCLAMATION)
            {
                stck.push(t);
            }
            else
            if (tt == token::type::TT_OPERATOR || tt == token::type::TT_COMPARISON || tt == token::type::TT_LOGICAL) // is this an operator?
            {
                if (!stck.empty())
                {
                    auto t2 = stck.top();
                    if (t2.get_type() != token::type::TT_OPEN_PARENTHESES && (operators[t2.data()]->precedence >= operators[s]->precedence))
                    {
                        result.insert(result.begin(), stck.top());
                        stck.pop();
                    }
                }
                stck.push(t);
            }
            else
            {
                if (tt == token::type::TT_OPEN_PARENTHESES)
                {
                    stck.push(t);
                }
                if (tt == token::type::TT_CLOSE_PARENTHESES)
                {
                    while (stck.top().get_type() != token::type::TT_OPEN_PARENTHESES)
                    {
                        auto c = stck.top();
                        result.insert(result.begin(), c);
                        stck.pop();
                    }
                    stck.pop();
                }
            }
        }
    }

    while (!stck.empty())
    {
        auto c = stck.top();
        result.insert(result.begin(), c);
        stck.pop();
    }
    return result;

}
