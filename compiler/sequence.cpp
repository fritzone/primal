#include "sequence.h"
#include "util.h"
#include "lexer.h"
#include "operators.h"
#include "keywords/kw_let.h"
#include "keywords/kw_if.h"
#include "ast.h"
#include "variable.h"
#include "generate.h"
#include "opcodes.h"
#include "registers.h"
#include "keywords.h"
#include "compiler.h"
#include "label.h"

#include <iostream>

using namespace primal;
using namespace primal::opcodes;

std::map<std::string, std::function<std::shared_ptr<sequence>(source&)>> keyword::store;


bool sequence::compile(compiler* c)
{
    traverse_ast(0, m_root, c);

    return false;
}

std::shared_ptr<ast>& sequence::root()
{
    return m_root;
}

std::shared_ptr<sequence> sequence::create(std::vector<token> & tokens, source& src)
{
    if(tokens.empty())
    {
        throw syntax_error("Invalid sequence");
    }

    if(tokens[0].get_type() == token::type::TT_LABEL)
    {
        auto lbl = std::make_shared<label>(src);
        lbl->set_name(tokens[0].data().substr(1));
        return lbl;
    }
    else
    {
        std::string keyword = util::to_upper(tokens[0].data());
        if(keyword::store.count(keyword))
        {
            return keyword::store[keyword](src);
        }

        return std::shared_ptr<sequence>();
    }
}

void sequence::traverse_ast(uint8_t level, const std::shared_ptr<ast>& croot, compiler* c)
{
    if (!croot)
    {
        return;
    }
    token::type tt = croot->data.get_type();

    if (tt == token::type::TT_OPERATOR || tt == token::type::TT_COMPARISON)
    {
        traverse_ast(level + 1, croot->left, c);
        (*c->gen_code()) << MOV() << reg(level) << reg(level + 1);
        traverse_ast(level + 1, croot->right, c);
        (*c->gen_code()) << operators.at(croot->data.data())->opcode << reg(level) << reg(level + 1) ;
    }
    else
    if(tt == token::type::TT_EXCLAMATION)
    {
        traverse_ast(level + 1, croot->right, c);
        (*c->gen_code()) << MOV() << reg(level) << reg(level + 1);
        (*c->gen_code()) << NOT() << reg(level);
    }
    else
    if (tt == token::type::TT_NUMBER)
    {
        (*c->gen_code()) << MOV() << reg(level) << croot->data;
    }

    if (tt == token::type::TT_VARIABLE)
    {
        (*c->gen_code()) << MOV() << reg(level) << variable::variables[croot->data.data()];
    }
}
