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
#include "exceptions.h"
#include "function_call.h"
#include "function.h"
#include "stringtable.h"

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

        // Now go through the tokens and see if one of them looks like a function call (func name, followed by a parenthesis)
        // if yes return a function call sequence
        if( fun::get(tokens[0].data()) )
        {
            return std::shared_ptr<function_call>(new function_call(tokens, src));
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

    if (tt == token::type::TT_OPERATOR)
    {
        traverse_ast(level + 1, croot->left, c);
        (*c->generator()) << MOV() << reg(level) << reg(level + 1);
        traverse_ast(level + 1, croot->right, c);
        (*c->generator()) << operators.at(croot->data.data())->opcode << reg(level) << reg(level + 1) ;
    }
    if(tt == token::type::TT_COMPARISON || tt == token::type::TT_LOGICAL)
    {
        traverse_ast(level + 2, croot->left, c);
        (*c->generator()) << MOV() << reg(level + 1) << reg(level + 2);
        traverse_ast(level + 2, croot->right, c);
        (*c->generator()) << operators.at(croot->data.data())->opcode << reg(level + 1) << reg(level + 2) ;
        (*c->generator()) << MOV() << reg(level) << reg(253);
    }
    else
    if(tt == token::type::TT_EXCLAMATION)
    {
        traverse_ast(level + 1, croot->right, c);
        (*c->generator()) << MOV() << reg(level) << reg(level + 1);
        (*c->generator()) << NOT() << reg(level);
    }
    else
    if (tt == token::type::TT_NUMBER)
    {
        (*c->generator()) << MOV() << reg(level) << croot->data;
    }
    else
    if(tt == token::type::TT_STRING)
    {
        // 1. allocate a location for the given string in the memory after the end of the global variables
        word_t after_variables = variable::global_variable_count() * word_size + 4096;

        // 2. mov into the given register level the allocated address
        (*c->generator()) << MOV() << reg(level) << type_destination::TYPE_MOD_IMM << after_variables;

        // 3. copy the data from the string table to the given address
        (*c->generator()) << COPY() << type_destination::TYPE_MOD_IMM << after_variables     // DEST
                          << type_destination::TYPE_MOD_IMM;                                 // SRC

        // 4. notify the compiled code we have a future string reference here
        compiled_code::instance(c).string_encountered(croot->data.get_extra_info());
        for(size_t i=0; i<word_size; i++)                                                    // SRC
        {                                                                                    // SRC
            compiled_code::instance(c).append (0xFF);                                        // SRC
        }                                                                                    // SRC

        std::string to_copy = stringtable::instance().e(croot->data.get_extra_info()).the_string;
        (*c->generator()) << type_destination::TYPE_MOD_IMM <<                               // COUNT
                             static_cast<word_t>(to_copy.length() + 1);                      // +1: because the string starts with the length
    }

    if (tt == token::type::TT_VARIABLE)
    {
        (*c->generator()) << MOV() << reg(level) << c->get_variable(croot->data.data());
    }
}
