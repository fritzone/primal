#ifndef FUNCTION_H
#define FUNCTION_H

#include <string>
#include <map>

namespace primal {

    class fun
    {
    public:

        static fun register_function(const std::string& name)
        {
            primal::fun f(name);
            m_functions[name] = f;
            return f;
        }

        static fun* get(const std::string& name)
        {
            if(m_functions.count(name))
            {
                return &m_functions[name];
            }
            return nullptr;
        }

        fun() = default;

        explicit fun(const std::string& s) : m_name(s)
        {
        }

        bool has_variadic_parameters() const
        {
            return true;
        }

    private:

        static std::map<std::string, fun> m_functions;

    private:
        std::string m_name;
    };

}


#endif // FUNCTION_H
