#ifndef VARIABLE_H
#define VARIABLE_H

#include "types.h"

#include <numeric_decl.h>

#include <string>
#include <memory>
#include <vector>

namespace primal
{

    class compiler;
    class fun;

    /* This class will hold the variables and their associated memory adresses */
    class variable
    {
    public:

        variable(compiler* c, const std::string& name);
        word_t location() const;
        fun* frame() const;

    public:

        /* Tells us if the variable with the given name was created or not */
        static bool has_variable(const std::string& name);

        /* when done with the current compilation shut it down */
        static void reset();
        static void introduce_name(const std::string &name, entity_type et, entity_origin eo);
        static void enter_function(const std::string& function_name);
        static void leave_function();

    private:

        std::string m_name;
        word_t m_location;
        static std::vector<std::tuple<std::string, entity_type, entity_origin>> variables;
        static std::string working_function;
        fun* m_frame;
    };

}

#endif

