#include "kw_if.h"
#include "operators.h"
#include "ast.h"
#include "parser.h"
#include "generate.h"
#include "label.h"
#include "opcodes.h"
#include "compiler.h"
#include "kw_end.h"

#include <iostream>
#include <options.h>

using namespace primal;
using namespace primal::opcodes;

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
    auto seqs = p.parse(m_src,
            [&](std::string s)
            {
                std::string uprs = util::to_upper(s);
                return uprs == kw_end::N || uprs == "ELSE";
            },
    last);
    m_if_body = std::get<0>(seqs);

    if(util::to_upper(last) == "ELSE")
    {
        auto seqs_else = p.parse(m_src,
                [&](std::string s)
                {
                    return util::to_upper(s) == kw_end::N;
                },
        last);
        m_else_body = std::get<0>(seqs_else);
    }

    return sequence::prepared_type::PT_NORMAL;
}

bool kw_if::compile(compiler* c)
{

    if(options::instance().generate_assembly())
    {
        options::instance().asm_stream() << "===" << m_string_seq << "===" << std::endl;
    }

    // to compile the expression on which the IF takes its decision
    sequence::compile(c);

    // and set up the jumps depending on the trueness of the expression
    label lbl_after_if = label::create(c->get_source());
    label lbl_if_body = label::create(c->get_source());
    label lbl_else = label::create(c->get_source());

    // let's get the comparator for this IF
    comp* comparator = dynamic_cast<comp*>(operators[m_root->data.data()].get());
    if(!comparator)
    {
        throw syntax_error("Invalid IF statement condition. Nothing to compare");
    }
    // jump to IF body if condition is ok
    (*c->generator()) << comparator->jump << lbl_if_body;

    // otherwise jump to else if any
    if(!m_else_body.empty())
    {
        (*c->generator()) << DJMP() << lbl_else;
    }
    else
    {
        (*c->generator()) << DJMP() << lbl_after_if;
    }

    (*c->generator()) << declare_label(lbl_if_body);
    for(const auto& seq : m_if_body)
    {
        seq->compile(c);
    }
    (*c->generator()) << DJMP() << lbl_after_if;
    (*c->generator()) << declare_label(lbl_else);

    // compile the ELSE branch
    for(const auto& seq : m_else_body)
    {
        seq->compile(c);
    }

    (*c->generator()) << declare_label(lbl_after_if);

    return false;

}
