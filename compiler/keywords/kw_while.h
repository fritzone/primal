#ifndef KW_WHILE_H
#define KW_WHILE_H

#include "sequence.h"
#include "keywords.h"

namespace primal
{
    class kw_while : public sequence, public keyword
    {
    public:

	static constexpr const char* N= "WHILE";

	explicit kw_while(source& src) : sequence(src) {}
	sequence::prepared_type prepare(std::vector<token>& tokens) override;
	bool compile(compiler* c) override;

	std::string name() override { return N; }

    private:

	std::vector<std::shared_ptr<sequence>> m_while_body;
    };
}

#endif
