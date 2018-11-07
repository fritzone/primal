#include "function.h"
#include "util.h"
#include "source.h"
#include "parser.h"
#include "compiler.h"
#include "opcodes.h"
#include "sequence.h"
#include "generate.h"
#include "options.h"
#include "label.h"

#include <kw_endf.h>

using namespace primal;
using namespace primal::opcodes;

std::map<std::string, std::shared_ptr<fun>> fun::m_functions;

void fun::reset()
{
    m_functions.clear();
}

std::string fun::name() const { return m_name; }

void fun::parse()
{
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
}

bool fun::compile(compiler* c)
{
    label fun_label = label::create(c->get_source());
    fun_label.set_name(m_name);
    (*c->generator()) << declare_label(fun_label);

    // now the header for the function
    (*c->generator()) << PUSH() << reg(254);                // save R254 somewhere
    (*c->generator()) << MOV() << reg(254) << reg(255);     // save the SP into R254

    for(const auto& seq : m_body)
    {
        seq->compile(c);
    }

    (*c->generator()) << MOV() << reg(255) << reg(254);     // Restore the stack pointer
    (*c->generator()) << POP() << reg(254);                 // restore R254 to what it was before

    // and then return to the caller
    (*c->generator()) << RET();
    return true;
}
