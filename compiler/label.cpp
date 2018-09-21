#include "label.h"
#include "generate.h"
#include "compiler.h"

#include <cstdint>
#include <sstream>

uint32_t label::label_counter = 0;

label::label(source& s) : sequence(s)
{
    std::stringstream ss;
    ss << "_lbl_" << label_counter ++;
    this->lblname = ss.str();
}

std::string label::name() const
{
    return this->lblname;
}

label label::create(source& s)
{
    return label(s);
}

sequence::prepared_type label::prepare(std::vector<token> &tokens)
{
    return sequence::prepared_type::PT_NORMAL;
}

bool label::compile(compiler* c)
{
    (*c->gen_code()) << declare_label(*this);
    return true;
}
