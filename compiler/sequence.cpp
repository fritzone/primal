#include "sequence.h"
#include "util.h"
#include "hal.h"
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
#include "numeric_constant.h"
#include "generate.h"
#include "options.h"

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

        if(tokens[0].get_type() == token::type::TT_NUMBER)
        {
            auto constant = std::make_shared<numeric_constant>(src);
            constant->set_value(util::string_to_number<word_t>(tokens[0].data()));
            return constant;
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

    if (tt == token::type::TT_FUNCTION_CALL)
    {
        auto f = fun::get(croot->data.data());
        if (!f) {
            throw syntax_error("Call to undefined function: " + croot->data.data());
        }
        word_t pushed_params = 0;
        if (croot->children.size() > 0) {
            for (auto it = croot->children.rbegin(); it != croot->children.rend(); ++it) {
                traverse_ast(0, *it, c);
                (*c->generator()) << PUSH() << reg(0);
                pushed_params++;
            }
        }
        (*c->generator()) << CALL() << label(c->get_source(), f->name());
        (*c->generator()) << MOV() << reg(level) << reg(0);
        if (pushed_params > 0) {
            word_t stack_cleanup_size = pushed_params * word_size;
            (*c->generator()) << SUB() << reg(255)
                              << type_destination::TYPE_MOD_IMM << stack_cleanup_size;
        }
        return;
    }

    if (tt == token::type::TT_OPERATOR)
    {
        // NEW: Handle array indexing operator for reading a value
        if (croot->data.data() == "[") {
            if (!croot->left || croot->left->data.get_type() != token::type::TT_VARIABLE) {
                throw syntax_error("Array indexing must be applied to a variable.");
            }
            std::string var_name = croot->left->data.data();
            auto var = c->get_variable(var_name);
            if (!var) {
                // This case should ideally not be hit if has_variable checks are correct.
                throw syntax_error("Internal compiler error: could not resolve variable '" + var_name + "' for indexing.");
            }

            if (var->is_array()) { // Numeric array access
                // 1. Evaluate the index expression into r(level + 1)
                traverse_ast(level + 1, croot->right, c);

                // 2. Calculate offset: index * word_size. Result in r(level + 1)
                (*c->generator()) << MUL() << reg(level + 1) << token(std::to_string(word_size), token::type::TT_NUMBER);

                // 3. Get base address of array into r(level + 2)
                (*c->generator()) << MOV() << reg(level + 2) << var;

                // 4. Calculate final address: base_address + offset. Result in r(level + 1)
                (*c->generator()) << ADD() << reg(level + 1) << reg(level + 2);

                // 5. Load the value from the final address into the target register r(level)
                (*c->generator()) << MOV() << reg(level) << type_destination::TYPE_MOD_MEM_REG_IDX << static_cast<uint8_t>(level + 1);

            } else { // String character access
                // 1. Evaluate the index expression into r(level + 1)
                traverse_ast(level + 1, croot->right, c);

                // 2. Get base address of string into r(level + 2)
                (*c->generator()) << MOV() << reg(level + 2) << var;

                // 3. Calculate final address: base_address + index + 1 (for length prefix). Result in r(level + 1)
                (*c->generator()) << ADD() << reg(level + 1) << reg(level + 2);
                (*c->generator()) << ADD() << reg(level + 1) << token("1", token::type::TT_NUMBER);


                // 4. Load the byte from the final address into the target register r(level)
                (*c->generator()) << MOV() << reg(level) << type_destination::TYPE_MOD_MEM_REG_BYTE << static_cast<uint8_t>(level + 1);
            }
            return; // We are done for this branch
        }

        // Existing operator logic
        traverse_ast(level + 1, croot->left, c);
        (*c->generator()) << MOV() << reg(level) << reg(level + 1);
        traverse_ast(level + 1, croot->right, c);
        (*c->generator()) << operators.at(croot->data.data())->opcode << reg(level) << reg(level + 1) ;
    }
    // ... (rest of the function is unchanged) ...
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
                    auto nr_of_strings = stringtable::instance().count();
                    auto in_mem_loc = stringtable::instance().e(croot->data.get_extra_info()).in_mem_location;

                    word_t in_vm_mem_loc = /*(nr_of_strings - 1) * word_size
                                           + */STRING_TABLE_INDEX_IN_MEM
                                           + in_mem_loc;

                    (*c->generator()) << MOV() << reg(level) << type_destination::TYPE_MOD_IMM << in_vm_mem_loc;
                    (*c->generator()) << COPY() << type_destination::TYPE_MOD_IMM << in_vm_mem_loc
                                      << type_destination::TYPE_MOD_IMM;
                    compiled_code::instance(c).string_encountered(croot->data.get_extra_info());
                    for(size_t i=0; i<word_size; i++)
                    {
                        compiled_code::instance(c).append (0xFF);
                        if(options::instance().generate_assembly())
                        {
                            options::instance().asm_stream() << "0xFF ";
                        }
                    }
                    if(options::instance().generate_assembly())
                    {
                        options::instance().asm_stream() << "\n";
                    }
                    std::string to_copy = stringtable::instance().e(croot->data.get_extra_info()).the_string;
                    (*c->generator()) << type_destination::TYPE_MOD_IMM <<
                        static_cast<word_t>(to_copy.length() + 1);
                    if(options::instance().generate_assembly())
                    {
                        options::instance().asm_stream() << "<- LENGTH\n";
                    }
                }

    if (tt == token::type::TT_VARIABLE)
    {
        auto&& v = c->get_variable(croot->data.data());
        if(!v)
        {
            throw std::runtime_error("cannot locate a variable:" + croot->data.data());
        }
        (*c->generator()) << MOV() << reg(level) << v;
    }
}

void sequence::set_string_seq(const std::string &newString_seq)
{
    m_string_seq = newString_seq;
}
