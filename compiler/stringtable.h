#ifndef STRINGTABLE_H
#define STRINGTABLE_H

#include "numeric_decl.h"

#include <string>
#include <vector>
#include <map>

class stringtable final
{
public:

    struct entry final
    {
        word_t location {0};
        std::string the_string;
        word_t in_mem_location {0};
    };

    static stringtable& instance();

    word_t add(const std::string& s);

    entry& e(word_t i);

    word_t count() const;

private:

    std::map<word_t, entry> m_entries;
};

#endif // STRINGTABLE_H
