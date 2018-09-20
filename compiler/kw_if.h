#ifndef KW_IF_H
#define KW_IF_H

#include "sequence.h"
#include "keywords.h"

/* Class representing the LET keyword */
class kw_if : public sequence, public keyword
{
public:

    static constexpr const char* N= "IF";

    explicit kw_if(source& src) : sequence(src) {}
    sequence::prepared_type prepare(std::vector<token>& tokens) override;
    bool compile(compiler* c) override;

    std::string name() override { return N; }

private:

    std::vector<std::shared_ptr<sequence>> m_if_body;
};

class kw_endif : public sequence, public keyword
{
public:

    static constexpr const char *N= "ENDIF";

    explicit kw_endif(source& src) : sequence(src) {}
    sequence::prepared_type prepare(std::vector<token>&) override { return sequence::prepared_type::PT_NORMAL; }
    virtual bool compile(compiler*) override { return true; }

    std::string name() override { return N; }

};

#endif // KW_IF_H
