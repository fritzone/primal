#include "variable.h"
#include "compiler.h"
#include "options.h"

#include <memory>
#include <iostream>
#include <algorithm>

using namespace primal;

std::vector<std::tuple<std::string, entity_type, entity_origin>> variable::variables;
std::string variable::working_function;
word_t variable::global_var_cnt = 0;

variable::variable(compiler* c, const std::string & name) : m_name(name), m_frame(c->frame()), m_compiler(c)
{
    // place this variable into the given location of the VM's stack
    m_location = c->next_varcount(c->frame(), m_name);

    // if it is a parameter substract: BP - 2 * num-t_size - parcount * num-t_size
    //if(options::instance().generate_assembly()) { options::instance().asm_stream() << "CV:" << m_name << "@" << m_location << " - " << reinterpret_cast<void*>(c->frame()) << std::endl; }
}

word_t variable::location() const
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

word_t variable::global_variable_count()
{
    return global_var_cnt;
}

void variable::reset()
{
    variables.clear();
    working_function = "";
    global_var_cnt = 0;
}

void variable::introduce_name(const std::string &name, entity_type et, entity_origin eo)
{
    std::string vn = working_function.empty()? name : working_function + ":" + name;
    variables.push_back ( std::make_tuple(vn, et, eo) );
    if(working_function.empty())
    {
        global_var_cnt ++;
    }
}

void variable::enter_function(const std::string &function_name)
{
    working_function = function_name;
}

void variable::leave_function()
{
    working_function = "";
}

entity_type variable::get_type(const std::string &name)
{
    auto i = std::find_if(variables.begin(), variables.end(),
                          [&](auto& t) -> bool
                          {
                              std::string nm = std::get<0>(t);
                              return nm == name;
                          }
    );
    if(i == variables.end()) return entity_type::ET_UNKNOWN;
    return std::get<1>(*i);
}
