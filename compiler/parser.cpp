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

void parser::parse(source &input, std::string &next_seq, std::vector<std::shared_ptr<sequence> > &result_operations, std::vector<std::shared_ptr<sequence> > &result_func_decl)
{

    // Now, tokenizing the sequence
    lexer l(next_seq);
    std::vector<token> tokens = l.tokenize();
    if (tokens.empty())
    {
        throw syntax_error(next_seq);
    }

    std::shared_ptr<sequence> seq = sequence::create(tokens, input);
    if (!seq)
    {
        if(tokens[0].get_type() == token::type::TT_COMMENT_LINE)
        {
            return;
        }
        throw syntax_error(next_seq);
    }

    // We remove the first element of the tokens, which always is the keyword
    tokens.erase(tokens.begin());

    // let the current keyword sequence do required modifications on the tokens
    sequence::prepared_type prep_t = sequence::prepared_type::PT_INVALID;
    try
    {
        prep_t = seq->prepare(tokens);
    }
    catch (primal::syntax_error& e)
    {
        std::string msg = e.what();
        throw syntax_error("[syntax error] " + msg + " -> " + next_seq);
    }

    if(prep_t == sequence::prepared_type::PT_INVALID)
    {
        throw syntax_error("[syntax error] invalid code sequence -> " + next_seq);
    }

    if(prep_t == sequence::prepared_type::PT_FUNCTION_DECL)
    {
        result_func_decl.push_back(seq);
    }
    else
    {
        if(prep_t == sequence::prepared_type::PT_FUNCTION_CALL)
        {
            // for each parameter create an AST
            primal::function_call* fc = dynamic_cast<primal::function_call*>(seq.get()) ;
            if (!fc)
            {
                throw primal::syntax_error("[syntax error] invalid function call -> " + next_seq);
            }
            for(auto& p : fc->params())
            {
                std::vector<token> parIoutput = shuntyard(p.tokens());
                ast::build_ast(parIoutput, p.root());
            }
            fc->root().reset(new ast());
            token t;
            t.set_type(token::type::TT_FUNCTION_CALL);
            fc->root()->data = t;
        }
        else
        {
            if(prep_t != sequence::prepared_type::PT_CONSUMED && prep_t != sequence::prepared_type::PT_PRECOMPILED)
            {
                // create the RPN for the expression
                std::vector<token> output = shuntyard(tokens);

                // build the abstract syntax tree for the result of the shuntyard
                ast::build_ast(output, seq->root());
            }
        }
        result_operations.push_back(seq);
    }
}

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
            if(tt == token::type::TT_STRING)
            {
                word_t idx = stringtable::instance().add(s);
                t.set_extra_info(idx);
            }
            result.insert(result.begin(), t);
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
