#ifndef KW_NEXT_H
#define KW_NEXT_H

#include "sequence.h"
#include "keywords.h"

namespace primal {
class kw_next : public sequence, public keyword {
public:
    static constexpr const char* N = "NEXT";
    explicit kw_next(source& src) : sequence(src) {}

    sequence::prepared_type prepare(std::vector<token>& tokens) override;
    bool compile(compiler* c) override;
    std::string name() override { return N; }
};
}

#endif // KW_NEXT_H
