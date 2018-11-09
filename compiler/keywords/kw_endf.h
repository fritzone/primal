#ifndef KW_ENDF_H
#define KW_ENDF_H

#include "sequence.h"
#include "keywords.h"

namespace primal
{
    class kw_endf : public sequence, public keyword
    {
    public:

        static constexpr const char *N= "ENDF";

        explicit kw_endf(source& src) : sequence(src) {}

        sequence::prepared_type prepare(std::vector<token>&) override;
        bool compile(compiler*) override;

        std::string name() override { return N; }

    };
}

#endif //KW_ENDF_H
