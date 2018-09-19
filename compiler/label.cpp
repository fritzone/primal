#include "label.h"

#include <cstdint>
#include <sstream>

uint32_t label::label_counter = 0;

label::label()
{
    std::stringstream ss;
    ss << "_lbl_" << label_counter ++;
    this->lblname = ss.str();
}

std::string label::name() const
{
    return this->lblname;
}

label label::create()
{
    return label();
}
