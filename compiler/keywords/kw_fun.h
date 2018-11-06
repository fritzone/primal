#ifndef KW_FUN_H
#define KW_FUN_H

#include "sequence.h"
#include "keywords.h"
#include "function.h"

namespace primal
{
/* Class representing the IF keyword */
    class kw_fun : public sequence, public keyword
    {
    public:

        static constexpr const char* N= "FUN";

        explicit kw_fun(source& src) : sequence(src) {}
        sequence::prepared_type prepare(std::vector<token>& tokens) override;
        bool compile(compiler* c) override;

        std::string name() override { return N; }
        std::shared_ptr<fun> function() { return m_function; }
    private:

        // this is the associated function
        std::shared_ptr<fun> m_function;
    };
}

#endif // KW_FUN_H
