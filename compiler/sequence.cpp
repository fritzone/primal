#include "sequence.h"
#include "util.h"
#include "lexer.h"
#include "operators.h"
#include "kw_let.h"
#include "kw_if.h"
#include "ast.h"
#include "variable.h"
#include "generate.h"
#include "opcodes.h"
#include "registers.h"
#include "keywords.h"
#include "compiler.h"

#include <iostream>


std::map<std::string, std::function<std::shared_ptr<sequence>(source&)>> keyword::store;

using namespace opcodes;

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
    std::string keyword = util::to_upper(tokens[0].data());
    if(keyword::store.count(keyword))
    {
        return keyword::store[keyword](src);
    }

    return std::shared_ptr<sequence>();
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
    if (tt == token::type::TT_NUMBER)
    {
        (*c->gen_code()) << MOV() << reg(level) << croot->data;
    }

    if (tt == token::type::TT_VARIABLE)
    {
        (*c->gen_code()) << MOV() << reg(level) << variable::variables[croot->data.data()];
    }
}
