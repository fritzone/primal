#ifndef FUNCTION_CALL_H
#define FUNCTION_CALL_H

#include "sequence.h"
#include "token.h"

#include <vector>

namespace primal
{

    /**
     * Represents a parameter that goes into a given function call
     */
    class parameter final
    {
    public:

        enum class param_type
        {
            PT_NUMERIC = 0,
            PT_STRING = 1,
            PT_UNKNOWN = 255
        };

        explicit parameter(const std::vector<token>& t) : m_params_tokens(t) {}

        std::vector<token>& tokens() {return m_params_tokens; }
        std::shared_ptr<ast>& root() {return m_param_tree; }

    private:
        std::vector<token> m_params_tokens;
        std::shared_ptr<ast> m_param_tree;
    };

    /**
     * Represents a function/subroutine call sequence
     */
    class function_call : public sequence
    {
    public:
        ~function_call() override = default;

        function_call(const std::vector<primal::token>& tokens, source& src) : sequence(src), m_tokens(tokens)
        {}

        prepared_type prepare(std::vector<token>& tokens) override;
        bool compile(compiler* c) override;

        std::vector<parameter>& params() { return m_params; }

    private:
        std::vector<primal::token> m_tokens;
        std::vector<parameter> m_params;
        std::string m_function_name;
    };
}

#endif // FUNCTION_CALL_H
