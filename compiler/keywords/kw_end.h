#ifndef PRIMITIVE_KW_ENDIF_H
#define PRIMITIVE_KW_ENDIF_H

#include "sequence.h"
#include "keywords.h"

namespace primal
{
    class kw_end : public sequence, public keyword
    {
    public:

        static constexpr const char *N= "END";

        explicit kw_end(source& src) : sequence(src) {}

        sequence::prepared_type prepare(std::vector<token>&) override;
        bool compile(compiler*) override;

        std::string name() override { return N; }

    };
}

#endif //PRIMITIVE_KW_ENDIF_H
