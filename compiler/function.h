#ifndef FUNCTION_H
#define FUNCTION_H

#include "parameter.h"

#include <string>
#include <map>
#include <memory>
#include <vector>
#include <exceptions.h>

namespace primal {

    class source;
    class sequence;
    class compiler;

    const int MAX_FUNCTION_NAME_LEN = 255;
    const int MAX_FUNCTION_PARAMS = 32;

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
            return m_variadic;
        }

        void identify_parameters(std::vector<token>& t);

        std::string name() const;

        void parse();
        bool compile(compiler *c);
        parameter* get_parameter(const std::string& name);

        // returns the parameters' index. Starts from 1.
        int get_parameter_index(const parameter* p);

        size_t get_parameter_count() const { return m_parameters.size(); }
        void set_return_type(entity_type et) { m_return_type = et; }
        entity_type get_return_type() const { return m_return_type; }

        const std::vector<parameter>& get_parameters() const { return m_parameters; }


        void set_address(word_t addr) { m_address = addr; }
        word_t get_address() const { return m_address; }

        static const std::map<std::string, std::shared_ptr<fun>>& get_functions();

        bool is_extern() const;
        void set_extern(bool newExtern);

    private:

        static std::map<std::string, std::shared_ptr<fun>> m_functions;

    private:
        std::string m_name;
        std::vector<std::shared_ptr<sequence>> m_body;
        source& m_src;
        std::vector<parameter> m_parameters;
        bool m_variadic = false;
        entity_type m_return_type = entity_type::ET_UNKNOWN;
        word_t m_address = 0;
        bool m_extern = false;
    };

}


#endif // FUNCTION_H
