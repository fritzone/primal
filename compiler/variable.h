#ifndef VARIABLE_H
#define VARIABLE_H

#include <types.h>

#include <string>
#include <memory>
#include <vector>

namespace primal
{

    class compiler;

    /* This class will hold the variables and their associated memory adresses */
    class variable
    {
    public:

        variable(compiler* c, const std::string& name);

        numeric_t location() const;

    public:

        /* Tells us if the variable with the given name was created or not */
        static bool has_variable(const std::string& name);

        /* when done with the current compilation shut it down */
        static void reset();
        static void introduce_name(const std::string& name);
    private:

        std::string m_name;
        numeric_t m_location;
        static std::vector<std::string> variables;
    };

}

#endif

