#include "types.h"
#include "util.h"

#include <algorithm>

using namespace primal;

entity_type primal::get_entity_type(const std::string &s)
{
    std::string ups = util::to_upper(s);
    if(ups == "NUMBER" || ups == "INTEGER" || ups == "INT" || ups == "NUMERIC") return entity_type::ET_NUMERIC;
    if(ups == "STRING") return entity_type::ET_STRING;
    if(ups == "...") return entity_type::ET_ELLIPSIS;
    if (ups == "FUNCTION") return entity_type::ET_FUNCTION;

    // LATER: walk through all the registered types and fill in the void
    return entity_type::ET_UNKNOWN;
}
