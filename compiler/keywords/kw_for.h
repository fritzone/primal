#ifndef KW_FOR_H
#define KW_FOR_H

#include "sequence.h"
#include "token.h"
#include "keywords.h"

#include <vector>
#include <memory>

namespace primal {

struct variable;

class kw_for : public sequence, public keyword {
public:
    static constexpr const char* N = "FOR";
    explicit kw_for(source& src) : sequence(src) {}

    sequence::prepared_type prepare(std::vector<token>& tokens) override;
    bool compile(compiler* c) override;
    std::string name() override { return N; }

private:
    std::shared_ptr<variable> m_iterator;
    std::string m_iterator_name;

    // Expressions for the loop parameters
    std::shared_ptr<sequence> m_initial_seq;
    std::shared_ptr<sequence> m_end_seq;
    std::shared_ptr<sequence> m_step_seq;

    // The body of the loop
    std::vector<std::shared_ptr<sequence>> m_for_body;
};
}

#endif // KW_FOR_H
