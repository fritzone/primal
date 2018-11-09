#ifndef FUNCTION_CALL_H
#define FUNCTION_CALL_H

#include "sequence.h"
#include "token.h"
#include "parameter.h"

#include <vector>

namespace primal
{

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

        std::vector<parameter_inst>& params() { return m_params; }

    private:
        std::vector<primal::token> m_tokens;
        std::vector<parameter_inst> m_params;
        std::string m_function_name;
    };
}

#endif // FUNCTION_CALL_H
