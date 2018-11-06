#ifndef PRIMAL_EXCEPTIONS_H
#define PRIMAL_EXCEPTIONS_H

#include <stdexcept>

namespace primal
{
    class syntax_error : public std::runtime_error
    {
    public:
        explicit syntax_error(const std::string& s) : std::runtime_error(s), m_message("[syntax error] " + s)  {}
        ~syntax_error() override = default;
        const char* what() const noexcept override
        {
            return m_message.c_str();
        }
    private:
        std::string m_message;
    };

    class vm_panic : public std::runtime_error
    {
    public:
        explicit vm_panic(const std::string& s) : std::runtime_error(s), m_message("[VM panic] " + s)  {}
        const char* what() const noexcept override
        {
            return m_message.c_str();
        }
    private:
        std::string m_message;
    };
}


#endif
