#ifndef KW_IF_H
#define KW_IF_H

#include "sequence.h"
#include "keywords.h"

namespace primal
{
/* Class representing the IF keyword */
    class kw_if : public sequence, public keyword
    {
    public:

        static constexpr const char* N= "IF";

        explicit kw_if(source& src) : sequence(src) {}
        sequence::prepared_type prepare(std::vector<token>& tokens) override;
        bool compile(compiler* c) override;

        std::string name() override { return N; }

    private:

        std::vector<std::shared_ptr<sequence>> m_if_body;
        std::vector<std::shared_ptr<sequence>> m_else_body;
    };
}


#endif // KW_IF_H


