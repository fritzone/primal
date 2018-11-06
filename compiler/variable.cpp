#include "variable.h"
#include "compiler.h"

#include <memory>
#include <iostream>
#include <algorithm>

using namespace primal;

std::vector<std::string> variable::variables;

variable::variable(compiler* c, const std::string & name) : m_name(name)
{
    // place this variable into the given location of the VM's stack
    m_location = c->next_varcount(c->frame());
    std::cout << "create var:" << m_name << "@" << m_location << " - " << (void*)c->frame() << std::endl;
}

numeric_t variable::location() const
{
    return m_location;
}

bool variable::has_variable(const std::string & name)
{
    return std::find(variables.begin(), variables.end(), name) != variables.end();
}

void variable::reset()
{
    variables.clear();
}

void variable::introduce_name(const std::string &name)
{
    variables.push_back(name);
}
