#ifndef KW_RETURN_H
#define KW_RETURN_H

#include "sequence.h"
#include "token.h"
#include "keywords.h"
#include <vector>
#include <memory>

namespace primal {
class kw_return : public sequence, public keyword {
public:
    static constexpr const char* N = "RETURN";
    explicit kw_return(source& src) : sequence(src) {}

    sequence::prepared_type prepare(std::vector<token>& tokens) override;
    bool compile(compiler* c) override;
    std::string name() override { return N; }
};
}

#endif // KW_RETURN_H
