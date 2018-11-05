#include "variable.h"

#include <memory>

using namespace primal;

std::map<std::string, std::shared_ptr<variable>> variable::variables;
numeric_t variable::location_counter = 0;

variable::variable(const std::string & name) : m_name(name)
{
    // place this variable into the given location of the VM's stack
    m_location = location_counter ++;
}

numeric_t variable::location() const
{
    return m_location;
}

bool variable::has_variable(const std::string & name)
{
    return variables.count(name) != 0;
}

std::shared_ptr<variable> variable::create(const std::string & name)
{
    auto x = std::make_shared<variable>(name);
    variables[name] = x;
    return x;
}

numeric_t variable::last_location()
{
    return location_counter;
}
