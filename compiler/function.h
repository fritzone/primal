#ifndef FUNCTION_H
#define FUNCTION_H

#include <string>
#include <map>
#include <memory>
#include <vector>
#include <exceptions.h>

namespace primal {

    class source;
    class sequence;
    class compiler;

    class fun
    {
    public:

        static std::shared_ptr<fun> register_function(const std::string& name, source& src)
        {
            if(get(name))
            {
                throw syntax_error("Duplicate function name:" + name);
            }
            std::shared_ptr<fun> f( new fun(name, src) );
            m_functions[name] = f;
            return f;
        }

        static std::shared_ptr<fun> get(const std::string& name)
        {
            if(m_functions.count(name))
            {
                return m_functions[name];
            }
            return nullptr;
        }

        static void reset();

        fun() = default;

        explicit fun(const std::string& s, source& src) : m_name(s), m_src(src)
        {
        }

        bool has_variadic_parameters() const
        {
            return true;
        }

        std::string name() const;

        void parse();
        bool compile(compiler *c);

    private:

        static std::map<std::string, std::shared_ptr<fun>> m_functions;

    private:
        std::string m_name;
        std::vector<std::shared_ptr<sequence>> m_body;
        source& m_src;

    };

}


#endif // FUNCTION_H
