#include "ast.h"
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
