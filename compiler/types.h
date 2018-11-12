#ifndef TYPES_H
#define TYPES_H

#include <vector>
#include <string>

namespace primal
{
    /* this is a global type identifier for the entities (variables or parameters) that can be in the language */
    enum class entity_type
    {
        ET_NUMERIC = 0,
        ET_STRING = 1,
        ET_ELLIPSIS = 2,
        ET_UNKNOWN = 255
    };

    /* this is a global origin identifier for the entities (variables or parameters) that can be in the language */
    enum class entity_origin
    {
        EO_VARIABLE = 0,
        EO_PARAMETER = 0,
        ET_UNKNOWN = 255
    };

    // out of the box the script supports the following types
    static std::vector<std::string> default_types {"integer", "string"};

    // tells us if the required string was introduced as type in the system
    entity_type get_entity_type(const std::string& s);
}

#endif // TYPES_H
