#ifndef STRINGTABLE_H
#define STRINGTABLE_H

#include "types.h"

#include <string>
#include <vector>
#include <map>

class stringtable final
{
public:

    struct entry final
    {
        numeric_t location;
        std::string the_string;
    };

    static stringtable& instance();

    numeric_t add(const std::string& s);

    entry& e(numeric_t i);

    numeric_t count() const;

private:

    std::map<numeric_t, entry> m_entries;
};

#endif // STRINGTABLE_H
