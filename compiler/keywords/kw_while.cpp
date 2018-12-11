#include "kw_while.h"
#include "operators.h"
#include "ast.h"
#include "parser.h"
#include "generate.h"
#include "label.h"
#include "opcodes.h"
#include "compiler.h"
#include "kw_end.h"

#include <iostream>

using namespace primal;
using namespace primal::opcodes;

sequence::prepared_type kw_while::prepare(std::vector<token> &tokens)
{
    if(tokens.empty())
    {
        return sequence::prepared_type::PT_INVALID;
    }

    parser p;
    std::string last;
    auto seqs = p.parse(m_src,
            [&](std::string s)
            {
                return util::to_upper(s) == kw_end::N;
            },
    last);
    m_while_body = std::get<0>(seqs);

    return sequence::prepared_type::PT_NORMAL;
}

bool kw_while::compile(compiler* c)
{
    label lbl_while = label::create(c->get_source());
    (*c->generator()) << declare_label(lbl_while);

    // to compile the expression on which the WHILE takes its decision
    sequence::compile(c);

    // and set up the jumps depending on the trueness of the expression
    label lbl_after_while = label::create(c->get_source());
    label lbl_while_body = label::create(c->get_source());

    // let's get the comparator for this WHILE
    comp* comparator = dynamic_cast<comp*>(operators[m_root->data.data()].get());
    if(!comparator)
    {
        throw syntax_error("Invalid WHILE statement condition. Nothing to compare");
    }
    (*c->generator()) << comparator->jump << lbl_while_body;
    (*c->generator()) << DJMP() << lbl_after_while;

    (*c->generator()) << declare_label(lbl_while_body);
    for(const auto& seq : m_while_body)
    {
        seq->compile(c);
    }
    (*c->generator()) << DJMP() << lbl_while;

    (*c->generator()) << declare_label(lbl_after_while);

    return false;

}
