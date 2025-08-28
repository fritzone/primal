#include "function_call.h"
#include "compiler.h"
#include "generate.h"
#include "ast.h"
#include "stringtable.h"
#include "util.h"
#include "function.h"
#include "options.h"
#include "label.h"
#include "variable.h"

#include <opcodes.h>

#include <algorithm>

using namespace primal::opcodes;

primal::sequence::prepared_type primal::function_call::prepare(std::vector<primal::token>&)
{

    if(m_tokens.empty())
    {
        return prepared_type::PT_INVALID;
    }

    m_function_name = m_tokens[0].data();
    m_tokens.erase(m_tokens.begin());

    // firstly, find the open parenthesis:
    size_t i = 0;
    while(i < m_tokens.size() && m_tokens[i].get_type() != primal::token::type::TT_OPEN_PARENTHESES) i++;

    i++;
    // now build a vector of tokens, where each represents a parameter
    // for now this disables the function calls as parameters, more logic is needed for that
    while(i < m_tokens.size())
    {
        std::vector<primal::token> current_par_toks;
        while(i < m_tokens.size() && m_tokens[i].get_type() != primal::token::type::TT_COMMA
              && m_tokens[i].get_type() != primal::token::type::TT_CLOSE_PARENTHESES)
        {
            current_par_toks.push_back(m_tokens[i]);
            i++;
        }

        i++;
        parameter_inst p(current_par_toks);
        m_params.push_back(p);
    }

    return prepared_type::PT_FUNCTION_CALL;
}

bool primal::function_call::compile(primal::compiler *c)
{
    if(options::instance().generate_assembly())
    {
        options::instance().asm_stream() << "===" << m_string_seq << "===" << std::endl;
    }

    auto f = primal::fun::get(m_function_name);
    if(!f)
    {
        return false;
    }

    word_t pushed_params = 0;

    for(auto ri = m_params.rbegin(); ri != m_params.rend(); ++ri)
    {
        auto& riroot = ri->root();
        if(riroot->data.get_type() != token::type::TT_STRING)
        {
            // firstly: compile the parameter
            sequence::traverse_ast(0, ri->root(), c);
            // then push reg0 to the stack, since that contains the value of the parameter

            if(f->has_variadic_parameters())
            {
                if(riroot->data.get_type() == token::type::TT_VARIABLE)
                {
                    entity_type et = variable::get_type(riroot->data.data());
                    if(et == entity_type::ET_STRING)
                    {
                        // now mov int reg 0 the actual address of the string
                        auto v = c->get_variable(riroot->data.data());
                        if(!v)
                        {
                            throw primal::syntax_error("Internal compiler error. Lost a variable.");
                        }

                        (*c->generator()) << MOV() << reg(0) << c->get_variable(riroot->data.data());


                        // send ot the string type
                        (*c->generator()) << PUSH()
                                          << type_destination ::TYPE_MOD_IMM
                                          << static_cast<word_t>(util::to_integral(entity_type::ET_STRING));
                        pushed_params ++;
                    }
                    else
                    {
                        (*c->generator()) << opcodes::PUSH()
                                          << type_destination ::TYPE_MOD_IMM
                                          << static_cast<word_t>(util::to_integral(entity_type::ET_NUMERIC));
                        pushed_params ++;
                    }
                }
                else    // just a normal literal number
                {
                    (*c->generator()) << opcodes::PUSH()
                                      << type_destination ::TYPE_MOD_IMM
                                      << static_cast<word_t>(util::to_integral(entity_type::ET_NUMERIC));
                    pushed_params ++;
                }
            }
            pushed_params ++;
            (*c->generator()) << opcodes::PUSH() << reg(0);
        }
        else
        {
            if(ri->tokens().empty())
            {
                throw "internal compiler error";
            }

            if(f->has_variadic_parameters())
            {
                (*c->generator()) << opcodes::PUSH()
                                  << type_destination ::TYPE_MOD_IMM
                                  << static_cast<word_t>(util::to_integral(entity_type::ET_STRING));
                pushed_params ++;

            }

            // send out a PUSH with the bogus memory address of the string, which will be fixed in the finalize phase
            (*c->generator()) << opcodes::PUSH() << type_destination ::TYPE_MOD_IMM;
            if(options::instance().generate_assembly()) { options::instance().asm_stream() << "# S." << std::dec << ri->tokens()[0].get_extra_info() << std::endl; }

            pushed_params ++;

            // notify the compiled code we have a future string reference here
            compiled_code::instance(c).string_encountered(ri->tokens()[0].get_extra_info());

            for(size_t i=0; i<word_size; i++)
            {
                compiled_code::instance(c).append (0xFF);
            }

        }
    }

    // now, if variadic function push the number of parameters
    if(f->has_variadic_parameters())
    {
        (*c->generator()) << opcodes::PUSH()
                          << type_destination ::TYPE_MOD_IMM << pushed_params / 2;
        pushed_params ++;
    }

    // and now actually call the function
    (*c->generator()) << opcodes::CALL() << label(c->get_source(), f->name());

    // and now actually remove the pushed elements from the stack
    (*c->generator()) << opcodes::SUB() << reg(255) << type_destination::TYPE_MOD_IMM << (pushed_params * word_size);


    return true;
}
