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
        throw syntax_error(std::string("error, cannot find function to call:") + m_function_name);
    }

    word_t pushed_params = 0;

    for(auto ri = m_params.rbegin(); ri != m_params.rend(); ++ri)
    {
        auto& riroot = ri->root();

        // Case 1: non-string literal/variable/indexed variable
        if(riroot->data.get_type() != token::type::TT_STRING)
        {
            // compile parameter expression -> result in reg0
            sequence::traverse_ast(0, ri->root(), c);

            if(f->has_variadic_parameters() || f->is_extern())
            {
                if(riroot->data.get_type() == token::type::TT_VARIABLE)
                {
                    entity_type et = variable::get_type(riroot->data.data());
                    if(et == entity_type::ET_STRING)
                    {
                        // load actual address of string variable
                        auto v = c->get_variable(riroot->data.data());
                        if(!v)
                        {
                            throw primal::syntax_error("Internal compiler error. Lost a variable.");
                        }

                        (*c->generator()) << MOV() << reg(0) << c->get_variable(riroot->data.data());

                        (*c->generator()) << PUSH()
                                          << type_destination::TYPE_MOD_IMM
                                          << static_cast<word_t>(util::to_integral(entity_type::ET_STRING));
                        pushed_params++;
                    }
                    else
                    {
                        (*c->generator()) << opcodes::PUSH()
                        << type_destination::TYPE_MOD_IMM
                        << static_cast<word_t>(util::to_integral(entity_type::ET_NUMERIC));
                        pushed_params++;
                    }
                }
                else if(riroot->data.get_type() == token::type::TT_VARIABLE && !riroot->children.empty())
                {
                    // Indexed variable, e.g. arr[3]
                    entity_type et = variable::get_type(riroot->data.data());

                    (*c->generator()) << opcodes::PUSH()
                                      << type_destination::TYPE_MOD_IMM
                                      << static_cast<word_t>(util::to_integral(et));
                    pushed_params++;
                }
                else
                {
                    // numeric literal or expression
                    (*c->generator()) << opcodes::PUSH()
                                      << type_destination::TYPE_MOD_IMM
                                      << static_cast<word_t>(util::to_integral(entity_type::ET_NUMERIC));
                    pushed_params++;
                }
            }

            // push actual computed value in reg0
            pushed_params++;
            (*c->generator()) << opcodes::PUSH() << reg(0);
        }
        // Case 2: string literal
        else
        {
            if(ri->tokens().empty())
            {
                throw "internal compiler error";
            }

            if(f->has_variadic_parameters() || f->is_extern())
            {
                (*c->generator()) << opcodes::PUSH()
                    << type_destination::TYPE_MOD_IMM
                    << static_cast<word_t>(util::to_integral(entity_type::ET_STRING));
                pushed_params++;
            }

            // placeholder address of the string (fixed later in finalize)
            (*c->generator()) << opcodes::PUSH() << type_destination::TYPE_MOD_IMM;
            if(options::instance().generate_assembly())
            {
                options::instance().asm_stream() << "# S." << std::dec << ri->tokens()[0].get_extra_info() << std::endl;
            }

            pushed_params++;

            compiled_code::instance(c).string_encountered(ri->tokens()[0].get_extra_info());
            for(size_t i=0; i<word_size; i++)
            {
                compiled_code::instance(c).append(0xFF);
            }
        }
    }

    // if variadic, push parameter count
    if(f->has_variadic_parameters() || f->is_extern())
    {
        (*c->generator()) << opcodes::PUSH()
        << type_destination::TYPE_MOD_IMM
        << pushed_params / 2;
        pushed_params++;
    }

    // perform the actual call
    (*c->generator()) << opcodes::CALL() << label(c->get_source(), f->name());

    // clean up stack
    (*c->generator()) << opcodes::SUB()
                      << reg(255)
                      << type_destination::TYPE_MOD_IMM
                      << (pushed_params * word_size);

    return true;
}
