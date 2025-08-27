#include "ast.h"
#include "function.h"

#include <sstream>

void primal::ast::build_ast(std::vector<token>& output, std::shared_ptr<ast>& croot)
{
    if (output.empty())
    {
        return;
    }

    if (croot == nullptr)
    {
        croot.reset(new ast);
        croot->left.reset();
        croot->right.reset();

        token t = *output.begin();
        croot->data = t;

        // remove current element, it's already in the tree
        output.erase(output.begin());

        if (t.get_type() == token::type::TT_FUNCTION_CALL)
        {
            auto f = fun::get(t.data());
            if (!f) {
                throw std::string("Internal compiler error: function not found in AST build: " + t.data());
            }

            // For variadic functions, we can't know the number of arguments here.
            // This implementation assumes fixed-arity functions for return values.
            // The special `write` function is handled elsewhere.
            if (!f->has_variadic_parameters()) {
                size_t num_args = f->get_parameter_count();
                for (size_t i = 0; i < num_args; ++i) {
                    std::shared_ptr<ast> arg_ast;
                    build_ast(output, arg_ast);
                    croot->children.push_back(arg_ast);
                }
                // The arguments are parsed from the RPN in reverse order, so we
                // reverse the children vector to get them in the correct order.
                std::reverse(croot->children.begin(), croot->children.end());
            }
            return; // We are done with this node.
        }

        // are we supposed to return from here?
        if (t.get_type() == token::type::TT_OPERATOR ||
            t.get_type() == token::type::TT_COMPARISON ||
            t.get_type() == token::type::TT_LOGICAL ||
            t.get_type() == token::type::TT_EXCLAMATION
           )
        {
            // build the right part
            build_ast(output, croot->right);
            if(t.get_type() == token::type::TT_EXCLAMATION)
            {
                // NOT operation has no left side
                return;
            }
            // build the left part of what is there
            build_ast(output, croot->left);
        }
    }
}
