#ifndef KW_GOTO_H
#define KW_GOTO_H

#include "sequence.h"
#include "keywords.h"
#include "label.h"

namespace primate
{
    class kw_goto : public sequence, public keyword
    {
    public:
        static constexpr const char* N= "GOTO";

        explicit kw_goto(source& src) : sequence(src), m_label(src) {}

        sequence::prepared_type prepare(std::vector<token>& tokens) override;
        bool compile(compiler* c) override;

        std::string name() override { return N; }

    private:

        label m_label;
    };
}

#endif
