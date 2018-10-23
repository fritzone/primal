#ifndef PRIMITIVE_KW_ENDIF_H
#define PRIMITIVE_KW_ENDIF_H

#include "sequence.h"
#include "keywords.h"

namespace primal
{
    class kw_endif : public sequence, public keyword
    {
    public:

        static constexpr const char *N= "ENDIF";

        explicit kw_endif(source& src) : sequence(src) {}

        sequence::prepared_type prepare(std::vector<token>&) override;
        bool compile(compiler*) override;

        std::string name() override { return N; }

    };
}

#endif //PRIMITIVE_KW_ENDIF_H
