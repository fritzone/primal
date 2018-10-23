#ifndef KW_ASM_H
#define KW_ASM_H

#include "sequence.h"
#include "keywords.h"

namespace primate
{
    class kw_asm : public sequence, public keyword
    {
    public:
        static constexpr const char* N= "ASM";

        explicit kw_asm(source& src) : sequence(src) {}

        sequence::prepared_type prepare(std::vector<token>& tokens) override;
        bool compile(compiler* c) override;

        std::string name() override { return N; }

    private:

        std::vector<uint8_t> m_precompiled;
    };

}


#endif
