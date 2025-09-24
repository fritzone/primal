#include "variable.h"
#include "compiler.h"
#include "options.h"

#include <memory>
#include <iostream>
#include <algorithm>

using namespace primal;

std::vector<std::tuple<std::string, entity_type, entity_origin, word_t>> variable::variables;
std::string variable::working_function;
word_t variable::global_var_cnt = 0;

variable::variable(compiler* c, const std::string & name, word_t size) : m_name(name), m_size(size), m_frame(c->frame()), m_compiler(c)
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
                                [name](const std::tuple<std::string, entity_type, entity_origin, word_t>& element){ return std::get<0>(element)== name;} );

        if(it != variables.end()) return true;
    }

    auto it = std::find_if( variables.begin(), variables.end(),
                            [name](const std::tuple<std::string, entity_type, entity_origin, word_t>& element){ return std::get<0>(element)== working_function + ":" + name;} );

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

void variable::introduce_name(const std::string &name, entity_type et, entity_origin eo, word_t size)
{
    std::string vn = working_function.empty()? name : working_function + ":" + name;
    variables.push_back ( std::make_tuple(vn, et, eo, size) );
    if(working_function.empty())
    {
        // For arrays, we increment the global counter by the size of the array
        global_var_cnt += size;
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

word_t variable::get_size(const std::string& name) {
    auto find_var = [&](const auto& element) {
        const auto& var_name = std::get<0>(element);
        return var_name == name || var_name == (working_function + ":" + name);
    };
    auto it = std::find_if(variables.rbegin(), variables.rend(), find_var);
    if (it == variables.rend()) return 0; // Or throw an error
    return std::get<3>(*it);
}

const std::vector<std::tuple<std::string, entity_type, entity_origin, word_t>>& variable::get_declarations() {
    return variables;
}
