#include "stringtable.h"

stringtable &stringtable::instance()
{
    static stringtable st;
    return st;
}

numeric_t stringtable::add(const std::string &s)
{
    numeric_t l = static_cast<numeric_t>(m_entries.size());
    entry e{ l, s};
    m_entries[l] = e;
    return l;
}

stringtable::entry &stringtable::e(numeric_t i)
{
    if(m_entries.count(i))
    {
        return m_entries.at(i);
    }
    throw "internal compiler error: string not found";
}

numeric_t stringtable::count() const
{
    return static_cast<numeric_t>(m_entries.size());
}
