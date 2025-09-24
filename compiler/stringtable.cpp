#include "stringtable.h"

#include <stdexcept>

stringtable &stringtable::instance()
{
    static stringtable st;
    return st;
}

/**
 * @brief stringtable::add
 *
 * Adds a string to the stringtable. Also it calculates the in memory location of the string
 * to be placed after all the previously found strings.
 *
 * @param s
 * @return
 */
word_t stringtable::add(const std::string &s)
{
    if(s.length() > 255)
    {
        throw std::runtime_error("string length exceeds max");
    }
    word_t l = static_cast<word_t>(m_entries.size());
    word_t total_length = 0;
    for (const auto& [key, val] : m_entries)
    {
        total_length += 256;
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
    throw std::runtime_error("internal compiler error: string not found");
}

word_t stringtable::count() const
{
    return static_cast<word_t>(m_entries.size());
}
