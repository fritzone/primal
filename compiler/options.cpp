#include "options.h"

using namespace primate;

options &options::instance()
{
    static options i;
    return i;
}

bool options::generate_assembly() const
{
    return m_generate_assembly;
}
