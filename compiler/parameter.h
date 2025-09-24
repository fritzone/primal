#ifndef PARAMETER_H
#define PARAMETER_H

#include "sequence.h"
#include "token.h"
#include "types.h"

#include <vector>

namespace primal
{

    /* Represents a parameter that goes in a function declaration */
    struct parameter
    {
        std::string name;
        primal::entity_type type;
    };

    /**
     * Represents a parameter that goes into a given function call
     */
    class parameter_inst final
    {
    public:

        explicit parameter_inst(const std::vector<token>& t) : m_params_tokens(t) {}

        std::vector<token>& tokens() {return m_params_tokens; }
        std::shared_ptr<ast>& root() {return m_param_tree; }

    private:
        std::vector<token> m_params_tokens;
        std::shared_ptr<ast> m_param_tree;

    };

}

#endif // PARAMETER_H
