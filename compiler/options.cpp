#include "options.h"

using namespace primal;

options &options::instance()
{
    static options i;
    return i;
}

bool options::generate_assembly() const
{
    return m_generate_assembly;
}

void options::generate_assembly(bool g)
{
    m_generate_assembly = true;
}
