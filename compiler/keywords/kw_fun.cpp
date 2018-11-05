#include "kw_fun.h"
#include "kw_endf.h"
#include "operators.h"
#include "ast.h"
#include "parser.h"
#include "generate.h"
#include "label.h"
#include "opcodes.h"
#include "compiler.h"
#include "function.h"

#include <iostream>

using namespace primal;
using namespace primal::opcodes;

sequence::prepared_type kw_fun::prepare(std::vector<token> &tokens)
{
    if(tokens.empty())
    {
        return sequence::prepared_type::PT_INVALID;
    }

    // function name should be the first token
    fun::register_function(tokens[0].data());

    // npw resolve the parameters, etc...
    parser p;
    std::string last;
    auto seqs = p.parse(m_src,
            [&](std::string s)
            {
                return util::to_upper(s) == kw_endf::N;
            },
    last);
    m_body = std::get<0>(seqs);

    return sequence::prepared_type::PT_FUNCTION_DECL;
}

bool kw_fun::compile(compiler* c)
{

    for(const auto& seq : m_body)
    {
        seq->compile(c);
    }
    (*c->generator()) << RET();

    return false;

}
