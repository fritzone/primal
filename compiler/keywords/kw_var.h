#ifndef KW_VAR_H
#define KW_VAR_H

#include "sequence.h"
#include "keywords.h"

#include <string>
#include <memory>

namespace primal
{
    class variable;

    class kw_var : public sequence, public keyword
    {
    public:
        static constexpr const char* N= "VAR";

        explicit kw_var(source& src) : sequence(src) {}

        sequence::prepared_type prepare(std::vector<token>& tokens) override;
        virtual bool compile(compiler* c) override;

        std::string name() override { return N; }

    };

}

#endif // KW_VAR_H
