#ifndef PARSER_H
#define PARSER_H

#include "util.h"
#include "lexer.h"
#include "source.h"
#include "sequence.h"
#include "ast.h"

#include <string>
#include <vector>
#include <memory>

namespace primate
{
/* Represents the parsing phase of the compiler, ie. the one which creates the AST for one
   expression at a time */
    class parser
    {
    public:

        template<class CH>
        std::vector<std::shared_ptr<sequence>> parse(source& input, CH checker, std::string& last_read)
        {
            std::vector<std::shared_ptr<sequence>> result;
            while (!input.empty())
            {
                std::string next_seq = input.next();
                if(checker(next_seq))
                {
                    last_read = next_seq;
                    break;
                }

                // this might be an empty line
                if (next_seq.empty())
                {
                    continue;
                }

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
                        continue;
                    }
                    throw syntax_error(next_seq);
                }

                // We remove the first element of the tokens, which always is the keyword
                tokens.erase(tokens.begin());

                // let the current keyword sequence do required modifications on the tokens
                auto prep_t = seq->prepare(tokens);
                if(prep_t == sequence::prepared_type::PT_INVALID)
                {
                    throw syntax_error(next_seq);
                }

                if(prep_t != sequence::prepared_type::PT_CONSUMED)
                {
                    // create the RPN for the expression
                    std::vector<token> output = shuntyard(tokens);

                    // build the abstract syntax tree for the result of the shuntyard
                    ast::build_ast(output, seq->root());
                }
                result.push_back(seq);
            }

            return result;

        }
    private:

        /* Creates the Reverse Polish Notation of the given tokens */
        std::vector<token> shuntyard(const std::vector<token> &tokens);

    };

}

#endif
