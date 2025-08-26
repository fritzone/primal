#include "stringtable.h"

stringtable &stringtable::instance()
{
    static stringtable st;
    return st;
}

word_t stringtable::add(const std::string &s)
{
    word_t l = static_cast<word_t>(m_entries.size());
    word_t total_length = 0;
    for (const auto& [key, val] : m_entries)
    {
        total_length += val.the_string.size() + 1;
    }
    entry e{ l, s, total_length};
    m_entries[l] = e;
    return l;
}

stringtable::entry &stringtable::e(word_t i)
{
    if(m_entries.count(i))
    {
        return m_entries.at(i);
    }
    throw "internal compiler error: string not found";
}

word_t stringtable::count() const
{
    return static_cast<word_t>(m_entries.size());
}
