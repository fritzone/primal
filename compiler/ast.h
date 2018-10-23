#ifndef AST_H
#define AST_H

#include <memory>
#include <string>
#include <vector>

#include "token.h"

namespace primate
{
// The abstract syntax tree
    class ast
    {
    public:
        ast() = default;

        std::shared_ptr<ast> right;
        std::shared_ptr<ast> left;
        token data;

        /* Builds the AST of the given input token vector inf RPN */
        static void build_ast(std::vector<token>& output, std::shared_ptr<ast>& croot);

    };

}


#endif // AST_H
