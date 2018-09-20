#include "kw_if.h"
#include "operators.h"
#include "ast.h"
#include "parser.h"
#include "generate.h"
#include "label.h"
#include "opcodes.h"
#include "compiler.h"

#include <iostream>

using namespace opcodes;

sequence::prepared_type kw_if::prepare(std::vector<token> &tokens)
{
    if(tokens.empty())
    {
        return sequence::prepared_type::PT_INVALID;
    }

    // find the then at the end of this sequence
    if(util::to_upper(tokens.back().data()) != "THEN")
    {
        throw syntax_error("IF without THEN");
    }
    tokens.pop_back();
    // load all the sequences till we find a corresponding endif

    parser p;
    std::string last;
    m_if_body = p.parse(m_src,
            [&](std::string s)
            {
                return util::to_upper(s) == kw_endif::N;
            },
    last);


    return sequence::prepared_type::PT_NORMAL;
}

bool kw_if::compile(compiler* c)
{
    sequence::compile(c);
    label lbl_after_if = label::create();
    label lbl_if_body = label::create();

    (*c->gen_code()) << (dynamic_cast<comp*>(operators[m_root->data.data()].get()))->jump << lbl_if_body;
    (*c->gen_code()) << JMP() << lbl_after_if;

    (*c->gen_code()) << declare_label(lbl_if_body);
    for(const auto& seq : m_if_body)
    {
        seq->compile(c);
    }
    (*c->gen_code()) << declare_label(lbl_after_if);

    return false;

}
