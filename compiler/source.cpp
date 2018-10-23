#include "source.h"
#include "util.h"

using namespace primal;

source::source(const char* src) : m_ss(src)
{
}

source::source(const std::string& src) : source(src.c_str())
{

}

bool source::empty()
{
    return m_empty;
}

std::string source::next()
{
    std::string next_sequence;
    if (std::getline(m_ss, next_sequence, '\n'))
    {
        return util::strim(next_sequence);
    }
    else
    {
        m_empty = true;
        return "";
    }    
}
