#include "kw_fun.h"
#include "kw_end.h"
#include "parser.h"
#include "opcodes.h"
#include "compiler.h"
#include "function.h"
#include "variable.h"

#include <iostream>
#include <options.h>

using namespace primal;

sequence::prepared_type kw_fun::prepare(std::vector<token> &tokens)
{
    if (tokens.empty()) {
        return sequence::prepared_type::PT_INVALID;
    }

    // Function name is the first token.
    m_function = fun::register_function(tokens[0].data(), m_src);
    variable::enter_function(tokens[0].data());
    tokens.erase(tokens.begin());

    // Find parameter list bounds '(' and ')'.
    auto it_open = std::find_if(tokens.begin(), tokens.end(), [](const token& t){ return t.get_type() == token::type::TT_OPEN_PARENTHESES; });
    auto it_close = std::find_if(tokens.begin(), tokens.end(), [](const token& t){ return t.get_type() == token::type::TT_CLOSE_PARENTHESES; });

    if (it_open == tokens.end() || it_close == tokens.end() || it_close < it_open) {
        throw syntax_error("Malformed parameter list for function " + m_function->name());
    }

    // Extract tokens for the parameter list.
    std::vector<token> param_tokens(it_open + 1, it_close);
    m_function->identify_parameters(param_tokens);

    // Check for an optional return type specifier after the parameter list.
    auto it_ret = it_close + 1;
    if (it_ret != tokens.end()) {
        entity_type ret_type = get_entity_type(it_ret->data());
        if (ret_type != entity_type::ET_UNKNOWN) {
            m_function->set_return_type(ret_type);
        } else {
            throw syntax_error("Invalid return type specified for function " + m_function->name());
        }
    }

    // Parse the function body.
    m_function->parse();
    variable::leave_function();

    return sequence::prepared_type::PT_FUNCTION_DECL;
}

bool kw_fun::compile(compiler* c)
{

    if(options::instance().generate_assembly())
    {
        options::instance().asm_stream() << "===" << m_string_seq << "===" << std::endl;
    }

    return m_function->compile(c);
}
