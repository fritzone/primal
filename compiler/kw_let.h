#ifndef KW_LET_H
#define KW_LET_H

#include "sequence.h"
#include "keywords.h"

#include <string>
#include <vector>
#include <memory>

class variable;

/* Class representing the LET keyword */
class kw_let : public sequence, public keyword
{
public:

    static constexpr const char* N= "LET";

    explicit kw_let(source& src) : sequence(src) {}

    virtual bool prepare(std::vector<token>& tokens) override;
    virtual bool compile(compiler* c) override;

    std::string name() override { return N; }

private:
    std::shared_ptr<variable> m_variable;
};


#endif

