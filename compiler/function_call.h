#ifndef FUNCTION_CALL_H
#define FUNCTION_CALL_H

#include "sequence.h"
#include "token.h"

#include <vector>

namespace primal
{
    class function_call : public sequence
    {
    public:
        ~function_call() = default;

        function_call(const std::vector<primal::token>& tokens, source& src) : sequence(src), m_tokens(tokens)
        {}

        prepared_type prepare(std::vector<token>& tokens) override;

    private:
        std::vector<primal::token> m_tokens;
        std::vector<std::vector<token>> m_params_tokens;
    };
}

#endif // FUNCTION_CALL_H
