#include "variable.h"
#include "compiler.h"
#include "options.h"

#include <memory>
#include <iostream>
#include <algorithm>

using namespace primal;

std::vector<std::tuple<std::string, entity_type, entity_origin>> variable::variables;
std::string variable::working_function;

variable::variable(compiler* c, const std::string & name) : m_name(name), m_frame(c->frame())
{
    // place this variable into the given location of the VM's stack
    m_location = c->next_varcount(c->frame(), m_name);

    // if it is a parameter substract: BP - 2 * num-t_size - parcount * num-t_size
    //if(options::instance().generate_assembly()) { options::instance().asm_stream() << "CV:" << m_name << "@" << m_location << " - " << reinterpret_cast<void*>(c->frame()) << std::endl; }
}

numeric_t variable::location() const
{
    return m_location;
}

fun *variable::frame() const
{
    return m_frame;
}

bool variable::has_variable(const std::string & name)
{
    {
        auto it = std::find_if( variables.begin(), variables.end(),
                                [name](const std::tuple<std::string, entity_type, entity_origin>& element){ return std::get<0>(element)== name;} );

        if(it != variables.end()) return true;
    }

    auto it = std::find_if( variables.begin(), variables.end(),
                            [name](const std::tuple<std::string, entity_type, entity_origin>& element){ return std::get<0>(element)== working_function + ":" + name;} );

    return it != variables.end();
}

void variable::reset()
{
    variables.clear();
}

void variable::introduce_name(const std::string &name, entity_type et, entity_origin eo)
{
    std::string vn = working_function.empty()? name : working_function + ":" + name;
    std::cout << "introducing " << vn << std::endl;
    variables.push_back ( {vn, et, eo} );

}

void variable::enter_function(const std::string &function_name)
{
    working_function = function_name;
}

void variable::leave_function()
{
    working_function = "";
}
