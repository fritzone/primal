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

    for(const auto& seq : m_body)
    {
        seq->compile(c);
    }
    (*c->generator()) << RET();
    return true;
}
