#include "compiler.h"
#include "parser.h"
#include "operators.h"
#include "source.h"
#include "sequence.h"
#include "generate.h"
#include "keywords.h"

#include <opcodes.h>

#include <all_keywords.h>

std::shared_ptr<compiler> compiler::initalize()
{
    register_all_keywords();
    register_opcode_compilers();

    return std::make_shared<compiler>();
}

bool compiler::compile(const std::string &s)
{
    parser p;
    std::string trimmed = s;
    trimmed = util::strim(trimmed);
    m_src = source(trimmed);
    std::string last;
    auto seqs = p.parse(m_src, [&](std::string) {return false;}, last);
    for(const auto& seq : seqs) seq->compile(this);
    compiled_code::instance(this).finalize();

    return true;
}

std::vector<uint8_t> compiler::bytecode() const
{
    return compiled_code::instance(const_cast<compiler*>(this)).bytecode();
}

std::shared_ptr<generate> compiler::gen_code()
{
    return std::make_shared<generate>(this);
}

source &compiler::get_source()
{
    return m_src;
}
