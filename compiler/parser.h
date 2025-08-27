#ifndef PARSER_H
#define PARSER_H

#include "util.h"
#include "lexer.h"
#include "source.h"
#include "sequence.h"
#include "ast.h"
#include "exceptions.h"
#include "function_call.h"

#include <string>
#include <vector>
#include <memory>
#include <iostream>

namespace primal
{
    /* Represents the parsing phase of the compiler, ie. the one which creates the AST for one
       expression at a time */
    class parser
    {
    public:

        template<class CH>
        std::tuple<std::vector<std::shared_ptr<sequence>>, std::vector<std::shared_ptr<sequence>>>
        parse(source& input, CH checker, std::string& last_read)
        {
            std::vector<std::shared_ptr<sequence>> result_operations;
            std::vector<std::shared_ptr<sequence>> result_func_decl;

            while (!input.empty())
            {
                std::string next_seq = input.next();

                std::cout << "NEXT_SEQ: " << next_seq << std::endl;

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
                parse(input, next_seq, result_operations, result_func_decl);
            }
            return std::make_tuple(result_operations, result_func_decl);

        }

        /* Creates the Reverse Polish Notation of the given tokens */
        static std::vector<token> shuntyard(std::vector<token> &tokens);
    private:

        void parse(source& input, std::string& next_seq,
                   std::vector<std::shared_ptr<sequence>>& result_operations,
                   std::vector<std::shared_ptr<sequence>>& result_func_decl);



    };

}

#endif
