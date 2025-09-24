#include "source.h"
#include "util.h"

using namespace primal;
source* source::m_s_instance = nullptr;

source::source()
{
    m_s_instance = this;
}

source::source(const char* src) : m_ss(src)
{
    m_s_instance = this;
}

source::source(const std::string& src) : source(src.c_str())
{
    m_s_instance = this;
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

source &source::get()
{
    return *m_s_instance;
}
