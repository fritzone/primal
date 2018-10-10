#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>

#include "token.h"

class lexer
{
public:

    explicit lexer(std::string s) : m_sequence(std::move(s)) {}

    std::vector<token> tokenize();

private:

    std::string m_sequence;
};

#endif // LEXER_H

